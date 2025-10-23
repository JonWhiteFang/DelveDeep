# DelveDeep Testing Guide

**Version**: 1.0  
**Last Updated**: October 23, 2025  
**Target**: Phase 1 - Data-Driven Configuration System

## Overview

This guide provides step-by-step instructions for thoroughly testing the DelveDeep project on a machine with Unreal Engine 5.6 installed. The testing focuses on validating the completed Data-Driven Configuration System and ensuring all components function correctly.

## Prerequisites

### Required Software
- **Unreal Engine 5.6** (installed and configured)
- **Visual Studio 2022** (with C++ game development workload)
- **Git** (for cloning the repository)

### System Requirements
- Windows 10/11 or macOS
- 16GB+ RAM recommended
- SSD storage for faster compilation
- GPU with DirectX 12 or Metal support

## Setup Instructions

### 1. Clone and Prepare Project

```bash
# Clone the repository
git clone <repository-url> DelveDeep
cd DelveDeep

# Verify project structure
ls -la Source/DelveDeep/
ls -la Documentation/
```

### 2. Generate Project Files

**Windows:**
```bash
# Right-click DelveDeep.uproject
# Select "Generate Visual Studio project files"

# Or use command line:
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="<full-path>\DelveDeep.uproject" -game -engine
```

**macOS:**
```bash
# Generate Xcode project
/Users/Shared/Epic\ Games/UE_5.6/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh -project="<full-path>/DelveDeep.uproject" -game -engine
```

### 3. Compile the Project

**Visual Studio (Windows):**
1. Open `DelveDeep.sln`
2. Set build configuration to `Development Editor`
3. Set platform to `Win64`
4. Build Solution (Ctrl+Shift+B)
5. Wait for compilation to complete (5-15 minutes first time)

**Xcode (macOS):**
1. Open `DelveDeep.xcworkspace`
2. Set scheme to `DelveDeepEditor`
3. Set configuration to `Development`
4. Build (Cmd+B)
5. Wait for compilation to complete

**Expected Output:**
```
========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
```

### 4. Launch Unreal Editor

