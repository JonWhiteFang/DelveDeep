// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTelemetrySubsystem.h"
#include "DelveDeepValidation.h"
#include "Engine/World.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "Misc/App.h"

// Baseline Management Implementation

bool UDelveDeepTelemetrySubsystem::CaptureBaseline(FName BaselineName)
{
	FValidationContext Context;
	Context.SystemName = TEXT("Telemetry");
	Context.OperationName = TEXT("CaptureBaseline");

	// Validate baseline name
	if (BaselineName.IsNone())
	{
		Context.AddError(TEXT("Baseline name cannot be empty"));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Warn if overwriting existing baseline
	if (Baselines.Contains(BaselineName))
	{
		Context.AddWarning(FString::Printf(
			TEXT("Baseline '%s' already exists and will be overwritten"),
			*BaselineName.ToString()));
		UE_LOG(LogDelveDeepTelemetry, Warning, TEXT("%s"), *Context.GetReport());
	}

	// Create new baseline
	FPerformanceBaseline Baseline;
	Baseline.BaselineName = BaselineName;
	Baseline.CaptureTime = FDateTime::Now();
	Baseline.BuildVersion = FApp::GetBuildVersion();
	
	// Get map name
	if (UWorld* World = GetWorld())
	{
		Baseline.MapName = World->GetMapName();
	}

	// Capture frame performance data
	Baseline.AverageFrameData = FrameTracker.GetCurrentFrameData();
	Baseline.AverageFPS = FrameTracker.GetAverageFPS();
	Baseline.OnePercentLowFPS = FrameTracker.GetOnePercentLowFPS();
	Baseline.TotalFramesCaptured = FrameTracker.GetFrameTimeHistory(3600).Num();

	// Capture system performance data
	TArray<FSystemPerformanceData> AllSystemData = SystemProfiler.GetAllSystemData();
	for (const FSystemPerformanceData& SystemData : AllSystemData)
	{
		Baseline.SystemData.Add(SystemData.SystemName, SystemData);
	}

	// Capture memory snapshot
	Baseline.MemoryData = MemoryTracker.GetCurrentSnapshot();

	// Store baseline
	Baselines.Add(BaselineName, Baseline);

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Captured baseline '%s': %.2f FPS, %.2f ms frame time, %d systems, %.2f MB memory"),
		*BaselineName.ToString(),
		Baseline.AverageFPS,
		Baseline.AverageFrameData.FrameTimeMs,
		Baseline.SystemData.Num(),
		Baseline.MemoryData.TotalMemory / (1024.0 * 1024.0));

	return true;
}

