# Implementation Plan

- [x] 1. Set up core telemetry subsystem infrastructure
  - Create UDelveDeepTelemetrySubsystem class inheriting from UGameInstanceSubsystem
  - Implement Initialize() and Deinitialize() lifecycle methods with logging
  - Override Tick() method for per-frame telemetry updates
  - Add DELVEDEEP_API macro and proper UCLASS specifiers (BlueprintType)
  - Create dedicated log category LogDelveDeepTelemetry
  - _Requirements: 1.1, 2.1_

- [x] 2. Implement stat groups and cycle counters
  - Create STATGROUP_DelveDeep main stat group
  - Create system-specific stat groups (Combat, AI, World, UI, Events, Config)
  - Declare cycle counters for each major system (STAT_CombatSystem, STAT_AISystem, etc.)
  - Declare memory counters for total and per-system memory tracking
  - Declare dword counters for entity counts (monsters, projectiles, particles)
  - Add stat group registration in subsystem Initialize()
  - _Requirements: 2.1, 2.2, 2.3, 9.1, 9.2, 9.5_

- [x] 3. Implement frame performance tracking
  - Create FFramePerformanceTracker class with TCircularBuffer for frame history
  - Implement RecordFrame() method to capture frame time each tick
  - Calculate current FPS, average FPS over 60 frames
  - Implement 1% low and 0.1% low FPS calculations using percentiles
  - Add spike detection for frames exceeding 16.67ms threshold
  - Track consecutive slow frames for performance alert triggering
  - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5_

- [x] 4. Implement system profiler with budget tracking
  - Create FSystemProfiler class with TMap for system performance data
  - Implement RegisterSystem() method to register systems with budgets
  - Create RecordSystemTime() method to track cycle times per system
  - Calculate budget utilization percentage (actual time / budget time)
  - Implement budget violation detection and logging
  - Maintain violation history with TArray (last 100 violations)
  - _Requirements: 2.1, 2.3, 2.4, 2.5, 4.1, 4.2, 4.3, 4.4, 4.5_

- [x] 5. Implement memory tracking system
  - Create FMemoryTracker class with FMemorySnapshot structure
  - Implement UpdateMemorySnapshot() to capture total and per-system memory
  - Track native memory, managed memory, and per-system allocations
  - Implement memory leak detection by monitoring growth rate
  - Calculate memory growth rate (MB per minute)
  - Update memory metrics every 100 frames to minimize overhead
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5_

- [x] 6. Implement performance budget configuration
  - Create FPerformanceBudget struct with SystemName, BudgetMilliseconds, bEnforceStrict
  - Load budget configuration from data assets on subsystem initialization
  - Implement RegisterSystemBudget() method for runtime budget registration
  - Create GetSystemBudgetUtilization() method to query budget usage
  - Add validation for budget values (must be positive, reasonable range)
  - _Requirements: 4.1, 4.2, 4.4_

- [x] 7. Implement performance baseline system
  - Create FPerformanceBaseline struct with frame data, system data, memory data
  - Implement CaptureBaseline() method to record current performance metrics
  - Create CompareToBaseline() method to calculate percentage differences
  - Implement SaveBaseline() and LoadBaseline() for persistence
  - Add baseline validation to ensure compatibility with current build
  - Store baselines in TMap with FName keys for quick lookup
  - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_

- [x] 8. Implement performance reporting system
  - Create FPerformanceReport struct with comprehensive statistics
  - Implement GeneratePerformanceReport() to aggregate data over time period
  - Calculate frame time percentiles (50th, 95th, 99th)
  - Include system breakdown with percentage of frame time
  - Add memory trends and spike analysis to reports
  - Include metadata (build version, map name, timestamp)
  - _Requirements: 6.1, 6.2, 6.4, 6.5_

- [x] 9. Implement report export functionality
  - Create ExportReportToCSV() method with proper column headers and escaping
  - Implement ExportReportToJSON() with hierarchical data structure
  - Add gzip compression for large exports to reduce file size
  - Validate file paths before export and handle permission errors
  - Log export completion with file path and size
  - _Requirements: 6.3, 12.1, 12.2, 12.3, 12.4, 12.5_

- [x] 10. Implement real-time performance overlay
  - Create FPerformanceOverlay class with EOverlayMode enum (Minimal, Standard, Detailed)
  - Implement Render() method to draw overlay on UCanvas
  - Create RenderMinimal() for FPS-only display
  - Implement RenderStandard() with FPS and frame time graph (last 120 frames)
  - Create RenderDetailed() with FPS, graph, system breakdown, and memory
  - Add color-coded performance zones (green: <16ms, yellow: 16-20ms, red: >20ms)
  - Ensure overlay rendering overhead is <0.1ms per frame
  - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5_

- [x] 11. Implement profiling session system
  - Create FProfilingSession struct with session name, start/end time, frame data
  - Implement StartProfilingSession() to begin detailed metric collection
  - Increase sampling rate during profiling for per-frame data capture
  - Implement StopProfilingSession() to end session and generate report
  - Limit profiling duration to 60 seconds to prevent excessive memory usage
  - Auto-save profiling data with timestamp-based filename
  - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5_

