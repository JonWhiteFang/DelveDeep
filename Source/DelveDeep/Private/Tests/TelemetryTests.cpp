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
 * Performance test: Telemetry overhead per frame
 * Target: <0.5ms per frame
 * Requirement: 1.3
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryOverheadTest,
	"DelveDeep.Telemetry.Performance.FrameOverhead",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryOverheadTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Register several systems to simulate realistic load
	TelemetrySubsystem->RegisterSystemBudget(FName("CombatSystem"), 2.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("AISystem"), 3.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("UISystem"), 1.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("EventSystem"), 1.5f);
	TelemetrySubsystem->RegisterSystemBudget(FName("ConfigSystem"), 0.5f);

	// Warm up - run a few frames to initialize caches
	for (int32 i = 0; i < 10; ++i)
	{
		TelemetrySubsystem->Tick(1.0f / 60.0f);
	}

	// Measure telemetry overhead over 100 frames
	const int32 FrameCount = 100;
	double TotalOverheadMs = 0.0;

	for (int32 i = 0; i < FrameCount; ++i)
	{
		double StartTime = FPlatformTime::Seconds();
		
		// Telemetry tick (this is what we're measuring)
		TelemetrySubsystem->Tick(1.0f / 60.0f);
		
		// Track some entities to simulate realistic usage
		TelemetrySubsystem->TrackEntityCount(FName("Monsters"), 50);
		TelemetrySubsystem->TrackEntityCount(FName("Projectiles"), 20);
		
		double EndTime = FPlatformTime::Seconds();
		double FrameOverheadMs = (EndTime - StartTime) * 1000.0;
		TotalOverheadMs += FrameOverheadMs;
	}

	double AvgOverheadMs = TotalOverheadMs / FrameCount;

	UE_LOG(LogTemp, Display, TEXT("Telemetry Performance:"));
	UE_LOG(LogTemp, Display, TEXT("  Total frames measured: %d"), FrameCount);
	UE_LOG(LogTemp, Display, TEXT("  Total overhead: %.4f ms"), TotalOverheadMs);
	UE_LOG(LogTemp, Display, TEXT("  Average overhead per frame: %.4f ms"), AvgOverheadMs);

	// Test against target (<0.5ms per frame)
	TestTrue(FString::Printf(TEXT("Telemetry overhead < 0.5ms per frame (actual: %.4f ms)"), AvgOverheadMs),
		AvgOverheadMs < 0.5);

	// Warn if approaching threshold
	if (AvgOverheadMs > 0.3 && AvgOverheadMs < 0.5)
	{
		AddWarning(FString::Printf(TEXT("Telemetry overhead approaching threshold: %.4f ms"), AvgOverheadMs));
	}

	return true;
}