bool UDelveDeepTelemetrySubsystem::CompareToBaseline(FName BaselineName, FPerformanceComparison& OutComparison)
{
	FValidationContext Context;
	Context.SystemName = TEXT("Telemetry");
	Context.OperationName = TEXT("CompareToBaseline");

	// Validate baseline exists
	const FPerformanceBaseline* Baseline = Baselines.Find(BaselineName);
	if (!Baseline)
	{
		Context.AddError(FString::Printf(
			TEXT("Baseline '%s' not found"),
			*BaselineName.ToString()));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Validate baseline compatibility
	if (!ValidateBaseline(*Baseline, Context))
	{
		UE_LOG(LogDelveDeepTelemetry, Warning,
			TEXT("Baseline validation warnings: %s"),
			*Context.GetReport());
	}

	// Initialize comparison
	OutComparison = FPerformanceComparison();
	OutComparison.BaselineName = BaselineName;
	OutComparison.ComparisonTime = FDateTime::Now();

	// Get current performance data
	float CurrentFPS = FrameTracker.GetAverageFPS();
	float CurrentFrameTime = FrameTracker.GetCurrentFrameData().FrameTimeMs;
	float CurrentOnePercentLow = FrameTracker.GetOnePercentLowFPS();
	FMemorySnapshot CurrentMemory = MemoryTracker.GetCurrentSnapshot();

	// Calculate FPS change (positive = improvement)
	if (Baseline->AverageFPS > 0.0f)
	{
		OutComparison.FPSChangePercent = ((CurrentFPS - Baseline->AverageFPS) / Baseline->AverageFPS) * 100.0f;
	}

	// Calculate frame time change (negative = improvement)
	if (Baseline->AverageFrameData.FrameTimeMs > 0.0f)
	{
		OutComparison.FrameTimeChangePercent = 
			((CurrentFrameTime - Baseline->AverageFrameData.FrameTimeMs) / Baseline->AverageFrameData.FrameTimeMs) * 100.0f;
	}

	// Calculate 1% low FPS change
	if (Baseline->OnePercentLowFPS > 0.0f)
	{
		OutComparison.OnePercentLowChangePercent = 
			((CurrentOnePercentLow - Baseline->OnePercentLowFPS) / Baseline->OnePercentLowFPS) * 100.0f;
	}

	// Calculate memory change
	if (Baseline->MemoryData.TotalMemory > 0)
	{
		OutComparison.MemoryChangePercent = 
			((static_cast<float>(CurrentMemory.TotalMemory) - static_cast<float>(Baseline->MemoryData.TotalMemory)) / 
			 static_cast<float>(Baseline->MemoryData.TotalMemory)) * 100.0f;
	}

	// Compare system performance
	TArray<FSystemPerformanceData> CurrentSystemData = SystemProfiler.GetAllSystemData();
	for (const FSystemPerformanceData& CurrentSystem : CurrentSystemData)
	{
		const FSystemPerformanceData* BaselineSystem = Baseline->SystemData.Find(CurrentSystem.SystemName);
		if (BaselineSystem && BaselineSystem->AverageTimeMs > 0.0)
		{
			float ChangePercent = 
				((CurrentSystem.AverageTimeMs - BaselineSystem->AverageTimeMs) / BaselineSystem->AverageTimeMs) * 100.0f;
			OutComparison.SystemTimeChanges.Add(CurrentSystem.SystemName, ChangePercent);
		}
	}

	// Determine if this is a regression or improvement
	// Regression threshold: >5% worse performance
	// Improvement threshold: >5% better performance
	constexpr float RegressionThreshold = 5.0f;
	constexpr float ImprovementThreshold = 5.0f;

	OutComparison.bIsRegression = 
		(OutComparison.FPSChangePercent < -RegressionThreshold) ||
		(OutComparison.FrameTimeChangePercent > RegressionThreshold) ||
		(OutComparison.MemoryChangePercent > RegressionThreshold * 2.0f); // More lenient for memory

	OutComparison.bIsImprovement = 
		(OutComparison.FPSChangePercent > ImprovementThreshold) ||
		(OutComparison.FrameTimeChangePercent < -ImprovementThreshold);

	// Generate detailed report
	FString Report;
	Report += FString::Printf(TEXT("Performance Comparison: %s\n"), *BaselineName.ToString());
	Report += FString::Printf(TEXT("Baseline captured: %s\n"), *Baseline->CaptureTime.ToString());
	Report += FString::Printf(TEXT("Comparison time: %s\n\n"), *OutComparison.ComparisonTime.ToString());
	
	Report += TEXT("Frame Performance:\n");
	Report += FString::Printf(TEXT("  FPS: %.2f -> %.2f (%+.2f%%)\n"), 
		Baseline->AverageFPS, CurrentFPS, OutComparison.FPSChangePercent);
	Report += FString::Printf(TEXT("  Frame Time: %.2f ms -> %.2f ms (%+.2f%%)\n"), 
		Baseline->AverageFrameData.FrameTimeMs, CurrentFrameTime, OutComparison.FrameTimeChangePercent);
	Report += FString::Printf(TEXT("  1%% Low FPS: %.2f -> %.2f (%+.2f%%)\n\n"), 
		Baseline->OnePercentLowFPS, CurrentOnePercentLow, OutComparison.OnePercentLowChangePercent);
	
	Report += TEXT("Memory:\n");
	Report += FString::Printf(TEXT("  Total: %.2f MB -> %.2f MB (%+.2f%%)\n\n"), 
		Baseline->MemoryData.TotalMemory / (1024.0 * 1024.0),
		CurrentMemory.TotalMemory / (1024.0 * 1024.0),
		OutComparison.MemoryChangePercent);
	
	if (OutComparison.SystemTimeChanges.Num() > 0)
	{
		Report += TEXT("System Performance Changes:\n");
		for (const auto& Pair : OutComparison.SystemTimeChanges)
		{
			Report += FString::Printf(TEXT("  %s: %+.2f%%\n"), *Pair.Key.ToString(), Pair.Value);
		}
	}

	if (OutComparison.bIsRegression)
	{
		Report += TEXT("\n*** PERFORMANCE REGRESSION DETECTED ***\n");
	}
	else if (OutComparison.bIsImprovement)
	{
		Report += TEXT("\n*** PERFORMANCE IMPROVEMENT DETECTED ***\n");
	}

	OutComparison.DetailedReport = Report;

	// Log comparison results
	if (OutComparison.bIsRegression)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning, TEXT("Performance regression detected:\n%s"), *Report);
	}
	else if (OutComparison.bIsImprovement)
	{
		UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Performance improvement detected:\n%s"), *Report);
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Performance comparison:\n%s"), *Report);
	}

	return true;
}

