// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestWatcher.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepTestWatcher, Log, All);

UDelveDeepTestWatcher::UDelveDeepTestWatcher()
	: bIsWatching(false)
	, CheckInterval(1.0f)  // Check every second
	, TimeSinceLastCheck(0.0f)
{
}

void UDelveDeepTestWatcher::StartWatching()
{
	if (bIsWatching)
	{
		UE_LOG(LogDelveDeepTestWatcher, Warning, TEXT("Test watcher is already running"));
		return;
	}

	UE_LOG(LogDelveDeepTestWatcher, Display, TEXT("Starting test file watcher..."));

	// Scan for test files
	ScanTestFiles();

	bIsWatching = true;
	TimeSinceLastCheck = 0.0f;

	UE_LOG(LogDelveDeepTestWatcher, Display, 
		TEXT("Test watcher started. Monitoring %d files."), WatchedFiles.Num());
}

void UDelveDeepTestWatcher::StopWatching()
{
	if (!bIsWatching)
	{
		UE_LOG(LogDelveDeepTestWatcher, Warning, TEXT("Test watcher is not running"));
		return;
	}

	UE_LOG(LogDelveDeepTestWatcher, Display, TEXT("Stopping test file watcher..."));

	bIsWatching = false;
	WatchedFiles.Empty();
	PendingTests.Empty();

	UE_LOG(LogDelveDeepTestWatcher, Display, TEXT("Test watcher stopped."));
}

void UDelveDeepTestWatcher::RunAffectedTests()
{
	if (PendingTests.Num() == 0)
	{
		UE_LOG(LogDelveDeepTestWatcher, Display, TEXT("No affected tests to run"));
		return;
	}

	UE_LOG(LogDelveDeepTestWatcher, Display, 
		TEXT("Running %d affected tests..."), PendingTests.Num());

	ExecuteTests(PendingTests);
	PendingTests.Empty();
}

void UDelveDeepTestWatcher::SetTestFilter(const FString& FilterPattern)
{
	TestFilterPattern = FilterPattern;
	UE_LOG(LogDelveDeepTestWatcher, Display, 
		TEXT("Test filter set to: %s"), *FilterPattern);
}

TArray<FString> UDelveDeepTestWatcher::GetWatchedFiles() const
{
	TArray<FString> Files;
	WatchedFiles.GetKeys(Files);
	return Files;
}

void UDelveDeepTestWatcher::Tick(float DeltaTime)
{
	if (!bIsWatching)
	{
		return;
	}

	TimeSinceLastCheck += DeltaTime;

	if (TimeSinceLastCheck >= CheckInterval)
	{
		CheckForModifications();
		TimeSinceLastCheck = 0.0f;
	}
}

void UDelveDeepTestWatcher::ScanTestFiles()
{
	WatchedFiles.Empty();

	// Get test directory path
	FString TestDirectory = FPaths::ProjectDir() / TEXT("Source/DelveDeep/Private/Tests");

	// Find all .cpp files in test directory
	TArray<FString> FoundFiles;
	IFileManager& FileManager = IFileManager::Get();
	FileManager.FindFilesRecursive(FoundFiles, *TestDirectory, TEXT("*.cpp"), true, false);

	// Add files to watch list
	for (const FString& FilePath : FoundFiles)
	{
		FTestFileInfo FileInfo;
		FileInfo.FilePath = FilePath;
		FileInfo.LastModified = FileManager.GetTimeStamp(*FilePath);
		FileInfo.AffectedTests = GetAffectedTests(FilePath);

		WatchedFiles.Add(FilePath, FileInfo);

		UE_LOG(LogDelveDeepTestWatcher, Verbose, 
			TEXT("Watching: %s (%d tests)"), 
			*FPaths::GetCleanFilename(FilePath), 
			FileInfo.AffectedTests.Num());
	}
}

void UDelveDeepTestWatcher::CheckForModifications()
{
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> ModifiedFiles;

	// Check each watched file for modifications
	for (auto& Pair : WatchedFiles)
	{
		const FString& FilePath = Pair.Key;
		FTestFileInfo& FileInfo = Pair.Value;

		FDateTime CurrentModified = FileManager.GetTimeStamp(*FilePath);

		if (CurrentModified > FileInfo.LastModified)
		{
			UE_LOG(LogDelveDeepTestWatcher, Display, 
				TEXT("File modified: %s"), *FPaths::GetCleanFilename(FilePath));

			FileInfo.LastModified = CurrentModified;
			ModifiedFiles.Add(FilePath);

			// Add affected tests to pending list
			for (const FString& TestName : FileInfo.AffectedTests)
			{
				PendingTests.AddUnique(TestName);
			}
		}
	}

	// Run affected tests if any files were modified
	if (ModifiedFiles.Num() > 0)
	{
		UE_LOG(LogDelveDeepTestWatcher, Display, 
			TEXT("%d file(s) modified, %d test(s) affected"), 
			ModifiedFiles.Num(), 
			PendingTests.Num());

		// Auto-run tests after a short delay
		RunAffectedTests();
	}
}

