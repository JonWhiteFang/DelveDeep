# Configuration System Performance Testing

## Overview

The DelveDeep configuration system includes comprehensive performance testing and profiling capabilities to ensure the system meets its performance targets:

- **Initialization**: < 100ms for loading 100+ assets
- **Single Query**: < 1ms per query
- **Bulk Queries**: < 1ms average for 1000 queries
- **Cache Hit Rate**: > 95% for repeated queries

## Automated Tests

### Running Unit Tests

Execute all performance tests via Unreal's automation system:

```bash
UnrealEditor-Cmd.exe DelveDeep -ExecCmds="Automation RunTests DelveDeep.Performance" -unattended
```

### Available Performance Tests

1. **FDelveDeepConfigInitializationPerformanceTest**
   - Measures subsystem initialization time
   - Target: < 100ms
   - Validates asset loading performance

2. **FDelveDeepConfigSingleQueryPerformanceTest**
   - Measures single data query time
   - Target: < 1ms
   - Tests cache lookup performance

3. **FDelveDeepConfigBulkQueryPerformanceTest**
   - Measures performance of 1000 queries
   - Target: < 1ms average
   - Tests sustained query performance

4. **FDelveDeepConfigCacheHitRateTest**
   - Measures cache efficiency for repeated queries
   - Target: > 95% hit rate
   - Validates caching implementation

5. **FDelveDeepConfigMemoryEfficiencyTest**
   - Verifies cache doesn't cause excessive memory usage
   - Tests performance under sustained load

6. **FDelveDeepConfigConcurrentQueryTest**
   - Simulates multiple systems querying simultaneously
   - Tests performance under concurrent load

7. **FDelveDeepConfigValidationPerformanceTest**
   - Measures validation system performance
   - Target: < 100ms for all data
   - Ensures validation doesn't impact runtime

## Console Commands

### Quick Performance Profile

Run a comprehensive performance profile with a single command:

```
DelveDeep.ProfileConfigPerformance
```

This command runs all performance tests and generates a detailed report including:
- Single query performance
- Bulk query performance (1000 queries)
- Cache hit rate analysis
- Validation performance
- Pass/fail status for each metric

### Performance Statistics

View current performance statistics:

```
DelveDeep.ShowConfigStats
```

Output includes:
- Total queries executed
- Cache hits and misses
- Cache hit rate percentage
- Average query time

### Asset Listing

List all loaded configuration assets:

```
DelveDeep.ListLoadedAssets
```

Shows cached assets by type:
- Character Data
- Upgrade Data
- Weapon Data
- Ability Data
- Monster Configs

## Performance Metrics

### Current Performance Characteristics

The configuration system is optimized for:

1. **Fast Initialization**
   - Assets loaded on subsystem initialization
   - Parallel asset loading where possible
   - Validation performed during initialization

2. **Sub-Millisecond Queries**
   - TMap-based caching for O(1) lookups
   - Const-correct query functions
   - Minimal overhead per query

3. **High Cache Hit Rate**
   - Assets cached on first access
   - Cache persists across level transitions
   - No cache invalidation during normal operation

4. **Memory Efficiency**
   - TSoftObjectPtr for asset references
   - UPROPERTY() prevents garbage collection
   - Minimal memory overhead per cached asset

### Performance Tracking

The system tracks:
- **Cache Hits**: Successful cache lookups
- **Cache Misses**: Assets not found in cache
- **Total Query Time**: Cumulative time spent in queries
- **Query Count**: Total number of queries executed
- **Average Query Time**: Calculated from total time / count

### Profiling with Unreal Insights

Use SCOPE_CYCLE_COUNTER stats for detailed profiling:

```
stat DelveDeepConfig
```

