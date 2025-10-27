// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestOptimization.h"
#include "HAL/PlatformTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

// Initialize singleton
FTestExecutionOptimizer* FTestExecutionOptimizer::Instance = nullptr;

FTestExecutionOptimizer& FTestExecutionOptimizer::Get()
{
	if (!Instance)
	{
		Instance = new FTestExecutionOptimizer();
	}
	return *Instance;
}

void FTestExecutionOptimizer::RecordTestExecution(const FString& TestName, double ExecutionTimeMs, bool bPassed)
{
	FTestExecutionStats& Stats = ExecutionStats.FindOrAdd(TestName);
	
	Stats.TestName = TestName;
	Stats.ExecutionTimeMs = ExecutionTimeMs;
	Stats.ExecutionCount++;
	Stats.bLastPassed = bPassed;
	Stats.LastExecutionTime = FDateTime::Now();

	// Update average
	if (Stats.ExecutionCount == 1)
	{
		Stats.AverageExecutionTimeMs = ExecutionTimeMs;
	}
	else
	{
		// Running average
		Stats.AverageExecutionTimeMs = 
			(Stats.AverageExecutionTimeMs * (Stats.ExecutionCount - 1) + ExecutionTimeMs) / Stats.ExecutionCount;
	}
}

const FTestExecutionStats* FTestExecutionOptimizer::GetTestStats(const FString& TestName) const
{
	return ExecutionStats.Find(TestName);
}

TArray<FString> FTestExecutionOptimizer::GetSlowestTests(int32 Count) const
{
	// Create array of test names with execution times
	TArray<TPair<FString, double>> TestTimes;
	for (const auto& Pair : ExecutionStats)
	{
		TestTimes.Add(TPair<FString, double>(Pair.Key, Pair.Value.AverageExecutionTimeMs));
	}

	// Sort by execution time (descending)
	TestTimes.Sort([](const TPair<FString, double>& A, const TPair<FString, double>& B)
	{
		return A.Value > B.Value;
	});

	// Extract test names
	TArray<FString> Result;
	for (int32 i = 0; i < FMath::Min(Count, TestTimes.Num()); ++i)
	{
		Result.Add(TestTimes[i].Key);
	}

	return Result;
}

TArray<FString> FTestExecutionOptimizer::GetFastestTests(int32 Count) const
{
	// Create array of test names with execution times
	TArray<TPair<FString, double>> TestTimes;
	for (const auto& Pair : ExecutionStats)
	{
		TestTimes.Add(TPair<FString, double>(Pair.Key, Pair.Value.AverageExecutionTimeMs));
	}

	// Sort by execution time (ascending)
	TestTimes.Sort([](const TPair<FString, double>& A, const TPair<FString, double>& B)
	{
		return A.Value < B.Value;
	});

	// Extract test names
	TArray<FString> Result;
	for (int32 i = 0; i < FMath::Min(Count, TestTimes.Num()); ++i)
	{
		Result.Add(TestTimes[i].Key);
	}

	return Result;
}

double FTestExecutionOptimizer::GetTotalExecutionTime() const
{
	double Total = 0.0;
	for (const auto& Pair : ExecutionStats)
	{
		Total += Pair.Value.AverageExecutionTimeMs;
	}
	return Total;
}

bool FTestExecutionOptimizer::ExportStatsToCSV(const FString& OutputPath) const
{
	FString CSV;

	// Header
	CSV += TEXT("Test Name,Execution Count,Last Execution Time (ms),Average Execution Time (ms),Last Passed,Last Execution Date\n");

	// Data rows
	for (const auto& Pair : ExecutionStats)
	{
		const FTestExecutionStats& Stats = Pair.Value;
		CSV += FString::Printf(TEXT("%s,%d,%.3f,%.3f,%s,%s\n"),
			*Stats.TestName,
			Stats.ExecutionCount,
			Stats.ExecutionTimeMs,
			Stats.AverageExecutionTimeMs,
			Stats.bLastPassed ? TEXT("true") : TEXT("false"),
			*Stats.LastExecutionTime.ToString());
	}

	// Write to file
	FString FullPath = FPaths::ProjectDir() / OutputPath;
	return FFileHelper::SaveStringToFile(CSV, *FullPath);
}

void FTestExecutionOptimizer::CacheTestResult(
	const FString& TestName,
	uint32 CodeHash,
	bool bPassed,
	double ExecutionTimeMs,
	const TArray<FString>& Output)
{
	FTestResultCacheEntry& Entry = ResultCache.FindOrAdd(TestName);
	
	Entry.TestName = TestName;
	Entry.CodeHash = CodeHash;
	Entry.bPassed = bPassed;
	Entry.ExecutionTimeMs = ExecutionTimeMs;
	Entry.CacheTime = FDateTime::Now();
	Entry.Output = Output;
}

const FTestResultCacheEntry* FTestExecutionOptimizer::GetCachedResult(const FString& TestName, uint32 CodeHash) const
{
	const FTestResultCacheEntry* Entry = ResultCache.Find(TestName);
	if (!Entry)
	{
		return nullptr;
	}

	// Check if code hash matches
	if (Entry->CodeHash != CodeHash)
	{
		return nullptr;
	}

	// Check if cache is not too old (1 hour)
	FTimespan Age = FDateTime::Now() - Entry->CacheTime;
	if (Age.GetTotalSeconds() > 3600.0)
	{
		return nullptr;
	}

	return Entry;
}