TArray<FName> UDelveDeepTelemetrySubsystem::GetAvailableBaselines() const
{
	TArray<FName> BaselineNames;
	Baselines.GetKeys(BaselineNames);
	return BaselineNames;
}

bool UDelveDeepTelemetrySubsystem::GetBaseline(FName BaselineName, FPerformanceBaseline& OutBaseline) const
{
	const FPerformanceBaseline* Baseline = Baselines.Find(BaselineName);
	if (Baseline)
	{
		OutBaseline = *Baseline;
		return true;
	}
	return false;
}

bool UDelveDeepTelemetrySubsystem::SaveBaseline(FName BaselineName, const FString& FilePath)
{
	FValidationContext Context;
	Context.SystemName = TEXT("Telemetry");
	Context.OperationName = TEXT("SaveBaseline");

	// Find baseline
	const FPerformanceBaseline* Baseline = Baselines.Find(BaselineName);
	if (!Baseline)
	{
		Context.AddError(FString::Printf(
			TEXT("Baseline '%s' not found"),
			*BaselineName.ToString()));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Determine save path
	FString SavePath = FilePath;
	if (SavePath.IsEmpty())
	{
		SavePath = GetDefaultBaselineDirectory() / (BaselineName.ToString() + TEXT(".json"));
	}

	// Validate directory exists
	FString Directory = FPaths::GetPath(SavePath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Directory))
	{
		if (!PlatformFile.CreateDirectoryTree(*Directory))
		{
			Context.AddError(FString::Printf(
				TEXT("Failed to create directory: %s"),
				*Directory));
			UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
			return false;
		}
	}

	// Create JSON object
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	
	// Serialize baseline data
	JsonObject->SetStringField(TEXT("BaselineName"), Baseline->BaselineName.ToString());
	JsonObject->SetStringField(TEXT("CaptureTime"), Baseline->CaptureTime.ToString());
	JsonObject->SetStringField(TEXT("BuildVersion"), Baseline->BuildVersion);
	JsonObject->SetStringField(TEXT("MapName"), Baseline->MapName);
	JsonObject->SetNumberField(TEXT("AverageFPS"), Baseline->AverageFPS);
	JsonObject->SetNumberField(TEXT("OnePercentLowFPS"), Baseline->OnePercentLowFPS);
	JsonObject->SetNumberField(TEXT("TotalFramesCaptured"), Baseline->TotalFramesCaptured);
	
	// Frame data
	TSharedPtr<FJsonObject> FrameDataObj = MakeShareable(new FJsonObject());
	FrameDataObj->SetNumberField(TEXT("FrameTimeMs"), Baseline->AverageFrameData.FrameTimeMs);
	FrameDataObj->SetNumberField(TEXT("GameThreadTimeMs"), Baseline->AverageFrameData.GameThreadTimeMs);
	FrameDataObj->SetNumberField(TEXT("RenderThreadTimeMs"), Baseline->AverageFrameData.RenderThreadTimeMs);
	JsonObject->SetObjectField(TEXT("FrameData"), FrameDataObj);
	
	// System data
	TArray<TSharedPtr<FJsonValue>> SystemDataArray;
	for (const auto& Pair : Baseline->SystemData)
	{
		TSharedPtr<FJsonObject> SystemObj = MakeShareable(new FJsonObject());
		SystemObj->SetStringField(TEXT("SystemName"), Pair.Key.ToString());
		SystemObj->SetNumberField(TEXT("CycleTimeMs"), Pair.Value.CycleTimeMs);
		SystemObj->SetNumberField(TEXT("BudgetTimeMs"), Pair.Value.BudgetTimeMs);
		SystemObj->SetNumberField(TEXT("AverageTimeMs"), Pair.Value.AverageTimeMs);
		SystemObj->SetNumberField(TEXT("PeakTimeMs"), Pair.Value.PeakTimeMs);
		SystemDataArray.Add(MakeShareable(new FJsonValueObject(SystemObj)));
	}
	JsonObject->SetArrayField(TEXT("SystemData"), SystemDataArray);
	
	// Memory data
	TSharedPtr<FJsonObject> MemoryObj = MakeShareable(new FJsonObject());
	MemoryObj->SetNumberField(TEXT("TotalMemory"), static_cast<double>(Baseline->MemoryData.TotalMemory));
	MemoryObj->SetNumberField(TEXT("NativeMemory"), static_cast<double>(Baseline->MemoryData.NativeMemory));
	MemoryObj->SetNumberField(TEXT("ManagedMemory"), static_cast<double>(Baseline->MemoryData.ManagedMemory));
	JsonObject->SetObjectField(TEXT("MemoryData"), MemoryObj);

	// Write to file
	FString JsonString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
	{
		Context.AddError(TEXT("Failed to serialize baseline to JSON"));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	if (!FFileHelper::SaveStringToFile(JsonString, *SavePath))
	{
		Context.AddError(FString::Printf(
			TEXT("Failed to write baseline to file: %s"),
			*SavePath));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Saved baseline '%s' to: %s (%.2f KB)"),
		*BaselineName.ToString(),
		*SavePath,
		JsonString.Len() / 1024.0f);

	return true;
}

bool UDelveDeepTelemetrySubsystem::LoadBaseline(FName BaselineName, const FString& FilePath)
{
	FValidationContext Context;
	Context.SystemName = TEXT("Telemetry");
	Context.OperationName = TEXT("LoadBaseline");

	// Validate file path
	if (FilePath.IsEmpty())
	{
		Context.AddError(TEXT("File path cannot be empty"));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Check file exists
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*FilePath))
	{
		Context.AddError(FString::Printf(
			TEXT("Baseline file not found: %s"),
			*FilePath));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Load file
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		Context.AddError(FString::Printf(
			TEXT("Failed to read baseline file: %s"),
			*FilePath));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		Context.AddError(TEXT("Failed to parse baseline JSON"));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Deserialize baseline
	FPerformanceBaseline Baseline;
	Baseline.BaselineName = BaselineName;
	
	FString CaptureTimeStr;
	if (JsonObject->TryGetStringField(TEXT("CaptureTime"), CaptureTimeStr))
	{
		FDateTime::Parse(CaptureTimeStr, Baseline.CaptureTime);
	}
	
	JsonObject->TryGetStringField(TEXT("BuildVersion"), Baseline.BuildVersion);
	JsonObject->TryGetStringField(TEXT("MapName"), Baseline.MapName);
	JsonObject->TryGetNumberField(TEXT("AverageFPS"), Baseline.AverageFPS);
	JsonObject->TryGetNumberField(TEXT("OnePercentLowFPS"), Baseline.OnePercentLowFPS);
	JsonObject->TryGetNumberField(TEXT("TotalFramesCaptured"), Baseline.TotalFramesCaptured);
	
	// Frame data
	const TSharedPtr<FJsonObject>* FrameDataObj;
	if (JsonObject->TryGetObjectField(TEXT("FrameData"), FrameDataObj))
	{
		(*FrameDataObj)->TryGetNumberField(TEXT("FrameTimeMs"), Baseline.AverageFrameData.FrameTimeMs);
		(*FrameDataObj)->TryGetNumberField(TEXT("GameThreadTimeMs"), Baseline.AverageFrameData.GameThreadTimeMs);
		(*FrameDataObj)->TryGetNumberField(TEXT("RenderThreadTimeMs"), Baseline.AverageFrameData.RenderThreadTimeMs);
	}
	
	// System data
	const TArray<TSharedPtr<FJsonValue>>* SystemDataArray;
	if (JsonObject->TryGetArrayField(TEXT("SystemData"), SystemDataArray))
	{
		for (const TSharedPtr<FJsonValue>& Value : *SystemDataArray)
		{
			const TSharedPtr<FJsonObject>* SystemObj;
			if (Value->TryGetObject(SystemObj))
			{
				FSystemPerformanceData SystemData;
				FString SystemNameStr;
				if ((*SystemObj)->TryGetStringField(TEXT("SystemName"), SystemNameStr))
				{
					SystemData.SystemName = FName(*SystemNameStr);
					(*SystemObj)->TryGetNumberField(TEXT("CycleTimeMs"), SystemData.CycleTimeMs);
					(*SystemObj)->TryGetNumberField(TEXT("BudgetTimeMs"), SystemData.BudgetTimeMs);
					(*SystemObj)->TryGetNumberField(TEXT("AverageTimeMs"), SystemData.AverageTimeMs);
					(*SystemObj)->TryGetNumberField(TEXT("PeakTimeMs"), SystemData.PeakTimeMs);
					Baseline.SystemData.Add(SystemData.SystemName, SystemData);
				}
			}
		}
	}
	
	// Memory data
	const TSharedPtr<FJsonObject>* MemoryObj;
	if (JsonObject->TryGetObjectField(TEXT("MemoryData"), MemoryObj))
	{
		double TotalMem, NativeMem, ManagedMem;
		if ((*MemoryObj)->TryGetNumberField(TEXT("TotalMemory"), TotalMem))
		{
			Baseline.MemoryData.TotalMemory = static_cast<uint64>(TotalMem);
		}
		if ((*MemoryObj)->TryGetNumberField(TEXT("NativeMemory"), NativeMem))
		{
			Baseline.MemoryData.NativeMemory = static_cast<uint64>(NativeMem);
		}
		if ((*MemoryObj)->TryGetNumberField(TEXT("ManagedMemory"), ManagedMem))
		{
			Baseline.MemoryData.ManagedMemory = static_cast<uint64>(ManagedMem);
		}
	}

	// Validate loaded baseline
	if (!ValidateBaseline(Baseline, Context))
	{
		UE_LOG(LogDelveDeepTelemetry, Warning,
			TEXT("Baseline validation warnings: %s"),
			*Context.GetReport());
	}

	// Store baseline
	Baselines.Add(BaselineName, Baseline);

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Loaded baseline '%s' from: %s (%.2f FPS, %d systems)"),
		*BaselineName.ToString(),
		*FilePath,
		Baseline.AverageFPS,
		Baseline.SystemData.Num());

	return true;
}

