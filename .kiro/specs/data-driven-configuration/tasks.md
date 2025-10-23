# Implementation Plan

- [x] 1. Set up core validation infrastructure
  - Create FValidationContext struct with error/warning tracking methods
  - Implement GetReport() function for formatted validation output
  - Add logging category DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepConfig, Log, All)
  - _Requirements: 5.2, 5.3_

- [x] 2. Implement base data asset classes
- [x] 2.1 Create UDelveDeepCharacterData class
  - Write header with all UPROPERTY fields (health, damage, speed, armor, resource stats)
  - Implement PostLoad() override for load-time validation
  - Implement Validate() method using FValidationContext
  - Add TSoftObjectPtr references for starting weapon and abilities
  - _Requirements: 1.2, 1.4, 9.2_

- [x] 2.2 Create UDelveDeepWeaponData class
  - Write header with combat stats (damage, attack speed, range, damage type)
  - Add projectile parameters (speed, piercing, max pierce targets)
  - Implement PostLoad() and Validate() methods
  - Add TSoftObjectPtr reference for special ability
  - _Requirements: 4.1, 4.4, 9.2_

- [x] 2.3 Create UDelveDeepAbilityData class
  - Write header with timing parameters (cooldown, cast time, duration)
  - Add resource cost and damage parameters
  - Add AoE parameters (radius, affects allies flag)
  - Implement PostLoad() and Validate() methods
  - _Requirements: 4.2, 4.3, 9.2_

- [x] 2.4 Create UDelveDeepUpgradeData class
  - Write header with cost parameters (base cost, scaling factor, max level)
  - Add stat modification properties (health, damage, speed, armor modifiers)
  - Implement CalculateCostForLevel() Blueprint-callable function
  - Add TArray of TSoftObjectPtr for required upgrades (dependencies)
  - Implement PostLoad() and Validate() methods
  - _Requirements: 3.1, 3.2, 3.3, 3.5_

- [x] 3. Implement data table structures
- [x] 3.1 Create FDelveDeepMonsterConfig struct
  - Write struct inheriting from FTableRowBase with all monster properties
  - Add display info (name, description), stats (health, damage, speed, armor)
  - Add AI parameters (detection range, attack range, behavior type)
  - Add reward parameters (coin drops, experience)
  - Implement OnPostDataImport() for import-time validation
  - Implement Validate() method using FValidationContext
  - _Requirements: 2.1, 2.5, 9.2_

- [x] 4. Implement configuration manager subsystem
- [x] 4.1 Create UDelveDeepConfigurationManager class skeleton
  - Write class inheriting from UGameInstanceSubsystem
  - Add TMap cache members for each data asset type
  - Add UDataTable pointers for monster and item configs
  - Add performance tracking members (cache hits, misses, query time, query count)
  - Declare all public Blueprint-callable query functions
  - _Requirements: 6.1, 6.3, 6.4_

- [x] 4.2 Implement subsystem initialization
  - Override Initialize() to load all data assets and tables
  - Implement LoadCharacterData() to scan and cache character assets
  - Implement LoadUpgradeData() to scan and cache upgrade assets
  - Implement LoadWeaponData() to scan and cache weapon assets
  - Implement LoadAbilityData() to scan and cache ability assets
  - Implement LoadDataTables() to load monster and item tables
  - Add timing measurement for initialization (log total time)
  - _Requirements: 1.1, 2.2, 6.2, 10.2_

- [x] 4.3 Implement data query functions
  - Implement GetCharacterData() with cache lookup and performance tracking
  - Implement GetMonsterConfig() with data table row lookup
  - Implement GetUpgradeData() with cache lookup
  - Implement GetWeaponData() with cache lookup
  - Implement GetAbilityData() with cache lookup
  - Add const reference returns for all query functions
  - Track query time and update performance metrics
  - _Requirements: 1.5, 2.4, 6.5, 9.3, 10.1_

- [x] 4.4 Implement validation system
  - Implement ValidateCharacterData() with range checks and reference validation
  - Implement ValidateMonsterConfig() with positive health and non-negative damage checks
  - Implement ValidateUpgradeData() with cost scaling factor range check (1.0-10.0)
  - Implement ValidateWeaponData() with cooldown and resource cost validation
  - Implement ValidateAbilityData() with timing parameter validation
  - Implement ValidateAllData() Blueprint-callable function that validates all cached data
  - Generate comprehensive validation report with all errors and warnings
  - _Requirements: 3.3, 5.1, 5.2, 5.3, 5.5_