- [x] 12. Implement gameplay metrics tracking
  - Create TMap for entity counts (monsters, projectiles, particles)
  - Implement TrackEntityCount() method to update entity counts
  - Create GetEntityCount() method to query current counts
  - Track peak entity counts over time
  - Log warnings when entity counts exceed recommended limits
  - Update entity counts every 10 frames to minimize overhead
  - _Requirements: 10.1, 10.2, 10.3, 10.4, 10.5_

- [ ] 13. Implement asset loading performance tracking
  - Create FAssetLoadRecord struct with asset path, load time, size, type
  - Implement RecordAssetLoad() method to capture load operations
  - Categorize assets by type (texture, mesh, sound, data asset)
  - Track synchronous and asynchronous loads separately
  - Log warnings for asset loads exceeding 100ms
  - Provide GetAssetLoadStatistics() for querying load performance
  - _Requirements: 11.1, 11.2, 11.3, 11.4, 11.5_

- [ ] 14. Implement console commands for telemetry
  - Create DelveDeep.Telemetry.ShowFPS command to display FPS statistics
  - Create DelveDeep.Telemetry.ShowSystemStats command for system breakdown
  - Create DelveDeep.Telemetry.ShowBudgets command to display budget utilization
  - Create DelveDeep.Telemetry.ShowMemory command for memory statistics
  - Create DelveDeep.Telemetry.CaptureBaseline command with baseline name parameter
  - Create DelveDeep.Telemetry.CompareBaseline command for baseline comparison
  - Create DelveDeep.Telemetry.GenerateReport command with optional duration
  - Create DelveDeep.Telemetry.StartProfiling and StopProfiling commands
  - Create DelveDeep.Telemetry.EnableOverlay and DisableOverlay commands
  - _Requirements: 1.1, 2.1, 3.1, 4.4, 5.3, 6.1, 7.1, 8.1_

- [ ] 15. Implement Unreal Insights integration
  - Add TRACE_CPUPROFILER_EVENT_SCOPE macros for major systems
  - Create named scopes for DelveDeep_CombatSystem, DelveDeep_AISystem, etc.
  - Ensure compatibility with Unreal Insights profiler
  - Add CSV profiler integration with CSV_SCOPED_TIMING_STAT
  - Create custom CSV stats for entity counts and gameplay metrics
  - _Requirements: 9.3, 9.4_

- [ ] 16. Implement Blueprint integration
  - Create UDelveDeepTelemetryBlueprintLibrary function library class
  - Implement GetTelemetrySubsystem() Blueprint-callable function with WorldContext
  - Create GetCurrentFPS() Blueprint helper function
  - Create CapturePerformanceBaseline() Blueprint helper function
  - Create EnablePerformanceOverlay() Blueprint helper function
  - Add BlueprintType specifiers to relevant structs (FPerformanceReport, FPerformanceBudget)
  - _Requirements: 1.5, 5.1, 7.1_

- [ ] 17. Implement conditional compilation for build configurations
  - Add preprocessor guards for development vs shipping builds
  - Enable full telemetry in development builds (!UE_BUILD_SHIPPING)
  - Reduce telemetry overhead in shipping builds (minimal tracking)
  - Add compile-time toggles for expensive features (profiling sessions, detailed overlay)
  - Document telemetry behavior differences between build types
  - _Requirements: 2.1, 7.1, 8.1_

- [ ] 18. Implement error handling and validation
  - Validate baseline names are not empty in CaptureBaseline()
  - Check file paths before export operations
  - Validate budget values are positive and within reasonable range
  - Handle rendering exceptions in overlay rendering
  - Implement graceful degradation if memory tracking fails
  - Use FValidationContext for consistent error reporting
  - _Requirements: 5.1, 5.5, 6.3, 7.1_

- [ ] 19. Create integration examples and documentation
  - Write example code for adding cycle counters to new systems
  - Document how to register system budgets
  - Create guide for capturing and comparing baselines
  - Write troubleshooting guide for common telemetry issues
  - Document console command usage with examples
  - Update DOCUMENTATION_INDEX.md with telemetry system documentation
  - _Requirements: 2.1, 4.1, 5.1, 6.1_

- [ ] 20. Write unit tests for core functionality
  - Test FPS calculation accuracy with known frame times
  - Test spike detection with various frame time patterns
  - Test budget violation detection and logging
  - Test memory leak detection algorithm
  - Test baseline capture and comparison
  - Test percentile calculations for frame times
  - Test report generation with sample data
  - _Requirements: 1.1, 1.2, 1.4, 2.5, 3.3, 4.3, 5.3, 6.4_

- [ ] 21. Write performance tests
  - Measure telemetry overhead per frame (target: <0.5ms)
  - Measure overlay rendering overhead (target: <0.1ms)
  - Test memory snapshot capture time (target: <1ms)
  - Test report generation time for 5-minute data (target: <100ms)
  - Verify telemetry memory footprint stays within budget
  - Profile profiling session overhead
  - _Requirements: 1.3, 7.2, 8.1_

- [ ] 22. Write integration tests
  - Test telemetry tracking of event system performance
  - Test telemetry tracking of configuration system performance
  - Verify budget allocations sum correctly across all systems
  - Test performance under realistic gameplay load
  - Verify correlation between entity counts and frame time
  - Test baseline comparison across different gameplay scenarios
  - _Requirements: 2.1, 2.4, 10.2_