**Windows:**
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "<full-path>\DelveDeep.uproject"
```

**macOS:**
```bash
/Users/Shared/Epic\ Games/UE_5.6/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor "<full-path>/DelveDeep.uproject"
```

**First Launch:**
- Editor will compile shaders (5-10 minutes)
- Wait for "Compiling Shaders" to complete
- Editor should open to default level

## Phase 1: Configuration System Testing

### Test 1: Verify Module Loading

**Objective**: Confirm DelveDeep module loads correctly

**Steps:**
1. Open Unreal Editor
2. Open Output Log (Window > Developer Tools > Output Log)
3. Filter by "LogDelveDeepConfig"

**Expected Output:**
```
LogDelveDeepConfig: Display: Configuration Manager initializing...
LogDelveDeepConfig: Display: Configuration Manager initialized: X assets loaded
```

**Pass Criteria:**
- ✅ No errors in Output Log
- ✅ Configuration Manager initializes
- ✅ Asset count > 0

**Troubleshooting:**
- If module fails to load, verify compilation succeeded
- Check for missing dependencies in DelveDeep.Build.cs
- Ensure all source files are present

---

### Test 2: Create Data Assets

**Objective**: Verify data asset classes are available and functional

#### 2.1 Create Character Data Asset

**Steps:**
1. In Content Browser, navigate to `Content/Data/Characters/`
2. Right-click > Miscellaneous > Data Asset
3. Search for "DelveDeepCharacterData"
4. Select `UDelveDeepCharacterData`
5. Name it `DA_Character_TestWarrior`
6. Double-click to open

**Configure Properties:**
```
Character Name: "Test Warrior"
Description: "A test warrior character for validation"
Base Health: 150.0
Base Damage: 25.0
Base Move Speed: 350.0
```

**Expected Behavior:**
- ✅ Data asset class appears in picker
- ✅ Asset opens in editor
- ✅ All properties are editable
- ✅ Numeric properties respect ClampMin/ClampMax
- ✅ Save succeeds without errors

#### 2.2 Create Weapon Data Asset

**Steps:**
1. Navigate to `Content/Data/Weapons/`
2. Create new Data Asset of type `UDelveDeepWeaponData`
3. Name it `DA_Weapon_TestSword`

**Configure Properties:**
```
Weapon Name: "Test Sword"
Description: "A test sword for validation"
Base Damage: 15.0
Attack Speed: 1.5
Attack Range: 100.0
```

**Expected Behavior:**
- ✅ Asset creates successfully
- ✅ Properties editable and save correctly

#### 2.3 Create Ability Data Asset

**Steps:**
1. Navigate to `Content/Data/Abilities/`
2. Create new Data Asset of type `UDelveDeepAbilityData`
3. Name it `DA_Ability_TestCleave`

**Configure Properties:**
```
Ability Name: "Test Cleave"
Description: "A test cleave ability"
Damage Multiplier: 1.5
Cooldown: 5.0
Resource Cost: 20.0
```

**Expected Behavior:**
- ✅ Asset creates successfully
- ✅ All properties function correctly

#### 2.4 Create Upgrade Data Asset

**Steps:**
1. Navigate to `Content/Data/Upgrades/`
2. Create new Data Asset of type `UDelveDeepUpgradeData`
3. Name it `DA_Upgrade_TestHealthBoost`

**Configure Properties:**
```
Upgrade Name: "Test Health Boost"
Description: "Increases maximum health"
Base Cost: 100
Cost Scaling Factor: 1.5
Max Level: 10
Health Modifier: 25.0
```

**Expected Behavior:**
- ✅ Asset creates successfully
- ✅ Cost calculation works (test in console)

#### 2.5 Create Monster Data Table

**Steps:**
1. Navigate to `Content/Data/Monsters/`
2. Right-click > Miscellaneous > Data Table
3. Select `FDelveDeepMonsterConfig` as row structure
4. Name it `DT_Monster_TestConfigs`
5. Open and add rows:

**Row 1: "TestGoblin"**
```
Monster Name: "Test Goblin"
Health: 50.0
Damage: 8.0
Move Speed: 250.0
Attack Range: 75.0
```

**Row 2: "TestOrc"**
```
Monster Name: "Test Orc"
Health: 120.0
Damage: 15.0
Move Speed: 200.0
Attack Range: 100.0
```

**Expected Behavior:**
- ✅ Data table creates with correct structure
- ✅ Rows add successfully
- ✅ All fields editable

**Pass Criteria for Test 2:**
- ✅ All 5 asset types create successfully
- ✅ Properties are editable and save correctly
- ✅ No errors in Output Log during creation
- ✅ Assets appear in Content Browser

---

### Test 3: Validation System

**Objective**: Verify FValidationContext and data validation work correctly

#### 3.1 Test Valid Data

**Steps:**
1. Open `DA_Character_TestWarrior`
2. Set valid values (within ranges)
3. Save asset
4. Check Output Log

**Expected Output:**
```
LogDelveDeepConfig: Display: Character data loaded successfully
```

**Pass Criteria:**
- ✅ No validation errors
- ✅ Asset saves successfully

#### 3.2 Test Invalid Health Value

**Steps:**
1. Open `DA_Character_TestWarrior`
2. Set `Base Health` to `-10.0` (invalid)
3. Save asset
4. Check Output Log

**Expected Output:**
```
LogDelveDeepConfig: Error: Character data validation failed: BaseHealth out of range: -10.00 (expected 1-10000)
```

**Pass Criteria:**
- ✅ Validation error logged
- ✅ Error message includes context
- ✅ Asset still saves (validation is warning, not blocking)

#### 3.3 Test Invalid Damage Value

**Steps:**
1. Set `Base Damage` to `0.0` (invalid)
2. Save asset
3. Check Output Log

**Expected Output:**
```
LogDelveDeepConfig: Error: BaseDamage out of range: 0.00 (expected 1-1000)
```

**Pass Criteria:**
- ✅ Validation error logged
- ✅ Multiple errors can be reported

#### 3.4 Test Missing Asset Reference

**Steps:**
1. Leave `Starting Weapon` as None (null)
2. Save asset
3. Check Output Log

**Expected Output:**
```
LogDelveDeepConfig: Warning: No starting weapon assigned
```

**Pass Criteria:**
- ✅ Warning (not error) logged
- ✅ Distinguishes between errors and warnings

#### 3.5 Console Command Validation

**Steps:**
1. Open Command Console (` key or ~)
2. Type: `DelveDeep.ValidateAllData`
3. Press Enter

**Expected Output:**
```
LogDelveDeepConfig: Display: Validating all configuration data...
LogDelveDeepConfig: Display: Validation complete: X assets validated, Y errors, Z warnings
```