bool FTestExecutionOptimizer::HasValidCachedResult(const FString& TestName, uint32 CodeHash) const
{
	return GetCachedResult(TestName, CodeHash) != nullptr;
}

void FTestExecutionOptimizer::ClearCache()
{
	ResultCache.Empty();
}

void FTestExecutionOptimizer::ClearOldCache(double MaxAgeSeconds)
{
	TArray<FString> ToRemove;
	FDateTime Now = FDateTime::Now();

	for (const auto& Pair : ResultCache)
	{
		FTimespan Age = Now - Pair.Value.CacheTime;
		if (Age.GetTotalSeconds() > MaxAgeSeconds)
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (const FString& TestName : ToRemove)
	{
		ResultCache.Remove(TestName);
	}
}

bool FTestExecutionOptimizer::SaveCache(const FString& FilePath) const
{
	// Create JSON object
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());

	// Add cache entries
	TArray<TSharedPtr<FJsonValue>> EntriesArray;
	for (const auto& Pair : ResultCache)
	{
		const FTestResultCacheEntry& Entry = Pair.Value;

		TSharedPtr<FJsonObject> EntryObject = MakeShareable(new FJsonObject());
		EntryObject->SetStringField(TEXT("TestName"), Entry.TestName);
		EntryObject->SetNumberField(TEXT("CodeHash"), Entry.CodeHash);
		EntryObject->SetBoolField(TEXT("Passed"), Entry.bPassed);
		EntryObject->SetNumberField(TEXT("ExecutionTimeMs"), Entry.ExecutionTimeMs);
		EntryObject->SetStringField(TEXT("CacheTime"), Entry.CacheTime.ToString());

		TArray<TSharedPtr<FJsonValue>> OutputArray;
		for (const FString& Line : Entry.Output)
		{
			OutputArray.Add(MakeShareable(new FJsonValueString(Line)));
		}
		EntryObject->SetArrayField(TEXT("Output"), OutputArray);

		EntriesArray.Add(MakeShareable(new FJsonValueObject(EntryObject)));
	}
	RootObject->SetArrayField(TEXT("Entries"), EntriesArray);

	// Serialize to string
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	if (!FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize test cache"));
		return false;
	}

	// Write to file
	FString FullPath = FPaths::ProjectDir() / FilePath;
	return FFileHelper::SaveStringToFile(JsonString, *FullPath);
}

bool FTestExecutionOptimizer::LoadCache(const FString& FilePath)
{
	// Read file
	FString FullPath = FPaths::ProjectDir() / FilePath;
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FullPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load test cache from: %s"), *FullPath);
		return false;
	}

	// Parse JSON
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, RootObject))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse test cache JSON"));
		return false;
	}

	// Load entries
	const TArray<TSharedPtr<FJsonValue>>* EntriesArray;
	if (!RootObject->TryGetArrayField(TEXT("Entries"), EntriesArray))
	{
		UE_LOG(LogTemp, Error, TEXT("Test cache JSON missing Entries array"));
		return false;
	}

	ResultCache.Empty();

	for (const TSharedPtr<FJsonValue>& EntryValue : *EntriesArray)
	{
		TSharedPtr<FJsonObject> EntryObject = EntryValue->AsObject();
		if (!EntryObject.IsValid())
		{
			continue;
		}

		FTestResultCacheEntry Entry;
		Entry.TestName = EntryObject->GetStringField(TEXT("TestName"));
		Entry.CodeHash = static_cast<uint32>(EntryObject->GetNumberField(TEXT("CodeHash")));
		Entry.bPassed = EntryObject->GetBoolField(TEXT("Passed"));
		Entry.ExecutionTimeMs = EntryObject->GetNumberField(TEXT("ExecutionTimeMs"));
		
		FString CacheTimeStr = EntryObject->GetStringField(TEXT("CacheTime"));
		FDateTime::Parse(CacheTimeStr, Entry.CacheTime);

		const TArray<TSharedPtr<FJsonValue>>* OutputArray;
		if (EntryObject->TryGetArrayField(TEXT("Output"), OutputArray))
		{
			for (const TSharedPtr<FJsonValue>& OutputValue : *OutputArray)
			{
				Entry.Output.Add(OutputValue->AsString());
			}
		}

		ResultCache.Add(Entry.TestName, Entry);
	}

	UE_LOG(LogTemp, Display, TEXT("Loaded %d cached test results"), ResultCache.Num());
	return true;
}

void FTestExecutionOptimizer::Reset()
{
	ExecutionStats.Empty();
	ResultCache.Empty();
}

// FScopedTestExecutionTracker implementation

FScopedTestExecutionTracker::FScopedTestExecutionTracker(const FString& InTestName)
	: TestName(InTestName)
	, StartTime(FPlatformTime::Seconds())
	, bPassed(false)
{
}

FScopedTestExecutionTracker::~FScopedTestExecutionTracker()
{
	double EndTime = FPlatformTime::Seconds();
	double ExecutionTimeMs = (EndTime - StartTime) * 1000.0;

	FTestExecutionOptimizer::Get().RecordTestExecution(TestName, ExecutionTimeMs, bPassed);
}

void FScopedTestExecutionTracker::SetPassed(bool bInPassed)
{
	bPassed = bInPassed;
}
