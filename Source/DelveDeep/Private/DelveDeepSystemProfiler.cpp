// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepSystemProfiler.h"
#include "DelveDeepTelemetrySubsystem.h"

FSystemProfiler::FSystemProfiler()
	: TotalViolations(0)
	, CurrentFrame(0)
{
	ViolationHistory.Reserve(MaxViolationHistory);
}

void FSystemProfiler::RegisterSystem(FName SystemName, float BudgetMs)
{
	if (SystemName.IsNone())
	{
		UE_LOG(LogDelveDeepTelemetry, Warning, TEXT("Attempted to register system with empty name"));
		return;
	}

	if (BudgetMs <= 0.0f)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning, 
			TEXT("Invalid budget for system '%s': %.2fms (must be positive)"), 
			*SystemName.ToString(), BudgetMs);
		return;
	}

	// Create or update system data
	FSystemPerformanceData& Data = SystemData.FindOrAdd(SystemName);
	Data.SystemName = SystemName;
	Data.BudgetTimeMs = BudgetMs;

	// Initialize time history
	SystemTimeHistory.FindOrAdd(SystemName).Reserve(AverageFrameCount);

	UE_LOG(LogDelveDeepTelemetry, Display, 
		TEXT("Registered system '%s' with budget %.2fms"), 
		*SystemName.ToString(), BudgetMs);
}

void FSystemProfiler::RecordSystemTime(FName SystemName, double CycleTimeMs)
{
	if (SystemName.IsNone())
	{
		return;
	}

	// Get or create system data
	FSystemPerformanceData* Data = SystemData.Find(SystemName);
	if (!Data)
	{
		// Auto-register with default budget if not registered
		RegisterSystem(SystemName, 2.0f); // Default 2ms budget
		Data = SystemData.Find(SystemName);
	}

	if (Data)
	{
		// Update current cycle time
		Data->CycleTimeMs = CycleTimeMs;
		Data->CallCount++;

		// Update peak time
		if (CycleTimeMs > Data->PeakTimeMs)
		{
			Data->PeakTimeMs = CycleTimeMs;
		}

		// Add to time history for averaging
		TArray<double>& TimeHistory = SystemTimeHistory.FindOrAdd(SystemName);
		TimeHistory.Add(CycleTimeMs);

		// Keep history size limited
		if (TimeHistory.Num() > AverageFrameCount)
		{
			TimeHistory.RemoveAt(0);
		}

		// Update average
		UpdateAverageTime(SystemName);

		// Check for budget violation
		CheckBudgetViolation(SystemName);
	}
}

FSystemPerformanceData FSystemProfiler::GetSystemData(FName SystemName) const
{
	const FSystemPerformanceData* Data = SystemData.Find(SystemName);
	if (Data)
	{
		return *Data;
	}

	// Return empty data if system not found
	return FSystemPerformanceData();
}

TArray<FSystemPerformanceData> FSystemProfiler::GetAllSystemData() const
{
	TArray<FSystemPerformanceData> AllData;
	AllData.Reserve(SystemData.Num());

	for (const auto& Pair : SystemData)
	{
		AllData.Add(Pair.Value);
	}

	// Sort by cycle time (descending) for easier analysis
	AllData.Sort([](const FSystemPerformanceData& A, const FSystemPerformanceData& B)
	{
		return A.CycleTimeMs > B.CycleTimeMs;
	});

	return AllData;
}

float FSystemProfiler::GetBudgetUtilization(FName SystemName) const
{
	const FSystemPerformanceData* Data = SystemData.Find(SystemName);
	if (!Data || Data->BudgetTimeMs <= 0.0)
	{
		return 0.0f;
	}

	return static_cast<float>(Data->CycleTimeMs / Data->BudgetTimeMs);
}

bool FSystemProfiler::IsBudgetViolated(FName SystemName) const
{
	return GetBudgetUtilization(SystemName) > 1.0f;
}

void FSystemProfiler::ResetStatistics()
{
	// Reset all system data
	for (auto& Pair : SystemData)
	{
		FSystemPerformanceData& Data = Pair.Value;
		Data.CycleTimeMs = 0.0;
		Data.CallCount = 0;
		Data.PeakTimeMs = 0.0;
		Data.AverageTimeMs = 0.0;
	}

	// Clear time history
	for (auto& Pair : SystemTimeHistory)
	{
		Pair.Value.Empty();
	}

	// Clear violation history
	ViolationHistory.Empty();
	TotalViolations = 0;
	CurrentFrame = 0;

	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("System profiler statistics reset"));
}

void FSystemProfiler::UpdateFrame()
{
	CurrentFrame++;

	// Reset per-frame counters
	for (auto& Pair : SystemData)
	{
		FSystemPerformanceData& Data = Pair.Value;
		Data.CallCount = 0;
		Data.CycleTimeMs = 0.0;
	}
}

void FSystemProfiler::CheckBudgetViolation(FName SystemName)
{
	const FSystemPerformanceData* Data = SystemData.Find(SystemName);
	if (!Data || Data->BudgetTimeMs <= 0.0)
	{
		return;
	}

	// Check if over budget
	if (Data->CycleTimeMs > Data->BudgetTimeMs)
	{
		// Calculate overage percentage
		const float OveragePercentage = static_cast<float>(
			((Data->CycleTimeMs - Data->BudgetTimeMs) / Data->BudgetTimeMs) * 100.0
		);

		// Create violation record
		FBudgetViolation Violation;
		Violation.SystemName = SystemName;
		Violation.ActualTimeMs = static_cast<float>(Data->CycleTimeMs);
		Violation.BudgetTimeMs = static_cast<float>(Data->BudgetTimeMs);
		Violation.OveragePercentage = OveragePercentage;
		Violation.Timestamp = FDateTime::Now();
		Violation.FrameNumber = static_cast<int32>(CurrentFrame);

		// Record violation
		RecordViolation(Violation);

		// Log warning
		UE_LOG(LogDelveDeepTelemetry, Warning,
			TEXT("Budget violation: System '%s' took %.2fms (budget: %.2fms, %.1f%% over)"),
			*SystemName.ToString(),
			Violation.ActualTimeMs,
			Violation.BudgetTimeMs,
			OveragePercentage);
	}
}

void FSystemProfiler::RecordViolation(const FBudgetViolation& Violation)
{
	// Add to history
	ViolationHistory.Add(Violation);

	// Keep history size limited (circular buffer behavior)
	if (ViolationHistory.Num() > MaxViolationHistory)
	{
		ViolationHistory.RemoveAt(0);
	}

	// Increment total count
	TotalViolations++;
}

void FSystemProfiler::UpdateAverageTime(FName SystemName)
{
	const TArray<double>* TimeHistory = SystemTimeHistory.Find(SystemName);
	if (!TimeHistory || TimeHistory->Num() == 0)
	{
		return;
	}

	// Calculate average
	double TotalTime = 0.0;
	for (double Time : *TimeHistory)
	{
		TotalTime += Time;
	}

	const double AverageTime = TotalTime / TimeHistory->Num();

	// Update system data
	FSystemPerformanceData* Data = SystemData.Find(SystemName);
	if (Data)
	{
		Data->AverageTimeMs = AverageTime;
	}
}