**Pass Criteria:**
- ✅ Command executes successfully
- ✅ Reports validation results
- ✅ Lists any errors/warnings found

---

### Test 4: Configuration Manager Subsystem

**Objective**: Verify subsystem initialization and data access

#### 4.1 Verify Subsystem Initialization

**Steps:**
1. Launch Editor
2. Open Output Log
3. Filter by "LogDelveDeepConfig"

**Expected Output:**
```
LogDelveDeepConfig: Display: Configuration Manager initializing...
LogDelveDeepConfig: Display: Loading character data...
LogDelveDeepConfig: Display: Loading monster data...
LogDelveDeepConfig: Display: Loading upgrade data...
LogDelveDeepConfig: Display: Configuration Manager initialized: X assets loaded
```

**Pass Criteria:**
- ✅ Subsystem initializes on startup
- ✅ All data types load
- ✅ Initialization completes in <100ms (check timing)

#### 4.2 Test Asset Caching

**Steps:**
1. Open Command Console
2. Type: `DelveDeep.ShowConfigStats`
3. Press Enter

**Expected Output:**
```
=== Configuration Manager Statistics ===
Total Assets Loaded: X
Character Data: Y
Monster Configs: Z
Upgrade Data: W
Cache Hits: A
Cache Misses: B
Cache Hit Rate: C%
Average Query Time: D ms
```

**Pass Criteria:**
- ✅ Statistics display correctly
- ✅ Cache hit rate > 0% after queries
- ✅ Average query time < 1ms

#### 4.3 Test Asset Listing

**Steps:**
1. Type: `DelveDeep.ListLoadedAssets`
2. Press Enter

**Expected Output:**
```
=== Loaded Configuration Assets ===
Character Data:
  - TestWarrior
  - (other characters)
Monster Configs:
  - TestGoblin
  - TestOrc
Upgrade Data:
  - TestHealthBoost
  - (other upgrades)
```

**Pass Criteria:**
- ✅ All created assets appear in list
- ✅ Assets organized by type
- ✅ Names match created assets

#### 4.4 Test Asset Dump

**Steps:**
1. Type: `DelveDeep.DumpConfigData TestWarrior`
2. Press Enter

**Expected Output:**
```
=== Character Data: TestWarrior ===
Character Name: Test Warrior
Description: A test warrior character for validation
Base Health: 150.00
Base Damage: 25.00
Base Move Speed: 350.00
Starting Weapon: DA_Weapon_TestSword
```

**Pass Criteria:**
- ✅ Asset properties display correctly
- ✅ Values match configured data
- ✅ Asset references resolve

---

### Test 5: Performance Testing

**Objective**: Verify performance targets are met

#### 5.1 Initialization Performance

**Steps:**
1. Close Editor
2. Delete `Saved/` and `Intermediate/` folders
3. Relaunch Editor
4. Check Output Log for timing

**Expected Output:**
```
LogDelveDeepConfig: Display: Configuration Manager initialized in 87.3 ms
```

**Pass Criteria:**
- ✅ Initialization < 100ms
- ✅ Timing logged correctly

#### 5.2 Query Performance

**Steps:**
1. Open Command Console
2. Type: `DelveDeep.ProfileConfigPerformance`
3. Press Enter

**Expected Output:**
```
=== Configuration Performance Profile ===
Running 1000 queries per asset type...

Character Data Queries:
  Total Time: 0.45 ms
  Average: 0.00045 ms per query
  
Monster Config Queries:
  Total Time: 0.38 ms
  Average: 0.00038 ms per query
  
Upgrade Data Queries:
  Total Time: 0.42 ms
  Average: 0.00042 ms per query

Overall Average: 0.00042 ms per query
Cache Hit Rate: 98.7%
```

**Pass Criteria:**
- ✅ Average query time < 1ms
- ✅ Cache hit rate > 95%
- ✅ Performance consistent across asset types

#### 5.3 Memory Usage

**Steps:**
1. Type: `stat memory`
2. Press Enter
3. Observe memory statistics

**Pass Criteria:**
- ✅ Memory usage reasonable for asset count
- ✅ No memory leaks (stable over time)

---

### Test 6: Hot Reload (Development Only)

**Objective**: Verify hot-reload functionality works

#### 6.1 Test Asset Modification

**Steps:**
1. Open `DA_Character_TestWarrior`
2. Change `Base Health` to `200.0`
3. Save asset
4. Check Output Log