Available stats:
- `STAT_ConfigInitialize` - Initialization time
- `STAT_LoadCharacterData` - Character data loading
- `STAT_LoadUpgradeData` - Upgrade data loading
- `STAT_LoadWeaponData` - Weapon data loading
- `STAT_LoadAbilityData` - Ability data loading
- `STAT_LoadDataTables` - Data table loading
- `STAT_GetCharacterData` - Character data queries
- `STAT_GetMonsterConfig` - Monster config queries
- `STAT_GetUpgradeData` - Upgrade data queries
- `STAT_GetWeaponData` - Weapon data queries
- `STAT_GetAbilityData` - Ability data queries
- `STAT_ValidateAllData` - Validation time

## Optimization Guidelines

### When to Optimize

Consider optimization if:
- Initialization time exceeds 100ms
- Average query time exceeds 1ms
- Cache hit rate falls below 95%
- Memory usage grows unexpectedly

### Optimization Strategies

1. **Reduce Asset Count**
   - Combine similar assets
   - Use data tables for bulk data
   - Lazy-load optional assets

2. **Improve Cache Efficiency**
   - Pre-warm cache for common assets
   - Increase cache size if needed
   - Profile cache access patterns

3. **Optimize Validation**
   - Move validation to editor-only builds
   - Cache validation results
   - Reduce validation frequency

4. **Parallel Loading**
   - Load asset types in parallel
   - Use async loading for large assets
   - Defer non-critical asset loading

## Continuous Integration

### CI/CD Performance Tests

Include performance tests in CI pipeline:

```bash
# Run performance tests
UnrealEditor-Cmd.exe DelveDeep -ExecCmds="Automation RunTests DelveDeep.Performance" -unattended -log

# Check for performance regressions
# Parse log for FAIL messages
# Fail build if performance targets not met
```

### Performance Regression Detection

Monitor these metrics over time:
- Initialization time trend
- Average query time trend
- Cache hit rate trend
- Memory usage trend

Alert if:
- Initialization time increases by > 20%
- Query time increases by > 50%
- Cache hit rate decreases by > 5%
- Memory usage increases by > 30%

## Troubleshooting

### Slow Initialization

If initialization exceeds 100ms:
1. Check asset count with `DelveDeep.ListLoadedAssets`
2. Profile with `stat DelveDeepConfig`
3. Look for slow asset loading operations
4. Consider lazy loading non-critical assets

### Slow Queries

If queries exceed 1ms:
1. Check cache hit rate with `DelveDeep.ShowConfigStats`
2. Verify assets are being cached correctly
3. Profile with `stat DelveDeepConfig`
4. Look for expensive validation or lookup operations

### Low Cache Hit Rate

If cache hit rate is below 95%:
1. Verify asset names are consistent
2. Check for typos in asset references
3. Ensure assets exist in expected directories
4. Review query patterns for inefficiencies

### High Memory Usage

If memory usage is excessive:
1. Check number of cached assets
2. Verify TSoftObjectPtr usage for references
3. Look for memory leaks in validation
4. Profile with memory profiler

## Best Practices

1. **Run performance tests regularly** during development
2. **Profile before and after changes** to detect regressions
3. **Monitor cache hit rate** to ensure efficient caching
4. **Use console commands** for quick performance checks
5. **Include performance tests in CI/CD** pipeline
6. **Document performance characteristics** of new features
7. **Set performance budgets** for new systems
8. **Optimize based on profiling data**, not assumptions

## Future Improvements

Potential optimizations for future consideration:

1. **Async Asset Loading**
   - Load assets asynchronously during initialization
   - Reduce blocking time on startup

2. **Asset Streaming**
   - Stream assets on-demand
   - Reduce initial memory footprint

3. **Cache Warming**
   - Pre-load commonly used assets
   - Improve first-query performance

4. **Query Batching**
   - Batch multiple queries together
   - Reduce per-query overhead

5. **Validation Caching**
   - Cache validation results
   - Skip re-validation of unchanged assets

6. **Memory Pooling**
   - Pool validation context objects
   - Reduce allocation overhead