bool UDelveDeepTelemetrySubsystem::DeleteBaseline(FName BaselineName)
{
	if (Baselines.Remove(BaselineName) > 0)
	{
		UE_LOG(LogDelveDeepTelemetry, Display,
			TEXT("Deleted baseline '%s'"),
			*BaselineName.ToString());
		return true;
	}

	UE_LOG(LogDelveDeepTelemetry, Warning,
		TEXT("Baseline '%s' not found"),
		*BaselineName.ToString());
	return false;
}

FString UDelveDeepTelemetrySubsystem::GetDefaultBaselineDirectory() const
{
	return FPaths::ProjectSavedDir() / TEXT("Telemetry") / TEXT("Baselines");
}

bool UDelveDeepTelemetrySubsystem::ValidateBaseline(const FPerformanceBaseline& Baseline, FValidationContext& Context) const
{
	bool bIsValid = true;

	// Check baseline name
	if (Baseline.BaselineName.IsNone())
	{
		Context.AddError(TEXT("Baseline name is empty"));
		bIsValid = false;
	}

	// Check FPS values are reasonable
	if (Baseline.AverageFPS <= 0.0f || Baseline.AverageFPS > 1000.0f)
	{
		Context.AddWarning(FString::Printf(
			TEXT("Average FPS out of expected range: %.2f"),
			Baseline.AverageFPS));
	}

	// Check frame time is reasonable
	if (Baseline.AverageFrameData.FrameTimeMs <= 0.0f || Baseline.AverageFrameData.FrameTimeMs > 1000.0f)
	{
		Context.AddWarning(FString::Printf(
			TEXT("Frame time out of expected range: %.2f ms"),
			Baseline.AverageFrameData.FrameTimeMs));
	}

	// Warn if build version doesn't match
	FString CurrentBuildVersion = FApp::GetBuildVersion();
	if (!Baseline.BuildVersion.IsEmpty() && Baseline.BuildVersion != CurrentBuildVersion)
	{
		Context.AddWarning(FString::Printf(
			TEXT("Baseline build version (%s) differs from current build (%s)"),
			*Baseline.BuildVersion,
			*CurrentBuildVersion));
	}

	// Check system data is present
	if (Baseline.SystemData.Num() == 0)
	{
		Context.AddWarning(TEXT("Baseline contains no system performance data"));
	}

	return bIsValid;
}