- [x] 4.5 Implement performance metrics
  - Implement GetPerformanceStats() to return cache hits, misses, and average query time
  - Add cache hit/miss tracking to all query functions
  - Calculate average query time from total time and query count
  - Log performance warning when queries exceed 1000 per frame
  - _Requirements: 10.1, 10.3, 10.4_

- [x] 5. Implement hot-reload support for development builds
  - Add #if !UE_BUILD_SHIPPING preprocessor guards
  - Implement SetupHotReload() to register asset registry callbacks
  - Implement OnAssetReloaded() to detect modified data assets
  - Reload modified assets and update cache within 2 seconds
  - Re-validate reloaded assets and log results
  - Broadcast notification event when hot-reload completes
  - Store FDelegateHandle for cleanup in Deinitialize()
  - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5_

- [x] 6. Implement console commands for testing and debugging
  - Create console command "DelveDeep.ValidateAllData" that calls ValidateAllData() and logs report
  - Create console command "DelveDeep.ShowConfigStats" that calls GetPerformanceStats() and logs results
  - Create console command "DelveDeep.ListLoadedAssets" that logs all cached asset names
  - Create console command "DelveDeep.ReloadConfigData" that forces full reload of all data
  - Create console command "DelveDeep.DumpConfigData" that logs all properties of specified asset
  - _Requirements: 5.3, 10.3_

- [x] 7. Add module dependencies and build configuration
  - Update DelveDeep.Build.cs to include GameplayTags in PublicDependencyModuleNames
  - Add AssetRegistry to PrivateDependencyModuleNames for hot-reload support
  - Verify all required modules are present (Core, CoreUObject, Engine)
  - _Requirements: 6.1_

- [x] 8. Implement SCOPE_CYCLE_COUNTER profiling
  - Add DECLARE_STATS_GROUP for DelveDeep configuration system
  - Add DECLARE_CYCLE_STAT for data loading operations
  - Add DECLARE_CYCLE_STAT for data query operations
  - Add SCOPE_CYCLE_COUNTER to Initialize() function
  - Add SCOPE_CYCLE_COUNTER to all query functions
  - _Requirements: 10.5_

- [x] 9. Create documentation for content directory structure
  - Document standard directory structure (Content/Data/Characters/, Monsters/, etc.)
  - Document naming conventions (DA_ prefix for data assets, DT_ prefix for data tables)
  - Document asset organization best practices
  - Add examples of proper asset naming
  - _Requirements: 8.1, 8.2, 8.3, 8.5_

- [x] 10. Implement directory structure validation
  - Add function to check if loaded assets follow standard directory structure
  - Log warning when assets are found outside standard directories
  - Provide suggestions for correct asset placement in warnings
  - _Requirements: 8.4_

- [x] 11. Create example data assets for testing
  - Create example character data asset (DA_Character_Warrior) with valid test values
  - Create example monster config data table (DT_Monster_Configs) with 3-5 test monsters
  - Create example upgrade data asset (DA_Upgrade_HealthBoost) with valid test values
  - Create example weapon data asset (DA_Weapon_Sword) with valid test values
  - Create example ability data asset (DA_Ability_Cleave) with valid test values
  - _Requirements: 1.1, 2.1, 3.1, 4.1_

- [ ] 12. Write unit tests for validation system
  - Write test for FValidationContext error/warning tracking
  - Write test for character data validation with invalid health value
  - Write test for monster config validation with negative damage
  - Write test for upgrade data validation with out-of-range scaling factor
  - Write test for weapon data validation with invalid cooldown
  - Write test for ability data validation with invalid resource cost
  - _Requirements: 5.1, 5.2_

- [ ] 13. Write unit tests for configuration manager
  - Write test for asset caching on first query
  - Write test for cached asset returned on subsequent queries
  - Write test for cache hit rate tracking accuracy
  - Write test for data table lookup by name
  - Write test for invalid name returns nullptr
  - Write test for query performance under target thresholds
  - _Requirements: 6.4, 10.1_

- [ ] 14. Write integration tests
  - Write test for upgrade cost calculation at various levels
  - Write test for asset reference resolution (weapon → ability)
  - Write test for upgrade dependency chain validation
  - Write test for hot-reload functionality in development builds
  - _Requirements: 3.5, 7.2, 7.3_

- [ ] 15. Performance testing and optimization
  - Profile initialization time with 100+ assets (target: < 100ms)
  - Profile single query time (target: < 1ms)
  - Profile bulk query performance (1000 queries)
  - Measure cache hit rate (target: > 95%)
  - Optimize any bottlenecks found during profiling
  - _Requirements: 1.5, 2.4, 5.5, 10.1_