/**
 * Performance test: Overlay rendering overhead
 * Target: <0.1ms per frame
 * Requirement: 7.2
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryOverlayOverheadTest,
	"DelveDeep.Telemetry.Performance.OverlayOverhead",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryOverlayOverheadTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Generate some performance data
	for (int32 i = 0; i < 120; ++i)
	{
		TelemetrySubsystem->Tick(1.0f / 60.0f);
	}

	// Test overlay rendering overhead for each mode
	TArray<EOverlayMode> ModesToTest = {
		EOverlayMode::Minimal,
		EOverlayMode::Standard,
		EOverlayMode::Detailed
	};

	for (EOverlayMode Mode : ModesToTest)
	{
		TelemetrySubsystem->EnablePerformanceOverlay(Mode);
		
		// Warm up
		for (int32 i = 0; i < 10; ++i)
		{
			TelemetrySubsystem->RenderOverlay(nullptr);  // Pass nullptr for test (no actual rendering)
		}

		// Measure rendering overhead
		const int32 RenderCount = 100;
		double TotalRenderTimeMs = 0.0;

		for (int32 i = 0; i < RenderCount; ++i)
		{
			double StartTime = FPlatformTime::Seconds();
			
			TelemetrySubsystem->RenderOverlay(nullptr);
			
			double EndTime = FPlatformTime::Seconds();
			TotalRenderTimeMs += (EndTime - StartTime) * 1000.0;
		}

		double AvgRenderTimeMs = TotalRenderTimeMs / RenderCount;

		FString ModeName;
		switch (Mode)
		{
		case EOverlayMode::Minimal:
			ModeName = TEXT("Minimal");
			break;
		case EOverlayMode::Standard:
			ModeName = TEXT("Standard");
			break;
		case EOverlayMode::Detailed:
			ModeName = TEXT("Detailed");
			break;
		}

		UE_LOG(LogTemp, Display, TEXT("Overlay Rendering Performance (%s mode):"), *ModeName);
		UE_LOG(LogTemp, Display, TEXT("  Average render time: %.4f ms"), AvgRenderTimeMs);

		// Test against target (<0.1ms per frame)
		TestTrue(FString::Printf(TEXT("Overlay rendering (%s) < 0.1ms (actual: %.4f ms)"), 
			*ModeName, AvgRenderTimeMs),
			AvgRenderTimeMs < 0.1);
	}

	TelemetrySubsystem->DisablePerformanceOverlay();

	return true;
}

/**
 * Performance test: Memory snapshot capture time
 * Target: <1ms per capture
 * Requirement: 1.3
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryMemorySnapshotPerformanceTest,
	"DelveDeep.Telemetry.Performance.MemorySnapshot",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryMemorySnapshotPerformanceTest::RunTest(const FString& Parameters)
{
	FMemoryTracker Tracker;

	// Simulate some memory allocations across multiple systems
	Tracker.TrackSystemAllocation(FName("CombatSystem"), 1000000);   // 1 MB
	Tracker.TrackSystemAllocation(FName("AISystem"), 2000000);       // 2 MB
	Tracker.TrackSystemAllocation(FName("UISystem"), 500000);        // 0.5 MB
	Tracker.TrackSystemAllocation(FName("EventSystem"), 750000);     // 0.75 MB
	Tracker.TrackSystemAllocation(FName("ConfigSystem"), 300000);    // 0.3 MB

	// Measure snapshot capture time
	const int32 SnapshotCount = 100;
	double TotalCaptureTimeMs = 0.0;

	for (int32 i = 0; i < SnapshotCount; ++i)
	{
		double StartTime = FPlatformTime::Seconds();
		
		Tracker.UpdateMemorySnapshot();
		
		double EndTime = FPlatformTime::Seconds();
		TotalCaptureTimeMs += (EndTime - StartTime) * 1000.0;
	}

	double AvgCaptureTimeMs = TotalCaptureTimeMs / SnapshotCount;

	UE_LOG(LogTemp, Display, TEXT("Memory Snapshot Performance:"));
	UE_LOG(LogTemp, Display, TEXT("  Total snapshots: %d"), SnapshotCount);
	UE_LOG(LogTemp, Display, TEXT("  Total capture time: %.4f ms"), TotalCaptureTimeMs);
	UE_LOG(LogTemp, Display, TEXT("  Average capture time: %.4f ms"), AvgCaptureTimeMs);

	// Test against target (<1ms per capture)
	TestTrue(FString::Printf(TEXT("Memory snapshot capture < 1ms (actual: %.4f ms)"), AvgCaptureTimeMs),
		AvgCaptureTimeMs < 1.0);

	// Verify snapshot contains expected data
	FMemorySnapshot Snapshot = Tracker.GetCurrentSnapshot();
	TestTrue(TEXT("Snapshot should have total memory"), Snapshot.TotalMemoryBytes > 0);
	TestTrue(TEXT("Snapshot should have per-system data"), Snapshot.PerSystemMemory.Num() > 0);

	return true;
}

/**
 * Performance test: Report generation time
 * Target: <100ms for 5-minute data
 * Requirement: 8.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryReportGenerationPerformanceTest,
	"DelveDeep.Telemetry.Performance.ReportGeneration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryReportGenerationPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Register systems
	TelemetrySubsystem->RegisterSystemBudget(FName("CombatSystem"), 2.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("AISystem"), 3.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("UISystem"), 1.0f);

	// Simulate 5 minutes of gameplay at 60 FPS (18,000 frames)
	const int32 TotalFrames = 18000;
	UE_LOG(LogTemp, Display, TEXT("Simulating %d frames (5 minutes at 60 FPS)..."), TotalFrames);

	for (int32 i = 0; i < TotalFrames; ++i)
	{
		// Vary frame times slightly
		float FrameTime = (1.0f / 60.0f) + (FMath::FRand() * 0.002f);
		TelemetrySubsystem->Tick(FrameTime);

		// Track entities periodically
		if (i % 10 == 0)
		{
			TelemetrySubsystem->TrackEntityCount(FName("Monsters"), 10 + (i % 50));
			TelemetrySubsystem->TrackEntityCount(FName("Projectiles"), 5 + (i % 30));
		}
	}

	UE_LOG(LogTemp, Display, TEXT("Simulation complete. Generating report..."));

	// Measure report generation time
	double StartTime = FPlatformTime::Seconds();
	
	FPerformanceReport Report;
	bool bGenerated = TelemetrySubsystem->GeneratePerformanceReport(Report, 300.0f);  // 5 minutes
	
	double EndTime = FPlatformTime::Seconds();
	double GenerationTimeMs = (EndTime - StartTime) * 1000.0;

	TestTrue(TEXT("Report should be generated successfully"), bGenerated);

	UE_LOG(LogTemp, Display, TEXT("Report Generation Performance:"));
	UE_LOG(LogTemp, Display, TEXT("  Data duration: %.2f seconds"), Report.DurationSeconds);
	UE_LOG(LogTemp, Display, TEXT("  Total frames: %d"), Report.TotalFrames);
	UE_LOG(LogTemp, Display, TEXT("  Generation time: %.2f ms"), GenerationTimeMs);

	// Test against target (<100ms for 5-minute data)
	TestTrue(FString::Printf(TEXT("Report generation < 100ms (actual: %.2f ms)"), GenerationTimeMs),
		GenerationTimeMs < 100.0);

	// Verify report contains expected data
	if (bGenerated)
	{
		TestTrue(TEXT("Report should have frames"), Report.TotalFrames > 0);
		TestTrue(TEXT("Report should have average FPS"), Report.AverageFPS > 0.0f);
		TestTrue(TEXT("Report should have system breakdown"), Report.SystemBreakdown.Num() > 0);
	}

	return true;
}

/**
 * Performance test: Telemetry memory footprint
 * Verifies telemetry stays within memory budget
 * Requirement: 1.3
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryMemoryFootprintTest,
	"DelveDeep.Telemetry.Performance.MemoryFootprint",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryMemoryFootprintTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Get initial memory usage
	FPlatformMemoryStats InitialStats = FPlatformMemory::GetStats();
	uint64 InitialMemory = InitialStats.UsedPhysical;

	// Register systems and run for a while
	TelemetrySubsystem->RegisterSystemBudget(FName("CombatSystem"), 2.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("AISystem"), 3.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("UISystem"), 1.0f);

	// Run for 1 minute at 60 FPS (3600 frames)
	for (int32 i = 0; i < 3600; ++i)
	{
		TelemetrySubsystem->Tick(1.0f / 60.0f);
		
		if (i % 10 == 0)
		{
			TelemetrySubsystem->TrackEntityCount(FName("Monsters"), 50);
			TelemetrySubsystem->TrackEntityCount(FName("Projectiles"), 20);
		}
	}

	// Get final memory usage
	FPlatformMemoryStats FinalStats = FPlatformMemory::GetStats();
	uint64 FinalMemory = FinalStats.UsedPhysical;

	// Calculate telemetry memory usage (approximate)
	int64 MemoryDelta = static_cast<int64>(FinalMemory) - static_cast<int64>(InitialMemory);
	double MemoryDeltaMB = MemoryDelta / (1024.0 * 1024.0);

	UE_LOG(LogTemp, Display, TEXT("Telemetry Memory Footprint:"));
	UE_LOG(LogTemp, Display, TEXT("  Initial memory: %.2f MB"), InitialMemory / (1024.0 * 1024.0));
	UE_LOG(LogTemp, Display, TEXT("  Final memory: %.2f MB"), FinalMemory / (1024.0 * 1024.0));
	UE_LOG(LogTemp, Display, TEXT("  Memory delta: %.2f MB"), MemoryDeltaMB);

	// Telemetry should use less than 5 MB for 1 minute of data
	// (Frame history ~14KB + System data ~5KB + Memory history ~10KB + overhead)
	TestTrue(FString::Printf(TEXT("Telemetry memory footprint < 5 MB (actual: %.2f MB)"), MemoryDeltaMB),
		MemoryDeltaMB < 5.0);

	// Warn if memory usage is high
	if (MemoryDeltaMB > 3.0 && MemoryDeltaMB < 5.0)
	{
		AddWarning(FString::Printf(TEXT("Telemetry memory usage approaching budget: %.2f MB"), MemoryDeltaMB));
	}

	return true;
}

/**
 * Performance test: Profiling session overhead
 * Verifies profiling doesn't significantly impact performance
 * Requirement: 8.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryProfilingSessionOverheadTest,
	"DelveDeep.Telemetry.Performance.ProfilingOverhead",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryProfilingSessionOverheadTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Measure baseline overhead (no profiling)
	const int32 FrameCount = 100;
	double BaselineOverheadMs = 0.0;

	for (int32 i = 0; i < FrameCount; ++i)
	{
		double StartTime = FPlatformTime::Seconds();
		TelemetrySubsystem->Tick(1.0f / 60.0f);
		double EndTime = FPlatformTime::Seconds();
		BaselineOverheadMs += (EndTime - StartTime) * 1000.0;
	}

	double AvgBaselineOverheadMs = BaselineOverheadMs / FrameCount;

	// Start profiling session
	TelemetrySubsystem->StartProfilingSession(FName("TestSession"));

	// Measure overhead with profiling active
	double ProfilingOverheadMs = 0.0;

	for (int32 i = 0; i < FrameCount; ++i)
	{
		double StartTime = FPlatformTime::Seconds();
		TelemetrySubsystem->Tick(1.0f / 60.0f);
		double EndTime = FPlatformTime::Seconds();
		ProfilingOverheadMs += (EndTime - StartTime) * 1000.0;
	}

	double AvgProfilingOverheadMs = ProfilingOverheadMs / FrameCount;

	// Stop profiling
	TelemetrySubsystem->StopProfilingSession();

	// Calculate additional overhead from profiling
	double AdditionalOverheadMs = AvgProfilingOverheadMs - AvgBaselineOverheadMs;
	double OverheadIncreasePercent = (AdditionalOverheadMs / AvgBaselineOverheadMs) * 100.0;

	UE_LOG(LogTemp, Display, TEXT("Profiling Session Overhead:"));
	UE_LOG(LogTemp, Display, TEXT("  Baseline overhead: %.4f ms"), AvgBaselineOverheadMs);
	UE_LOG(LogTemp, Display, TEXT("  Profiling overhead: %.4f ms"), AvgProfilingOverheadMs);
	UE_LOG(LogTemp, Display, TEXT("  Additional overhead: %.4f ms"), AdditionalOverheadMs);
	UE_LOG(LogTemp, Display, TEXT("  Overhead increase: %.2f%%"), OverheadIncreasePercent);

	// Profiling should add less than 0.2ms overhead per frame
	TestTrue(FString::Printf(TEXT("Profiling additional overhead < 0.2ms (actual: %.4f ms)"), 
		AdditionalOverheadMs),
		AdditionalOverheadMs < 0.2);

	// Overhead increase should be less than 50%
	TestTrue(FString::Printf(TEXT("Profiling overhead increase < 50%% (actual: %.2f%%)"), 
		OverheadIncreasePercent),
		OverheadIncreasePercent < 50.0);

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


/**
 * Integration test: Telemetry tracking of event system performance
 * Requirement: 2.1, 2.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryEventSystemIntegrationTest,
	"DelveDeep.Telemetry.Integration.EventSystem",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryEventSystemIntegrationTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystems
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	UDelveDeepEventSubsystem* EventSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepEventSubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);
	TestNotNull(TEXT("EventSubsystem should be available"), EventSubsystem);

	// Register event system budget
	TelemetrySubsystem->RegisterSystemBudget(FName("EventSystem"), 1.5f);  // 1.5ms budget

	// Simulate event system activity
	for (int32 i = 0; i < 100; ++i)
	{
		// Simulate event broadcasts
		double StartTime = FPlatformTime::Seconds();
		
		// Broadcast some test events
		if (EventSubsystem)
		{
			FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("Test.Event"));
			FDelveDeepEventPayload Payload;
			EventSubsystem->BroadcastEvent(TestTag, Payload);
		}
		
		double EndTime = FPlatformTime::Seconds();
		double EventTimeMs = (EndTime - StartTime) * 1000.0;
		
		// Record event system time in telemetry
		TelemetrySubsystem->RecordSystemTime(FName("EventSystem"), EventTimeMs);
		
		// Tick telemetry
		TelemetrySubsystem->Tick(1.0f / 60.0f);
	}

	// Get event system performance data
	FSystemPerformanceData EventSystemData = 
		TelemetrySubsystem->GetSystemPerformance(FName("EventSystem"));

	UE_LOG(LogTemp, Display, TEXT("Event System Integration:"));
	UE_LOG(LogTemp, Display, TEXT("  Average cycle time: %.4f ms"), EventSystemData.AverageCycleTimeMs);
	UE_LOG(LogTemp, Display, TEXT("  Peak cycle time: %.4f ms"), EventSystemData.PeakCycleTimeMs);
	UE_LOG(LogTemp, Display, TEXT("  Call count: %d"), EventSystemData.CallCount);
	UE_LOG(LogTemp, Display, TEXT("  Budget utilization: %.2f%%"), 
		TelemetrySubsystem->GetSystemBudgetUtilization(FName("EventSystem")) * 100.0f);

	// Verify telemetry tracked event system performance
	TestTrue(TEXT("Event system should have recorded calls"), EventSystemData.CallCount > 0);
	TestTrue(TEXT("Event system should have cycle time"), EventSystemData.AverageCycleTimeMs >= 0.0f);

	// Event system should stay within budget for simple events
	float BudgetUtilization = TelemetrySubsystem->GetSystemBudgetUtilization(FName("EventSystem"));
	TestTrue(TEXT("Event system should stay within budget"), BudgetUtilization <= 1.0f);

	return true;
}

/**
 * Integration test: Telemetry tracking of configuration system performance
 * Requirement: 2.1, 2.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryConfigSystemIntegrationTest,
	"DelveDeep.Telemetry.Integration.ConfigSystem",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryConfigSystemIntegrationTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystems
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	UDelveDeepConfigurationManager* ConfigManager = 
		GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);
	TestNotNull(TEXT("ConfigurationManager should be available"), ConfigManager);

	// Register config system budget
	TelemetrySubsystem->RegisterSystemBudget(FName("ConfigSystem"), 0.5f);  // 0.5ms budget

	// Simulate configuration queries
	TArray<FName> TestAssets = {
		FName("DA_Character_Warrior"),
		FName("DA_Weapon_Sword"),
		FName("DA_Ability_Cleave"),
		FName("DA_Upgrade_HealthBoost")
	};

	for (int32 i = 0; i < 100; ++i)
	{
		// Simulate config queries
		double StartTime = FPlatformTime::Seconds();
		
		// Query configuration data
		if (ConfigManager)
		{
			FName AssetName = TestAssets[i % TestAssets.Num()];
			ConfigManager->GetCharacterData(AssetName);
		}
		
		double EndTime = FPlatformTime::Seconds();
		double ConfigTimeMs = (EndTime - StartTime) * 1000.0;
		
		// Record config system time in telemetry
		TelemetrySubsystem->RecordSystemTime(FName("ConfigSystem"), ConfigTimeMs);
		
		// Tick telemetry
		TelemetrySubsystem->Tick(1.0f / 60.0f);
	}

	// Get config system performance data
	FSystemPerformanceData ConfigSystemData = 
		TelemetrySubsystem->GetSystemPerformance(FName("ConfigSystem"));

	UE_LOG(LogTemp, Display, TEXT("Config System Integration:"));
	UE_LOG(LogTemp, Display, TEXT("  Average cycle time: %.4f ms"), ConfigSystemData.AverageCycleTimeMs);
	UE_LOG(LogTemp, Display, TEXT("  Peak cycle time: %.4f ms"), ConfigSystemData.PeakCycleTimeMs);
	UE_LOG(LogTemp, Display, TEXT("  Call count: %d"), ConfigSystemData.CallCount);
	UE_LOG(LogTemp, Display, TEXT("  Budget utilization: %.2f%%"), 
		TelemetrySubsystem->GetSystemBudgetUtilization(FName("ConfigSystem")) * 100.0f);

	// Verify telemetry tracked config system performance
	TestTrue(TEXT("Config system should have recorded calls"), ConfigSystemData.CallCount > 0);
	TestTrue(TEXT("Config system should have cycle time"), ConfigSystemData.AverageCycleTimeMs >= 0.0f);

	// Config system should stay well within budget (queries are fast)
	float BudgetUtilization = TelemetrySubsystem->GetSystemBudgetUtilization(FName("ConfigSystem"));
	TestTrue(TEXT("Config system should stay within budget"), BudgetUtilization <= 1.0f);

	// With caching, average query time should be very low
	TestTrue(TEXT("Config system average time should be < 0.1ms"), 
		ConfigSystemData.AverageCycleTimeMs < 0.1f);

	return true;
}

/**
 * Integration test: Budget allocations sum correctly across all systems
 * Requirement: 2.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryBudgetAllocationTest,
	"DelveDeep.Telemetry.Integration.BudgetAllocation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryBudgetAllocationTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Register systems with budgets
	TMap<FName, float> SystemBudgets;
	SystemBudgets.Add(FName("CombatSystem"), 2.0f);
	SystemBudgets.Add(FName("AISystem"), 3.0f);
	SystemBudgets.Add(FName("UISystem"), 1.0f);
	SystemBudgets.Add(FName("EventSystem"), 1.5f);
	SystemBudgets.Add(FName("ConfigSystem"), 0.5f);
	SystemBudgets.Add(FName("WorldSystem"), 2.5f);

	for (const auto& Budget : SystemBudgets)
	{
		TelemetrySubsystem->RegisterSystemBudget(Budget.Key, Budget.Value);
	}

	// Calculate total budget
	float TotalBudget = 0.0f;
	for (const auto& Budget : SystemBudgets)
	{
		TotalBudget += Budget.Value;
	}

	UE_LOG(LogTemp, Display, TEXT("Budget Allocation Test:"));
	UE_LOG(LogTemp, Display, TEXT("  Total budget: %.2f ms"), TotalBudget);

	// Total budget should be less than frame time target (16.67ms for 60 FPS)
	const float TargetFrameTimeMs = 16.67f;
	TestTrue(FString::Printf(TEXT("Total budget < 16.67ms (actual: %.2f ms)"), TotalBudget),
		TotalBudget < TargetFrameTimeMs);

	// Verify each system has its budget registered
	for (const auto& Budget : SystemBudgets)
	{
		FSystemPerformanceData SystemData = TelemetrySubsystem->GetSystemPerformance(Budget.Key);
		TestEqual(FString::Printf(TEXT("%s budget should be %.2f ms"), 
			*Budget.Key.ToString(), Budget.Value),
			SystemData.BudgetTimeMs, Budget.Value);
	}

	// Simulate realistic system usage
	TelemetrySubsystem->RecordSystemTime(FName("CombatSystem"), 1.8f);   // 90% of budget
	TelemetrySubsystem->RecordSystemTime(FName("AISystem"), 2.7f);       // 90% of budget
	TelemetrySubsystem->RecordSystemTime(FName("UISystem"), 0.9f);       // 90% of budget
	TelemetrySubsystem->RecordSystemTime(FName("EventSystem"), 1.2f);    // 80% of budget
	TelemetrySubsystem->RecordSystemTime(FName("ConfigSystem"), 0.3f);   // 60% of budget
	TelemetrySubsystem->RecordSystemTime(FName("WorldSystem"), 2.0f);    // 80% of budget

	// Calculate total actual time
	float TotalActualTime = 1.8f + 2.7f + 0.9f + 1.2f + 0.3f + 2.0f;

	UE_LOG(LogTemp, Display, TEXT("  Total actual time: %.2f ms"), TotalActualTime);
	UE_LOG(LogTemp, Display, TEXT("  Total budget utilization: %.2f%%"), 
		(TotalActualTime / TotalBudget) * 100.0f);

	// Total actual time should be less than total budget
	TestTrue(FString::Printf(TEXT("Total actual time < total budget (%.2f ms < %.2f ms)"), 
		TotalActualTime, TotalBudget),
		TotalActualTime < TotalBudget);

	// Total actual time should be less than frame time target
	TestTrue(FString::Printf(TEXT("Total actual time < 16.67ms (actual: %.2f ms)"), TotalActualTime),
		TotalActualTime < TargetFrameTimeMs);

	return true;
}

/**
 * Integration test: Performance under realistic gameplay load
 * Requirement: 2.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryRealisticLoadTest,
	"DelveDeep.Telemetry.Integration.RealisticLoad",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryRealisticLoadTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Register all major systems
	TelemetrySubsystem->RegisterSystemBudget(FName("CombatSystem"), 2.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("AISystem"), 3.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("UISystem"), 1.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("EventSystem"), 1.5f);
	TelemetrySubsystem->RegisterSystemBudget(FName("ConfigSystem"), 0.5f);
	TelemetrySubsystem->RegisterSystemBudget(FName("WorldSystem"), 2.5f);

	// Simulate 10 seconds of gameplay at 60 FPS
	const int32 TotalFrames = 600;
	int32 BudgetViolationCount = 0;

	for (int32 i = 0; i < TotalFrames; ++i)
	{
		// Simulate varying system loads
		float CombatLoad = 1.5f + (FMath::FRand() * 0.8f);  // 1.5-2.3ms
		float AILoad = 2.0f + (FMath::FRand() * 1.5f);      // 2.0-3.5ms
		float UILoad = 0.7f + (FMath::FRand() * 0.5f);      // 0.7-1.2ms
		float EventLoad = 1.0f + (FMath::FRand() * 0.8f);   // 1.0-1.8ms
		float ConfigLoad = 0.2f + (FMath::FRand() * 0.3f);  // 0.2-0.5ms
		float WorldLoad = 1.8f + (FMath::FRand() * 1.0f);   // 1.8-2.8ms

		TelemetrySubsystem->RecordSystemTime(FName("CombatSystem"), CombatLoad);
		TelemetrySubsystem->RecordSystemTime(FName("AISystem"), AILoad);
		TelemetrySubsystem->RecordSystemTime(FName("UISystem"), UILoad);
		TelemetrySubsystem->RecordSystemTime(FName("EventSystem"), EventLoad);
		TelemetrySubsystem->RecordSystemTime(FName("ConfigSystem"), ConfigLoad);
		TelemetrySubsystem->RecordSystemTime(FName("WorldSystem"), WorldLoad);

		// Track entities
		int32 MonsterCount = 20 + (i % 30);
		int32 ProjectileCount = 10 + (i % 20);
		int32 ParticleCount = 50 + (i % 50);

		TelemetrySubsystem->TrackEntityCount(FName("Monsters"), MonsterCount);
		TelemetrySubsystem->TrackEntityCount(FName("Projectiles"), ProjectileCount);
		TelemetrySubsystem->TrackEntityCount(FName("Particles"), ParticleCount);

		// Calculate total frame time
		float TotalFrameTime = CombatLoad + AILoad + UILoad + EventLoad + ConfigLoad + WorldLoad;
		float FrameTime = TotalFrameTime / 1000.0f;  // Convert to seconds

		// Tick telemetry
		TelemetrySubsystem->Tick(FrameTime);

		// Check for budget violations
		if (TelemetrySubsystem->IsBudgetViolated(FName("CombatSystem")) ||
			TelemetrySubsystem->IsBudgetViolated(FName("AISystem")) ||
			TelemetrySubsystem->IsBudgetViolated(FName("UISystem")) ||
			TelemetrySubsystem->IsBudgetViolated(FName("EventSystem")) ||
			TelemetrySubsystem->IsBudgetViolated(FName("ConfigSystem")) ||
			TelemetrySubsystem->IsBudgetViolated(FName("WorldSystem")))
		{
			BudgetViolationCount++;
		}
	}

	// Get performance summary
	float CurrentFPS = TelemetrySubsystem->GetCurrentFPS();
	float AverageFPS = TelemetrySubsystem->GetAverageFPS();
	float OnePercentLow = TelemetrySubsystem->GetOnePercentLowFPS();

	UE_LOG(LogTemp, Display, TEXT("Realistic Load Test:"));
	UE_LOG(LogTemp, Display, TEXT("  Total frames: %d"), TotalFrames);
	UE_LOG(LogTemp, Display, TEXT("  Current FPS: %.2f"), CurrentFPS);
	UE_LOG(LogTemp, Display, TEXT("  Average FPS: %.2f"), AverageFPS);
	UE_LOG(LogTemp, Display, TEXT("  1%% Low FPS: %.2f"), OnePercentLow);
	UE_LOG(LogTemp, Display, TEXT("  Budget violations: %d"), BudgetViolationCount);

	// System breakdown
	TArray<FSystemPerformanceData> AllSystems = TelemetrySubsystem->GetAllSystemPerformance();
	for (const FSystemPerformanceData& SystemData : AllSystems)
	{
		UE_LOG(LogTemp, Display, TEXT("  %s: %.2f ms avg (%.2f%% budget)"),
			*SystemData.SystemName.ToString(),
			SystemData.AverageCycleTimeMs,
			TelemetrySubsystem->GetSystemBudgetUtilization(SystemData.SystemName) * 100.0f);
	}

	// Verify performance is acceptable
	TestTrue(TEXT("Average FPS should be > 30"), AverageFPS > 30.0f);
	TestTrue(TEXT("1% low FPS should be > 20"), OnePercentLow > 20.0f);

	// Budget violations should be relatively rare (< 10% of frames)
	float ViolationRate = (static_cast<float>(BudgetViolationCount) / TotalFrames) * 100.0f;
	UE_LOG(LogTemp, Display, TEXT("  Budget violation rate: %.2f%%"), ViolationRate);
	
	TestTrue(FString::Printf(TEXT("Budget violation rate < 10%% (actual: %.2f%%)"), ViolationRate),
		ViolationRate < 10.0f);

	return true;
}

/**
 * Integration test: Correlation between entity counts and frame time
 * Requirement: 10.2
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryEntityCorrelationTest,
	"DelveDeep.Telemetry.Integration.EntityCorrelation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryEntityCorrelationTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Register systems
	TelemetrySubsystem->RegisterSystemBudget(FName("CombatSystem"), 2.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("AISystem"), 3.0f);

	// Test 1: Low entity count - should have good performance
	UE_LOG(LogTemp, Display, TEXT("Testing low entity count..."));
	
	for (int32 i = 0; i < 60; ++i)
	{
		TelemetrySubsystem->TrackEntityCount(FName("Monsters"), 5);
		TelemetrySubsystem->TrackEntityCount(FName("Projectiles"), 3);
		
		// Simulate light system load
		TelemetrySubsystem->RecordSystemTime(FName("CombatSystem"), 0.8f);
		TelemetrySubsystem->RecordSystemTime(FName("AISystem"), 1.2f);
		
		TelemetrySubsystem->Tick(1.0f / 60.0f);
	}

	float LowEntityFPS = TelemetrySubsystem->GetAverageFPS();
	UE_LOG(LogTemp, Display, TEXT("  Low entity FPS: %.2f"), LowEntityFPS);

	// Test 2: High entity count - should have lower performance
	UE_LOG(LogTemp, Display, TEXT("Testing high entity count..."));
	
	for (int32 i = 0; i < 60; ++i)
	{
		TelemetrySubsystem->TrackEntityCount(FName("Monsters"), 100);
		TelemetrySubsystem->TrackEntityCount(FName("Projectiles"), 200);
		
		// Simulate heavy system load
		TelemetrySubsystem->RecordSystemTime(FName("CombatSystem"), 2.5f);
		TelemetrySubsystem->RecordSystemTime(FName("AISystem"), 4.0f);
		
		TelemetrySubsystem->Tick(1.0f / 60.0f);
	}

	float HighEntityFPS = TelemetrySubsystem->GetAverageFPS();
	UE_LOG(LogTemp, Display, TEXT("  High entity FPS: %.2f"), HighEntityFPS);

	// Verify correlation: high entity count should result in lower FPS
	TestTrue(TEXT("High entity count should result in lower FPS"), HighEntityFPS < LowEntityFPS);

	float FPSDifference = LowEntityFPS - HighEntityFPS;
	UE_LOG(LogTemp, Display, TEXT("  FPS difference: %.2f"), FPSDifference);

	// FPS should be noticeably different (at least 10 FPS difference)
	TestTrue(FString::Printf(TEXT("FPS difference should be significant (actual: %.2f)"), FPSDifference),
		FPSDifference > 10.0f);

	// Get entity count statistics
	int32 PeakMonsters = TelemetrySubsystem->GetPeakEntityCount(FName("Monsters"));
	int32 PeakProjectiles = TelemetrySubsystem->GetPeakEntityCount(FName("Projectiles"));

	UE_LOG(LogTemp, Display, TEXT("  Peak monsters: %d"), PeakMonsters);
	UE_LOG(LogTemp, Display, TEXT("  Peak projectiles: %d"), PeakProjectiles);

	TestEqual(TEXT("Peak monsters should be 100"), PeakMonsters, 100);
	TestEqual(TEXT("Peak projectiles should be 200"), PeakProjectiles, 200);

	return true;
}

/**
 * Integration test: Baseline comparison across different gameplay scenarios
 * Requirement: 2.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepTelemetryBaselineComparisonTest,
	"DelveDeep.Telemetry.Integration.BaselineComparison",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepTelemetryBaselineComparisonTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = 
		GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>();
	
	TestNotNull(TEXT("TelemetrySubsystem should be available"), TelemetrySubsystem);

	// Register systems
	TelemetrySubsystem->RegisterSystemBudget(FName("CombatSystem"), 2.0f);
	TelemetrySubsystem->RegisterSystemBudget(FName("AISystem"), 3.0f);

	// Scenario 1: Idle gameplay (baseline)
	UE_LOG(LogTemp, Display, TEXT("Capturing idle baseline..."));
	
	for (int32 i = 0; i < 120; ++i)
	{
		TelemetrySubsystem->RecordSystemTime(FName("CombatSystem"), 0.5f);
		TelemetrySubsystem->RecordSystemTime(FName("AISystem"), 0.8f);
		TelemetrySubsystem->TrackEntityCount(FName("Monsters"), 5);
		TelemetrySubsystem->Tick(1.0f / 60.0f);
	}

	bool bCaptured = TelemetrySubsystem->CaptureBaseline(FName("IdleBaseline"));
	TestTrue(TEXT("Idle baseline should be captured"), bCaptured);

	// Scenario 2: Light combat
	UE_LOG(LogTemp, Display, TEXT("Testing light combat..."));
	
	for (int32 i = 0; i < 120; ++i)
	{
		TelemetrySubsystem->RecordSystemTime(FName("CombatSystem"), 1.2f);
		TelemetrySubsystem->RecordSystemTime(FName("AISystem"), 1.8f);
		TelemetrySubsystem->TrackEntityCount(FName("Monsters"), 20);
		TelemetrySubsystem->Tick(1.0f / 60.0f);
	}

	FPerformanceComparison LightCombatComparison;
	bool bCompared = TelemetrySubsystem->CompareToBaseline(FName("IdleBaseline"), LightCombatComparison);
	TestTrue(TEXT("Light combat comparison should succeed"), bCompared);

	UE_LOG(LogTemp, Display, TEXT("Light Combat vs Idle:"));
	UE_LOG(LogTemp, Display, TEXT("  FPS change: %.2f%%"), LightCombatComparison.FPSChangePercent);
	UE_LOG(LogTemp, Display, TEXT("  Frame time change: %.2f%%"), LightCombatComparison.FrameTimeChangePercent);

	// Light combat should be slower than idle
	TestTrue(TEXT("Light combat should have lower FPS than idle"), 
		LightCombatComparison.FPSChangePercent < 0.0f);

	// Scenario 3: Heavy combat
	UE_LOG(LogTemp, Display, TEXT("Testing heavy combat..."));
	
	for (int32 i = 0; i < 120; ++i)
	{
		TelemetrySubsystem->RecordSystemTime(FName("CombatSystem"), 2.5f);
		TelemetrySubsystem->RecordSystemTime(FName("AISystem"), 3.5f);
		TelemetrySubsystem->TrackEntityCount(FName("Monsters"), 80);
		TelemetrySubsystem->Tick(1.0f / 60.0f);
	}

	FPerformanceComparison HeavyCombatComparison;
	bCompared = TelemetrySubsystem->CompareToBaseline(FName("IdleBaseline"), HeavyCombatComparison);
	TestTrue(TEXT("Heavy combat comparison should succeed"), bCompared);

	UE_LOG(LogTemp, Display, TEXT("Heavy Combat vs Idle:"));
	UE_LOG(LogTemp, Display, TEXT("  FPS change: %.2f%%"), HeavyCombatComparison.FPSChangePercent);
	UE_LOG(LogTemp, Display, TEXT("  Frame time change: %.2f%%"), HeavyCombatComparison.FrameTimeChangePercent);

	// Heavy combat should be significantly slower than idle
	TestTrue(TEXT("Heavy combat should have much lower FPS than idle"), 
		HeavyCombatComparison.FPSChangePercent < LightCombatComparison.FPSChangePercent);

	// Heavy combat should have larger performance impact than light combat
	float HeavyImpact = FMath::Abs(HeavyCombatComparison.FPSChangePercent);
	float LightImpact = FMath::Abs(LightCombatComparison.FPSChangePercent);
	
	TestTrue(FString::Printf(TEXT("Heavy combat impact (%.2f%%) > light combat impact (%.2f%%)"), 
		HeavyImpact, LightImpact),
		HeavyImpact > LightImpact);

	return true;
}