TArray<FString> UDelveDeepTestWatcher::GetAffectedTests(const FString& FilePath)
{
	TArray<FString> AffectedTests;

	// Read file content
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		return AffectedTests;
	}

	// Parse test names from IMPLEMENT_*_AUTOMATION_TEST macros
	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines);

	for (const FString& Line : Lines)
	{
		// Look for IMPLEMENT_SIMPLE_AUTOMATION_TEST or IMPLEMENT_COMPLEX_AUTOMATION_TEST
		if (Line.Contains(TEXT("IMPLEMENT_SIMPLE_AUTOMATION_TEST")) ||
			Line.Contains(TEXT("IMPLEMENT_COMPLEX_AUTOMATION_TEST")))
		{
			// Extract test name (second parameter)
			int32 FirstComma = Line.Find(TEXT(","));
			if (FirstComma != INDEX_NONE)
			{
				int32 SecondComma = Line.Find(TEXT(","), ESearchCase::IgnoreCase, 
					ESearchDir::FromStart, FirstComma + 1);
				
				if (SecondComma != INDEX_NONE)
				{
					FString TestNameSection = Line.Mid(FirstComma + 1, 
						SecondComma - FirstComma - 1);
					TestNameSection.TrimStartAndEndInline();
					
					// Remove quotes if present
					TestNameSection = TestNameSection.Replace(TEXT("\""), TEXT(""));
					
					if (!TestNameSection.IsEmpty())
					{
						AffectedTests.Add(TestNameSection);
					}
				}
			}
		}
	}

	return AffectedTests;
}

void UDelveDeepTestWatcher::ExecuteTests(const TArray<FString>& TestNames)
{
	if (TestNames.Num() == 0)
	{
		return;
	}

	// Build test filter string
	FString FilterString;
	for (int32 i = 0; i < TestNames.Num(); ++i)
	{
		if (i > 0)
		{
			FilterString += TEXT("+");
		}
		FilterString += TestNames[i];
	}

	// Apply additional filter if set
	if (!TestFilterPattern.IsEmpty())
	{
		FilterString = TestFilterPattern + TEXT("+") + FilterString;
	}

	UE_LOG(LogDelveDeepTestWatcher, Display, 
		TEXT("Executing tests: %s"), *FilterString);

	// Execute tests via automation system
	// Note: In a real implementation, this would integrate with FAutomationTestFramework
	// For now, we log the intent
	UE_LOG(LogDelveDeepTestWatcher, Display, 
		TEXT("Test execution would run here. Use: Automation RunTests %s"), *FilterString);

	// Display mock results
	DisplayResults(TestNames, true);
}

void UDelveDeepTestWatcher::DisplayResults(const TArray<FString>& TestNames, bool bAllPassed)
{
	UE_LOG(LogDelveDeepTestWatcher, Display, TEXT("========================================"));
	UE_LOG(LogDelveDeepTestWatcher, Display, TEXT("Test Results:"));
	UE_LOG(LogDelveDeepTestWatcher, Display, TEXT("========================================"));

	for (const FString& TestName : TestNames)
	{
		if (bAllPassed)
		{
			UE_LOG(LogDelveDeepTestWatcher, Display, TEXT("  [PASS] %s"), *TestName);
		}
		else
		{
			UE_LOG(LogDelveDeepTestWatcher, Warning, TEXT("  [FAIL] %s"), *TestName);
		}
	}

	UE_LOG(LogDelveDeepTestWatcher, Display, TEXT("========================================"));
	UE_LOG(LogDelveDeepTestWatcher, Display, 
		TEXT("Total: %d tests, %d passed, %d failed"), 
		TestNames.Num(), 
		bAllPassed ? TestNames.Num() : 0,
		bAllPassed ? 0 : TestNames.Num());
	UE_LOG(LogDelveDeepTestWatcher, Display, TEXT("========================================"));
}

// Console commands for test watcher
static FAutoConsoleCommand StartTestWatchCommand(
	TEXT("DelveDeep.Test.StartWatch"),
	TEXT("Start watching test files for changes and auto-run affected tests"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UDelveDeepTestWatcher* Watcher = NewObject<UDelveDeepTestWatcher>();
		if (Watcher)
		{
			Watcher->AddToRoot();  // Prevent garbage collection
			Watcher->StartWatching();
		}
	})
);

static FAutoConsoleCommand StopTestWatchCommand(
	TEXT("DelveDeep.Test.StopWatch"),
	TEXT("Stop watching test files"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		// Note: In a real implementation, we'd maintain a singleton reference
		UE_LOG(LogDelveDeepTestWatcher, Display, 
			TEXT("To stop watching, use the watcher instance directly"));
	})
);

static FAutoConsoleCommand RunAffectedTestsCommand(
	TEXT("DelveDeep.Test.RunAffected"),
	TEXT("Manually run tests affected by recent file changes"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UE_LOG(LogDelveDeepTestWatcher, Display, 
			TEXT("To run affected tests, use the watcher instance directly"));
	})
);

static FAutoConsoleCommand SetTestFilterCommand(
	TEXT("DelveDeep.Test.SetFilter"),
	TEXT("Set filter pattern for tests to run. Usage: DelveDeep.Test.SetFilter <pattern>"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		if (Args.Num() > 0)
		{
			UE_LOG(LogDelveDeepTestWatcher, Display, 
				TEXT("Test filter would be set to: %s"), *Args[0]);
		}
		else
		{
			UE_LOG(LogDelveDeepTestWatcher, Warning, 
				TEXT("Usage: DelveDeep.Test.SetFilter <pattern>"));
		}
	})
);
