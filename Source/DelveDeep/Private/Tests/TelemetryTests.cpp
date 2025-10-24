// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "DelveDeepTelemetrySubsystem.h"
#include "DelveDeepFramePerformanceTracker.h"
#include "DelveDeepSystemProfiler.h"
#include "DelveDeepMemoryTracker.h"
#include "DelveDeepPerformanceBaseline.h"
#include "DelveDeepPerformanceReport.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"

/**
 * Unit test: FPS calculation accuracy with known frame times
 * Requirement: 1.1, 1.2
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryFPSCalculationTest,
	"DelveDeep.Telemetry.FPSCalculation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryFPSCalculationTest::RunTest(const FString& Parameters)
{
	FFramePerformanceTracker Tracker;

	// Test 1: Constant 60 FPS (16.67ms per frame)
	const float TargetFrameTime60FPS = 1.0f / 60.0f; // 0.01667 seconds
	for (int32 i = 0; i < 120; ++i)
	{
		Tracker.RecordFrame(TargetFrameTime60FPS);
	}

	float CurrentFPS = Tracker.GetCurrentFPS();
	float AverageFPS = Tracker.GetAverageFPS(60);

	UE_LOG(LogTemp, Display, TEXT("60 FPS Test - Current: %.2f, Average: %.2f"), CurrentFPS, AverageFPS);

	// Allow 1% tolerance for floating point precision
	TestTrue(TEXT("Current FPS should be ~60"), FMath::IsNearlyEqual(CurrentFPS, 60.0f, 0.6f));
	TestTrue(TEXT("Average FPS should be ~60"), FMath::IsNearlyEqual(AverageFPS, 60.0f, 0.6f));

	// Test 2: Constant 30 FPS (33.33ms per frame)
	Tracker.ResetStatistics();
	const float TargetFrameTime30FPS = 1.0f / 30.0f; // 0.03333 seconds
	for (int32 i = 0; i < 120; ++i)
	{
		Tracker.RecordFrame(TargetFrameTime30FPS);
	}

	CurrentFPS = Tracker.GetCurrentFPS();
	AverageFPS = Tracker.GetAverageFPS(60);

	UE_LOG(LogTemp, Display, TEXT("30 FPS Test - Current: %.2f, Average: %.2f"), CurrentFPS, AverageFPS);

	TestTrue(TEXT("Current FPS should be ~30"), FMath::IsNearlyEqual(CurrentFPS, 30.0f, 0.3f));
	TestTrue(TEXT("Average FPS should be ~30"), FMath::IsNearlyEqual(AverageFPS, 30.0f, 0.3f));

	// Test 3: Variable frame times
	Tracker.ResetStatistics();
	TArray<float> VariableFrameTimes = {
		1.0f / 60.0f,  // 60 FPS
		1.0f / 50.0f,  // 50 FPS
		1.0f / 45.0f,  // 45 FPS
		1.0f / 60.0f,  // 60 FPS
		1.0f / 55.0f   // 55 FPS
	};

	for (int32 i = 0; i < 60; ++i)
	{
		Tracker.RecordFrame(VariableFrameTimes[i % VariableFrameTimes.Num()]);
	}

	AverageFPS = Tracker.GetAverageFPS(60);
	UE_LOG(LogTemp, Display, TEXT("Variable FPS Test - Average: %.2f"), AverageFPS);

	// Average should be between 45 and 60
	TestTrue(TEXT("Average FPS should be in expected range"), AverageFPS >= 45.0f && AverageFPS <= 60.0f);

	return true;
}

/**
 * Unit test: Spike detection with various frame time patterns
 * Requirement: 1.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetrySpikeDetectionTest,
	"DelveDeep.Telemetry.SpikeDetection",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetrySpikeDetectionTest::RunTest(const FString& Parameters)
{
	FFramePerformanceTracker Tracker;

	// Test 1: No spikes - consistent 60 FPS
	const float GoodFrameTime = 1.0f / 60.0f; // 16.67ms
	for (int32 i = 0; i < 60; ++i)
	{
		Tracker.RecordFrame(GoodFrameTime);
	}

	TestFalse(TEXT("No spike should be detected with consistent 60 FPS"), Tracker.IsSpikeDetected());

	// Test 2: Single spike
	Tracker.ResetStatistics();
	for (int32 i = 0; i < 30; ++i)
	{
		Tracker.RecordFrame(GoodFrameTime);
	}
	
	// Insert a spike (50ms frame = 20 FPS)
	Tracker.RecordFrame(0.050f);
	
	TestTrue(TEXT("Spike should be detected after slow frame"), Tracker.IsSpikeDetected());

	// Continue with good frames
	for (int32 i = 0; i < 10; ++i)
	{
		Tracker.RecordFrame(GoodFrameTime);
	}

	TestFalse(TEXT("Spike flag should clear after good frames"), Tracker.IsSpikeDetected());

	// Test 3: Multiple consecutive spikes
	Tracker.ResetStatistics();
	for (int32 i = 0; i < 30; ++i)
	{
		Tracker.RecordFrame(GoodFrameTime);
	}

	// Insert three consecutive slow frames (25ms each = 40 FPS)
	for (int32 i = 0; i < 3; ++i)
	{
		Tracker.RecordFrame(0.025f);
	}

	TestTrue(TEXT("Spike should be detected with consecutive slow frames"), Tracker.IsSpikeDetected());

	// Test 4: Borderline case (just at threshold)
	Tracker.ResetStatistics();
	for (int32 i = 0; i < 30; ++i)
	{
		Tracker.RecordFrame(GoodFrameTime);
	}

	// 16.67ms is the threshold for 60 FPS
	Tracker.RecordFrame(0.01667f);
	
	// Should not trigger spike at exactly the threshold
	TestFalse(TEXT("No spike at exactly 60 FPS threshold"), Tracker.IsSpikeDetected());

	// Just over threshold (17ms)
	Tracker.RecordFrame(0.017f);
	
	TestTrue(TEXT("Spike detected just over threshold"), Tracker.IsSpikeDetected());

	return true;
}

/**
 * Unit test: Budget violation detection and logging
 * Requirement: 2.5, 4.3
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryBudgetViolationTest,
	"DelveDeep.Telemetry.BudgetViolation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryBudgetViolationTest::RunTest(const FString& Parameters)
{
	FSystemProfiler Profiler;

	// Register systems with budgets
	Profiler.RegisterSystem(FName("CombatSystem"), 2.0f);  // 2ms budget
	Profiler.RegisterSystem(FName("AISystem"), 3.0f);      // 3ms budget
	Profiler.RegisterSystem(FName("UISystem"), 1.0f);      // 1ms budget

	// Test 1: No violations - systems within budget
	Profiler.RecordSystemTime(FName("CombatSystem"), 1.5);  // 1.5ms (within 2ms budget)
	Profiler.RecordSystemTime(FName("AISystem"), 2.5);      // 2.5ms (within 3ms budget)
	Profiler.RecordSystemTime(FName("UISystem"), 0.8);      // 0.8ms (within 1ms budget)

	TestFalse(TEXT("CombatSystem should not violate budget"), 
		Profiler.IsBudgetViolated(FName("CombatSystem")));
	TestFalse(TEXT("AISystem should not violate budget"), 
		Profiler.IsBudgetViolated(FName("AISystem")));
	TestFalse(TEXT("UISystem should not violate budget"), 
		Profiler.IsBudgetViolated(FName("UISystem")));

	TestEqual(TEXT("No violations should be recorded"), Profiler.GetTotalViolationCount(), 0);

	// Test 2: Single violation
	Profiler.RecordSystemTime(FName("CombatSystem"), 3.5);  // 3.5ms (over 2ms budget)

	TestTrue(TEXT("CombatSystem should violate budget"), 
		Profiler.IsBudgetViolated(FName("CombatSystem")));
	TestEqual(TEXT("One violation should be recorded"), Profiler.GetTotalViolationCount(), 1);

	TArray<FBudgetViolation> Violations = Profiler.GetViolationHistory();
	TestEqual(TEXT("Violation history should contain one entry"), Violations.Num(), 1);

	if (Violations.Num() > 0)
	{
		TestEqual(TEXT("Violation should be for CombatSystem"), 
			Violations[0].SystemName, FName("CombatSystem"));
		TestEqual(TEXT("Actual time should be 3.5ms"), Violations[0].ActualTimeMs, 3.5f);
		TestEqual(TEXT("Budget time should be 2.0ms"), Violations[0].BudgetTimeMs, 2.0f);
		
		float ExpectedOverage = ((3.5f - 2.0f) / 2.0f) * 100.0f; // 75% over budget
		TestTrue(TEXT("Overage percentage should be ~75%"), 
			FMath::IsNearlyEqual(Violations[0].OveragePercentage, ExpectedOverage, 1.0f));
	}

	// Test 3: Multiple violations
	Profiler.RecordSystemTime(FName("AISystem"), 5.0);      // 5ms (over 3ms budget)
	Profiler.RecordSystemTime(FName("UISystem"), 2.0);      // 2ms (over 1ms budget)

	TestEqual(TEXT("Three violations should be recorded"), Profiler.GetTotalViolationCount(), 3);

	Violations = Profiler.GetViolationHistory();
	TestEqual(TEXT("Violation history should contain three entries"), Violations.Num(), 3);

	// Test 4: Budget utilization calculation
	Profiler.ResetStatistics();
	Profiler.RegisterSystem(FName("TestSystem"), 5.0f);  // 5ms budget
	
	Profiler.RecordSystemTime(FName("TestSystem"), 2.5);  // 2.5ms
	float Utilization = Profiler.GetBudgetUtilization(FName("TestSystem"));
	
	TestTrue(TEXT("Budget utilization should be 0.5 (50%)"), 
		FMath::IsNearlyEqual(Utilization, 0.5f, 0.01f));

	Profiler.RecordSystemTime(FName("TestSystem"), 7.5);  // 7.5ms (over budget)
	Utilization = Profiler.GetBudgetUtilization(FName("TestSystem"));
	
	TestTrue(TEXT("Budget utilization should be 1.5 (150%)"), 
		FMath::IsNearlyEqual(Utilization, 1.5f, 0.01f));

	return true;
}

/**
 * Unit test: Memory leak detection algorithm
 * Requirement: 3.3
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryMemoryLeakDetectionTest,
	"DelveDeep.Telemetry.MemoryLeakDetection",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryMemoryLeakDetectionTest::RunTest(const FString& Parameters)
{
	FMemoryTracker Tracker;

	// Test 1: Stable memory usage - no leak
	for (int32 i = 0; i < 200; ++i)
	{
		// Simulate stable memory (small fluctuations)
		Tracker.TrackSystemAllocation(FName("TestSystem"), 1000 + (i % 10));
		
		if (i % 100 == 0)
		{
			Tracker.UpdateMemorySnapshot();
		}
	}

	TestFalse(TEXT("No leak should be detected with stable memory"), Tracker.IsLeakDetected());
	
	float GrowthRate = Tracker.GetMemoryGrowthRate();
	UE_LOG(LogTemp, Display, TEXT("Stable memory growth rate: %.2f MB/min"), GrowthRate);
	
	// Growth rate should be very low (< 1 MB/min)
	TestTrue(TEXT("Growth rate should be minimal"), GrowthRate < 1.0f);

	// Test 2: Gradual memory growth - potential leak
	Tracker.ResetStatistics();
	
	for (int32 i = 0; i < 200; ++i)
	{
		// Simulate growing memory (10KB per iteration)
		Tracker.TrackSystemAllocation(FName("LeakySystem"), 10000);
		
		if (i % 100 == 0)
		{
			Tracker.UpdateMemorySnapshot();
		}
	}

	// After significant growth, leak should be detected
	TestTrue(TEXT("Leak should be detected with growing memory"), Tracker.IsLeakDetected());
	
	GrowthRate = Tracker.GetMemoryGrowthRate();
	UE_LOG(LogTemp, Display, TEXT("Growing memory growth rate: %.2f MB/min"), GrowthRate);
	
	// Growth rate should exceed threshold (>10 MB/min)
	TestTrue(TEXT("Growth rate should exceed threshold"), GrowthRate > 10.0f);

	// Test 3: Memory allocation and deallocation balance
	Tracker.ResetStatistics();
	
	for (int32 i = 0; i < 200; ++i)
	{
		// Allocate and deallocate equal amounts
		Tracker.TrackSystemAllocation(FName("BalancedSystem"), 5000);
		Tracker.TrackSystemDeallocation(FName("BalancedSystem"), 5000);
		
		if (i % 100 == 0)
		{
			Tracker.UpdateMemorySnapshot();
		}
	}

	TestFalse(TEXT("No leak with balanced allocation/deallocation"), Tracker.IsLeakDetected());

	// Test 4: Per-system memory tracking
	Tracker.ResetStatistics();
	
	Tracker.TrackSystemAllocation(FName("System1"), 1000000);  // 1 MB
	Tracker.TrackSystemAllocation(FName("System2"), 2000000);  // 2 MB
	Tracker.TrackSystemAllocation(FName("System3"), 500000);   // 0.5 MB
	
	Tracker.UpdateMemorySnapshot();
	
	uint64 System1Memory = Tracker.GetSystemMemory(FName("System1"));
	uint64 System2Memory = Tracker.GetSystemMemory(FName("System2"));
	uint64 System3Memory = Tracker.GetSystemMemory(FName("System3"));
	
	TestEqual(TEXT("System1 should have 1MB"), System1Memory, (uint64)1000000);
	TestEqual(TEXT("System2 should have 2MB"), System2Memory, (uint64)2000000);
	TestEqual(TEXT("System3 should have 0.5MB"), System3Memory, (uint64)500000);

	return true;
}

/**
 * Unit test: Baseline capture and comparison
 * Requirement: 5.3
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryBaselineTest,
	"DelveDeep.Telemetry.BaselineCapture",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryBaselineTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Simulate some performance data
	TelemetrySubsystem->RegisterSystemBudget(FName("TestSystem"), 2.0f);
	
	// Record some frames
	for (int32 i = 0; i < 60; ++i)
	{
		TelemetrySubsystem->Tick(1.0f / 60.0f);  // 60 FPS
	}

	// Test 1: Capture baseline
	bool bCaptured = TelemetrySubsystem->CaptureBaseline(FName("TestBaseline"));
	TestTrue(TEXT("Baseline should be captured successfully"), bCaptured);

	// Test 2: Verify baseline exists
	TArray<FName> Baselines = TelemetrySubsystem->GetAvailableBaselines();
	TestTrue(TEXT("Baseline should be in available list"), Baselines.Contains(FName("TestBaseline")));

	// Test 3: Get baseline data
	FPerformanceBaseline Baseline;
	bool bFound = TelemetrySubsystem->GetBaseline(FName("TestBaseline"), Baseline);
	TestTrue(TEXT("Baseline should be retrievable"), bFound);

	if (bFound)
	{
		TestEqual(TEXT("Baseline name should match"), Baseline.BaselineName, FName("TestBaseline"));
		TestTrue(TEXT("Baseline should have frame data"), Baseline.AverageFrameData.FrameTimeMs > 0.0f);
	}

	// Test 4: Compare to baseline (with same performance)
	FPerformanceComparison Comparison;
	bool bCompared = TelemetrySubsystem->CompareToBaseline(FName("TestBaseline"), Comparison);
	TestTrue(TEXT("Comparison should succeed"), bCompared);

	if (bCompared)
	{
		// Since we haven't changed performance, differences should be minimal
		TestTrue(TEXT("FPS difference should be small"), 
			FMath::Abs(Comparison.FPSChangePercent) < 5.0f);
	}

	// Test 5: Simulate performance change and compare
	// Record slower frames
	for (int32 i = 0; i < 60; ++i)
	{
		TelemetrySubsystem->Tick(1.0f / 30.0f);  // 30 FPS (slower)
	}

	bCompared = TelemetrySubsystem->CompareToBaseline(FName("TestBaseline"), Comparison);
	TestTrue(TEXT("Comparison after performance change should succeed"), bCompared);

	if (bCompared)
	{
		// FPS should be significantly lower (negative difference)
		TestTrue(TEXT("FPS should be lower than baseline"), Comparison.FPSChangePercent < -10.0f);
		UE_LOG(LogTemp, Display, TEXT("FPS difference: %.2f%%"), Comparison.FPSChangePercent);
	}

	// Test 6: Delete baseline
	bool bDeleted = TelemetrySubsystem->DeleteBaseline(FName("TestBaseline"));
	TestTrue(TEXT("Baseline should be deleted successfully"), bDeleted);

	Baselines = TelemetrySubsystem->GetAvailableBaselines();
	TestFalse(TEXT("Baseline should not be in available list after deletion"), 
		Baselines.Contains(FName("TestBaseline")));

	// Test 7: Invalid baseline name
	bCaptured = TelemetrySubsystem->CaptureBaseline(NAME_None);
	TestFalse(TEXT("Capturing baseline with empty name should fail"), bCaptured);

	return true;
}

/**
 * Unit test: Percentile calculations for frame times
 * Requirement: 1.2, 6.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryPercentileTest,
	"DelveDeep.Telemetry.PercentileCalculation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryPercentileTest::RunTest(const FString& Parameters)
{
	FFramePerformanceTracker Tracker;

	// Create a known distribution of frame times
	// 90 frames at 60 FPS (16.67ms)
	// 9 frames at 30 FPS (33.33ms)
	// 1 frame at 20 FPS (50ms)
	// This gives us: 90% good, 9% medium, 1% bad

	for (int32 i = 0; i < 90; ++i)
	{
		Tracker.RecordFrame(1.0f / 60.0f);  // 16.67ms
	}

	for (int32 i = 0; i < 9; ++i)
	{
		Tracker.RecordFrame(1.0f / 30.0f);  // 33.33ms
	}

	Tracker.RecordFrame(1.0f / 20.0f);  // 50ms

	// Test 1% low FPS (should be around 30 FPS, as 1% of frames are at 20-30 FPS)
	float OnePercentLow = Tracker.GetOnePercentLowFPS();
	UE_LOG(LogTemp, Display, TEXT("1%% low FPS: %.2f"), OnePercentLow);

	// 1% low should be between 20-35 FPS (the worst 1% of frames)
	TestTrue(TEXT("1% low FPS should be in expected range"), 
		OnePercentLow >= 20.0f && OnePercentLow <= 35.0f);

	// Test 0.1% low FPS (should be around 20 FPS, the absolute worst frame)
	float PointOnePercentLow = Tracker.GetPointOnePercentLowFPS();
	UE_LOG(LogTemp, Display, TEXT("0.1%% low FPS: %.2f"), PointOnePercentLow);

	// 0.1% low should be close to 20 FPS (the worst frame)
	TestTrue(TEXT("0.1% low FPS should be close to worst frame"), 
		PointOnePercentLow >= 18.0f && PointOnePercentLow <= 25.0f);

	// Test with consistent frame times
	Tracker.ResetStatistics();
	for (int32 i = 0; i < 100; ++i)
	{
		Tracker.RecordFrame(1.0f / 60.0f);  // All frames at 60 FPS
	}

	OnePercentLow = Tracker.GetOnePercentLowFPS();
	PointOnePercentLow = Tracker.GetPointOnePercentLowFPS();
	float AverageFPS = Tracker.GetAverageFPS();

	UE_LOG(LogTemp, Display, TEXT("Consistent frames - Average: %.2f, 1%% low: %.2f, 0.1%% low: %.2f"),
		AverageFPS, OnePercentLow, PointOnePercentLow);

	// With consistent frames, all percentiles should be similar
	TestTrue(TEXT("1% low should be close to average with consistent frames"),
		FMath::Abs(OnePercentLow - AverageFPS) < 5.0f);
	TestTrue(TEXT("0.1% low should be close to average with consistent frames"),
		FMath::Abs(PointOnePercentLow - AverageFPS) < 5.0f);

	return true;
}

/**
 * Unit test: Report generation with sample data
 * Requirement: 6.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryReportGenerationTest,
	"DelveDeep.Telemetry.ReportGeneration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryReportGenerationTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Register systems with budgets
	TelemetrySubsystem->RegisterSystemBudget(FName("CombatSystem"), 2.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("AISystem"), 3.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("UISystem"), 1.0f);

	// Simulate gameplay with varying performance
	for (int32 i = 0; i < 300; ++i)  // 5 seconds at 60 FPS
	{
		// Vary frame times slightly
		float FrameTime = (1.0f / 60.0f) + (FMath::FRand() * 0.002f);
		TelemetrySubsystem->Tick(FrameTime);

		// Track some entities
		TelemetrySubsystem->TrackEntityCount(FName("Monsters"), 10 + (i % 20));
		TelemetrySubsystem->TrackEntityCount(FName("Projectiles"), 5 + (i % 10));
	}

	// Generate report
	FPerformanceReport Report;
	bool bGenerated = TelemetrySubsystem->GeneratePerformanceReport(Report, 5.0f);
	
	TestTrue(TEXT("Report should be generated successfully"), bGenerated);

	if (bGenerated)
	{
		// Verify report contains expected data
		TestTrue(TEXT("Report should have valid generation time"), 
			Report.GenerationTime > FDateTime::MinValue());
		
		TestTrue(TEXT("Report duration should be ~5 seconds"), 
			FMath::IsNearlyEqual(Report.DurationSeconds, 5.0f, 0.5f));

		TestTrue(TEXT("Report should have average FPS"), Report.AverageFPS > 0.0f);
		TestTrue(TEXT("Report should have min FPS"), Report.MinFPS > 0.0f);
		TestTrue(TEXT("Report should have max FPS"), Report.MaxFPS > 0.0f);
		TestTrue(TEXT("Report should have 1% low FPS"), Report.OnePercentLowFPS > 0.0f);

		TestTrue(TEXT("Report should have total frames"), Report.TotalFrames > 0);
		
		// With 5 seconds at ~60 FPS, we should have ~300 frames
		TestTrue(TEXT("Total frames should be ~300"), 
			Report.TotalFrames >= 250 && Report.TotalFrames <= 350);

		TestTrue(TEXT("Report should have system breakdown"), 
			Report.SystemBreakdown.Num() > 0);

		TestTrue(TEXT("Report should have peak monster count"), 
			Report.PeakMonsterCount > 0);

		UE_LOG(LogTemp, Display, TEXT("Report Summary:"));
		UE_LOG(LogTemp, Display, TEXT("  Duration: %.2f seconds"), Report.DurationSeconds);
		UE_LOG(LogTemp, Display, TEXT("  Total Frames: %d"), Report.TotalFrames);
		UE_LOG(LogTemp, Display, TEXT("  Average FPS: %.2f"), Report.AverageFPS);
		UE_LOG(LogTemp, Display, TEXT("  Min FPS: %.2f"), Report.MinFPS);
		UE_LOG(LogTemp, Display, TEXT("  Max FPS: %.2f"), Report.MaxFPS);
		UE_LOG(LogTemp, Display, TEXT("  1%% Low FPS: %.2f"), Report.OnePercentLowFPS);
		UE_LOG(LogTemp, Display, TEXT("  Spike Count: %d"), Report.SpikeCount);
		UE_LOG(LogTemp, Display, TEXT("  Peak Monsters: %d"), Report.PeakMonsterCount);
		UE_LOG(LogTemp, Display, TEXT("  Peak Projectiles: %d"), Report.PeakProjectileCount);
	}

	// Test report export (just verify the function doesn't crash)
	FString TestCSVPath = FPaths::ProjectSavedDir() / TEXT("Telemetry") / TEXT("test_report.csv");
	bool bExported = TelemetrySubsystem->ExportReportToCSV(Report, TestCSVPath);
	
	if (bExported)
	{
		UE_LOG(LogTemp, Display, TEXT("Report exported to CSV: %s"), *TestCSVPath);
	}

	FString TestJSONPath = FPaths::ProjectSavedDir() / TEXT("Telemetry") / TEXT("test_report.json");
	bExported = TelemetrySubsystem->ExportReportToJSON(Report, TestJSONPath);
	
	if (bExported)
	{
		UE_LOG(LogTemp, Display, TEXT("Report exported to JSON: %s"), *TestJSONPath);
	}

	return true;
}