**Expected Output:**
```
LogDelveDeepConfig: Display: Hot-reloading character data: TestWarrior
LogDelveDeepConfig: Display: Re-validating asset...
```

**Pass Criteria:**
- ✅ Hot-reload triggers within 2 seconds
- ✅ Asset re-validates automatically
- ✅ Changes reflected immediately

#### 6.2 Test Invalid Modification

**Steps:**
1. Change `Base Health` to `-50.0` (invalid)
2. Save asset
3. Check Output Log

**Expected Output:**
```
LogDelveDeepConfig: Display: Hot-reloading character data: TestWarrior
LogDelveDeepConfig: Error: Character data validation failed: BaseHealth out of range: -50.00 (expected 1-10000)
```

**Pass Criteria:**
- ✅ Hot-reload detects change
- ✅ Validation runs automatically
- ✅ Errors reported immediately

#### 6.3 Test Reload Command

**Steps:**
1. Fix invalid values
2. Type: `DelveDeep.ReloadConfigData`
3. Press Enter

**Expected Output:**
```
LogDelveDeepConfig: Display: Reloading all configuration data...
LogDelveDeepConfig: Display: Configuration data reloaded: X assets
```

**Pass Criteria:**
- ✅ Manual reload works
- ✅ All assets reload successfully
- ✅ Cache cleared and rebuilt

---

### Test 7: Example Data Creation

**Objective**: Verify example data generation works

**Steps:**
1. Open Command Console
2. Type: `DelveDeep.CreateExampleData`
3. Press Enter
4. Check Content Browser

**Expected Output:**
```
LogDelveDeepConfig: Display: Creating example data assets...
LogDelveDeepConfig: Display: Created character data: ExampleWarrior
LogDelveDeepConfig: Display: Created weapon data: ExampleSword
LogDelveDeepConfig: Display: Created ability data: ExampleCleave
LogDelveDeepConfig: Display: Created upgrade data: ExampleHealthBoost
LogDelveDeepConfig: Display: Example data creation complete
```

**Expected Assets Created:**
- `Content/Data/Characters/DA_Character_ExampleWarrior`
- `Content/Data/Weapons/DA_Weapon_ExampleSword`
- `Content/Data/Abilities/DA_Ability_ExampleCleave`
- `Content/Data/Upgrades/DA_Upgrade_ExampleHealthBoost`

**Pass Criteria:**
- ✅ Command executes successfully
- ✅ Assets created in correct directories
- ✅ Assets have valid default values
- ✅ Assets pass validation

---

### Test 8: Blueprint Integration

**Objective**: Verify C++ classes are Blueprint-accessible

#### 8.1 Create Blueprint Function Library Test

**Steps:**
1. Create new Blueprint (Blueprint Class)
2. Search for "DelveDeep" in parent class picker
3. Verify classes appear

**Expected Classes:**
- `DelveDeepCharacterData`
- `DelveDeepWeaponData`
- `DelveDeepAbilityData`
- `DelveDeepUpgradeData`
- `DelveDeepConfigurationManager`

**Pass Criteria:**
- ✅ All classes visible in Blueprint
- ✅ Classes marked as BlueprintType

#### 8.2 Test Blueprint Access to Configuration Manager

**Steps:**
1. Create new Blueprint (Actor or Function Library)
2. Add Event Graph node
3. Search for "Get Configuration Manager"
4. Verify function appears

**Pass Criteria:**
- ✅ Function accessible from Blueprint
- ✅ Returns valid subsystem reference

#### 8.3 Test Data Asset Properties in Blueprint

**Steps:**
1. Create Blueprint variable of type `DelveDeepCharacterData` (Object Reference)
2. Set default value to `DA_Character_TestWarrior`
3. Add nodes to access properties (Base Health, Base Damage, etc.)

**Pass Criteria:**
- ✅ Data asset usable as variable type
- ✅ Properties accessible via Blueprint nodes
- ✅ Values read correctly

---

## Test Results Template

Use this template to document test results:

```markdown
# DelveDeep Test Results

**Date**: [Date]
**Tester**: [Name]
**UE Version**: 5.6
**Platform**: [Windows/macOS]
**Build Configuration**: Development Editor

## Test Summary

| Test | Status | Notes |
|------|--------|-------|
| Module Loading | ✅/❌ | |
| Data Asset Creation | ✅/❌ | |
| Validation System | ✅/❌ | |
| Configuration Manager | ✅/❌ | |
| Performance Testing | ✅/❌ | |
| Hot Reload | ✅/❌ | |
| Example Data | ✅/❌ | |
| Blueprint Integration | ✅/❌ | |

## Performance Metrics

- Initialization Time: ___ ms (Target: <100ms)
- Average Query Time: ___ ms (Target: <1ms)
- Cache Hit Rate: ___% (Target: >95%)

## Issues Found

1. [Issue description]
   - Severity: Critical/High/Medium/Low
   - Steps to reproduce:
   - Expected behavior:
   - Actual behavior:

## Additional Notes

[Any additional observations or comments]
```

## Common Issues and Solutions

### Issue: Module Fails to Load

**Symptoms:**
- "Plugin 'DelveDeep' failed to load" error
- Missing module in Output Log

**Solutions:**
1. Verify compilation succeeded without errors
2. Check DelveDeep.Build.cs for correct dependencies
3. Regenerate project files
4. Clean and rebuild solution

### Issue: Data Asset Classes Not Appearing

**Symptoms:**
- Cannot find DelveDeepCharacterData in asset picker
- Classes missing from Blueprint picker

**Solutions:**
1. Verify DELVEDEEP_API macro present on classes
2. Check UCLASS specifiers include BlueprintType
3. Restart Editor after compilation
4. Refresh Content Browser

### Issue: Validation Not Running

**Symptoms:**
- No validation messages in Output Log
- Invalid data not detected

**Solutions:**
1. Verify PostLoad() implemented in data assets
2. Check LogDelveDeepConfig category enabled
3. Ensure validation context properly initialized
4. Test with console command: DelveDeep.ValidateAllData

### Issue: Console Commands Not Working

**Symptoms:**
- "Unknown command" error
- Commands not appearing in autocomplete

**Solutions:**
1. Verify FAutoConsoleCommand registered correctly
2. Check command names match exactly (case-sensitive)
3. Ensure module loaded successfully
4. Try restarting Editor

### Issue: Poor Performance

**Symptoms:**
- Initialization > 100ms
- Query time > 1ms
- Low cache hit rate

**Solutions:**
1. Check asset count (may need optimization for large datasets)
2. Verify caching implemented correctly
3. Profile with DelveDeep.ProfileConfigPerformance
4. Check for redundant asset loading

## Next Steps After Testing

### If All Tests Pass ✅

1. **Document Results**
   - Fill out test results template
   - Note any observations
   - Record performance metrics

2. **Commit Test Assets**
   - Save all created test assets
   - Commit to repository
   - Tag as "Phase1-Validated"

3. **Proceed to Next Phase**
   - Review Phase 2 requirements
   - Begin character system implementation
   - Integrate with configuration system

### If Tests Fail ❌

1. **Document Failures**
   - Record which tests failed
   - Capture error messages
   - Note reproduction steps

2. **Prioritize Issues**
   - Critical: Module loading, compilation errors
   - High: Validation failures, performance issues
   - Medium: Console command issues, hot-reload problems
   - Low: Minor UI issues, cosmetic problems

3. **Fix and Retest**
   - Address critical issues first
   - Rerun failed tests after fixes
   - Verify fixes don't break passing tests

## Automated Testing (Future)

For CI/CD integration, use these commands:

```bash
# Run automated tests
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunTests DelveDeep" -unattended -nopause -nullrhi -log

# Generate test report
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunTests DelveDeep; Automation Report" -unattended -nopause -nullrhi -log

# Performance profiling
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="DelveDeep.ProfileConfigPerformance; DelveDeep.ShowConfigStats" -unattended -nopause -nullrhi -log
```

## Contact and Support

If you encounter issues not covered in this guide:

1. Check Output Log for detailed error messages
2. Review [ValidationSystem.md](Systems/ValidationSystem.md)
3. Consult [DataDrivenConfiguration.md](Systems/DataDrivenConfiguration.md)
4. Check [DOCUMENTATION_INDEX.md](../DOCUMENTATION_INDEX.md)

---

**Testing Checklist:**

- [ ] Prerequisites installed
- [ ] Project compiled successfully
- [ ] All 8 test sections completed
- [ ] Test results documented
- [ ] Performance targets met
- [ ] Issues logged and prioritized
- [ ] Test assets committed to repository

**Estimated Testing Time**: 2-3 hours for complete validation

**Last Updated**: October 23, 2025
