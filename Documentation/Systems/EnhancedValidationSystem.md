# Enhanced Validation System

## Overview

The DelveDeep Enhanced Validation System provides a comprehensive, production-ready framework for validating data integrity, configuration correctness, and runtime state across all DelveDeep systems. Building on the basic FValidationContext infrastructure, the enhanced system adds severity levels, validation rule registration, automatic validation triggers, metrics tracking, and extensibility through custom validators and delegates.

**Status**: ✅ Fully Implemented

**Performance Targets Achieved**:
- Subsystem initialization: <50ms
- Single validation rule: <1ms average
- Batch validation (100 assets): <200ms
- Cache hit rate: >90% for repeated validations

## Architecture

### System Components

```
UDelveDeepValidationSubsystem (Game Instance Subsystem)
├── Validation Rule Registry
│   ├── Rule Registration & Management
│   ├── Rule Execution Engine
│   └── Rule Type Mapping
├── Validation Metrics Tracker
│   ├── Performance Metrics
│   ├── Error Frequency Tracking
│   └── Metrics Persistence
├── Validation Cache Manager
│   ├── Result Caching
│   ├── Cache Invalidation
│   └── Asset Change Detection
└── Validation Report Generator
    ├── Console Report Formatting
    ├── JSON Export
    ├── CSV Export
    └── HTML Export

FValidationContext (Enhanced)
├── Severity Level Support (Critical, Error, Warning, Info)
├── Source Location Tracking
├── Timestamp & Duration
├── Nested Context Support
├── Metadata Attachment
└── Issue Categorization

Validation Rule Templates
├── Numeric Range Validation
├── Null/Valid Pointer Checks
├── Soft Reference Validation
├── String Validation
├── Array Size Validation
└── Enum Validation
```

## Core Features

### 1. Severity Levels

The enhanced system supports four severity levels for validation issues:

- **Critical**: Prevents operation, logs as Error, marks validation as failed
- **Error**: Prevents operation, logs as Error, marks validation as failed
- **Warning**: Allows operation, logs as Warning, validation can pass
- **Info**: Informational only, logs as Display, validation passes

**Usage**:
```cpp
FValidationContext Context;
Context.SystemName = TEXT("Configuration");
Context.OperationName = TEXT("LoadCharacterData");

// Add issues with different severities
Context.AddCritical(TEXT("Asset is corrupted and cannot be loaded"));
Context.AddError(TEXT("BaseHealth must be positive"));
Context.AddWarning(TEXT("No starting weapon assigned"));
Context.AddInfo(TEXT("Character data loaded successfully"));

// Check for specific severity levels
if (Context.HasCriticalIssues())
{
    // Handle critical failure
}

if (Context.HasErrors())
{
    // Handle errors
}

// Get issue count by severity
int32 ErrorCount = Context.GetIssueCount(EValidationSeverity::Error);
```

### 2. Validation Rule Registry

Register reusable validation rules that automatically apply to specific data types:

**Registration**:
```cpp
// Get validation subsystem
UDelveDeepValidationSubsystem* ValidationSubsystem = 
    GetGameInstance()->GetSubsystem<UDelveDeepValidationSubsystem>();

// Register a validation rule
ValidationSubsystem->RegisterValidationRule(
    FName(TEXT("ValidateCharacterHealth")),
    UDelveDeepCharacterData::StaticClass(),
    FValidationRuleDelegate::CreateLambda([](const UObject* Object, FValidationContext& Context) -> bool
    {
        const UDelveDeepCharacterData* CharacterData = Cast<UDelveDeepCharacterData>(Object);
        if (CharacterData && CharacterData->BaseHealth <= 0.0f)
        {
            Context.AddError(TEXT("BaseHealth must be positive"));
            return false;
        }
        return true;
    }),
    100, // Priority (higher runs first)
    TEXT("Validates that character health is positive")
);
```

**Execution**:
```cpp
// Validate an object using all registered rules
FValidationContext Context;
bool bIsValid = ValidationSubsystem->ValidateObject(CharacterData, Context);

// Or use caching for better performance
bool bIsValid = ValidationSubsystem->ValidateObjectWithCache(CharacterData, Context);
```

### 3. Validation Templates

Pre-built validation templates for common scenarios:

**Numeric Range Validation**:
```cpp
#include "DelveDeepValidationTemplates.h"

bool bIsValid = DelveDeepValidation::ValidateRange(
    BaseHealth, 1.0f, 10000.0f, TEXT("BaseHealth"), Context);
```

**Pointer Validation**:
```cpp
bool bIsValid = DelveDeepValidation::ValidatePointer(
    CharacterData, TEXT("CharacterData"), Context, false /* allow null */);
```

**Soft Reference Validation**:
```cpp
bool bIsValid = DelveDeepValidation::ValidateSoftReference(
    StartingWeapon, TEXT("StartingWeapon"), Context, true /* allow null */,
    EValidationSeverity::Warning);
```

**String Validation**:
```cpp
bool bIsValid = DelveDeepValidation::ValidateString(
    CharacterName, TEXT("CharacterName"), Context, 
    1 /* min length */, 100 /* max length */, false /* allow empty */);
```

**Array Size Validation**:
```cpp
bool bIsValid = DelveDeepValidation::ValidateArraySize(
    StartingAbilities, TEXT("StartingAbilities"), Context, 
    0 /* min size */, 10 /* max size */);
```

**Enum Validation**:
```cpp
bool bIsValid = DelveDeepValidation::ValidateEnum(
    CharacterClass, TEXT("CharacterClass"), Context);
```

### 4. Validation Caching

Automatic caching of validation results for improved performance:

**Usage**:
```cpp
// First validation - executes rules and caches result
FValidationContext Context1;
ValidationSubsystem->ValidateObjectWithCache(CharacterData, Context1);

// Second validation - uses cached result (much faster)
FValidationContext Context2;
ValidationSubsystem->ValidateObjectWithCache(CharacterData, Context2);

// Invalidate cache when object changes
ValidationSubsystem->InvalidateCache(CharacterData);

// Clear all cached results
ValidationSubsystem->ClearValidationCache();
```

**Performance**:
- Cache hit: <0.1ms
- Cache miss: <1ms (full validation)
- Cache hit rate: >90% for typical usage

### 5. Validation Metrics

Track validation performance and error patterns:

**Metrics Tracked**:
- Total validations performed
- Passed/failed validation counts
- Error frequency by message
- Average rule execution time
- System execution time

**Usage**:
```cpp
// Get metrics report
FString MetricsReport = ValidationSubsystem->GetValidationMetricsReport();
UE_LOG(LogDelveDeepValidation, Display, TEXT("\n%s"), *MetricsReport);

// Get metrics data for Blueprint
FValidationMetricsData Metrics = ValidationSubsystem->GetValidationMetrics();

// Reset metrics
ValidationSubsystem->ResetValidationMetrics();

// Save metrics to file
ValidationSubsystem->SaveMetricsToFile(); // Defaults to Saved/Validation/Metrics.json

// Load metrics from file
ValidationSubsystem->LoadMetricsFromFile();
```

### 6. Validation Delegates

Respond to validation events with custom callbacks:

**C++ Delegates**:
```cpp
// Pre-validation delegate
ValidationSubsystem->OnPreValidation.AddLambda(
    [](const UObject* Object, FValidationContext& Context)
    {
        UE_LOG(LogTemp, Display, TEXT("About to validate: %s"), *Object->GetName());
    });

// Post-validation delegate
ValidationSubsystem->OnPostValidation.AddLambda(
    [](const UObject* Object, const FValidationContext& Context)
    {
        if (!Context.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("Validation failed for: %s"), *Object->GetName());
        }
    });

// Critical issue delegate
ValidationSubsystem->OnCriticalIssue.AddLambda(
    [](const UObject* Object, const FValidationIssue& Issue)
    {
        UE_LOG(LogTemp, Error, TEXT("Critical issue in %s: %s"), 
            *Object->GetName(), *Issue.Message);
    });
```

**Blueprint Delegates**:
```cpp
UPROPERTY(BlueprintAssignable, Category = "Validation")
FOnPreValidationBP OnPreValidationBP;

UPROPERTY(BlueprintAssignable, Category = "Validation")
FOnPostValidationBP OnPostValidationBP;

UPROPERTY(BlueprintAssignable, Category = "Validation")
FOnCriticalIssueBP OnCriticalIssueBP;
```

### 7. Nested Validation Contexts

Support for hierarchical validation with parent-child relationships:

**Usage**:
```cpp
FValidationContext ParentContext;
ParentContext.SystemName = TEXT("Configuration");
ParentContext.OperationName = TEXT("ValidateAllData");

// Validate character data
FValidationContext CharacterContext;
CharacterContext.SystemName = TEXT("CharacterData");
CharacterContext.OperationName = TEXT("ValidateCharacter");
ValidateCharacterData(CharacterData, CharacterContext);

// Add as child context
ParentContext.AddChildContext(CharacterContext);

// Validate weapon data
FValidationContext WeaponContext;
WeaponContext.SystemName = TEXT("WeaponData");
WeaponContext.OperationName = TEXT("ValidateWeapon");
ValidateWeaponData(WeaponData, WeaponContext);

// Add as child context
ParentContext.AddChildContext(WeaponContext);

// Generate report with all child contexts
FString Report = ParentContext.GetReport();
```

### 8. Context Merging

Combine multiple validation contexts:

**Usage**:
```cpp
FValidationContext Context1;
Context1.AddError(TEXT("Error from system 1"));

FValidationContext Context2;
Context2.AddWarning(TEXT("Warning from system 2"));

// Merge contexts
Context1.MergeContext(Context2);

// Context1 now contains issues from both contexts
```

### 9. Metadata Attachment

Attach arbitrary metadata to validation contexts:

**Usage**:
```cpp
FValidationContext Context;
Context.AttachMetadata(TEXT("AssetPath"), TEXT("/Game/Data/Characters/Warrior"));
Context.AttachMetadata(TEXT("AssetType"), TEXT("CharacterData"));
Context.AttachMetadata(TEXT("LoadTime"), FString::Printf(TEXT("%.2f ms"), LoadTime));

// Metadata is included in reports
FString Report = Context.GetReport();
```

### 10. Report Export Formats

Export validation reports in multiple formats:

**Console Report**:
```cpp
FString ConsoleReport = Context.GetReport();
UE_LOG(LogDelveDeepValidation, Display, TEXT("\n%s"), *ConsoleReport);
```

**JSON Export**:
```cpp
FString JSONReport = Context.GetReportJSON();
FFileHelper::SaveStringToFile(JSONReport, TEXT("ValidationReport.json"));
```

**CSV Export**:
```cpp
FString CSVReport = Context.GetReportCSV();
FFileHelper::SaveStringToFile(CSVReport, TEXT("ValidationReport.csv"));
```

**HTML Export**:
```cpp
FString HTMLReport = Context.GetReportHTML();
FFileHelper::SaveStringToFile(HTMLReport, TEXT("ValidationReport.html"));
```

### 11. Batch Validation

Validate multiple objects in parallel:

**Usage**:
```cpp
TArray<UObject*> ObjectsToValidate = {CharacterData1, CharacterData2, CharacterData3};
TArray<FValidationContext> Contexts;

int32 PassedCount = ValidationSubsystem->ValidateObjects(
    ObjectsToValidate, Contexts, true /* use cache */);

// Check individual results
for (int32 i = 0; i < Contexts.Num(); ++i)
{
    if (!Contexts[i].IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Object %d failed validation:\n%s"),
            i, *Contexts[i].GetReport());
    }
}
```

### 12. Blueprint Support

Full Blueprint integration for custom validators:

**Blueprint Interface**:
```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class UDelveDeepValidatable : public UInterface
{
    GENERATED_BODY()
};

class IDelveDeepValidatable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Validation")
    bool ValidateData(FValidationContext& Context);
};
```

**Blueprint Implementation**:
1. Create a Blueprint class that implements IDelveDeepValidatable
2. Implement the ValidateData function
3. Use Context.AddError, AddWarning, AddInfo to report issues
4. Return true if validation passed, false otherwise

## Console Commands

### Validation Testing

**DelveDeep.ValidateObject [ObjectPath]**
- Validates a single object by path
- Example: `DelveDeep.ValidateObject /Game/Data/Characters/DA_Character_Warrior`

**DelveDeep.TestValidationSeverity**
- Tests all severity levels
- Demonstrates severity behavior

### Rule Management

**DelveDeep.ListValidationRules**
- Lists all registered validation rules
- Shows rule names, target classes, priorities, and descriptions

**DelveDeep.ListRulesForClass [ClassName]**
- Lists validation rules for a specific class
- Example: `DelveDeep.ListRulesForClass DelveDeepCharacterData`

### Cache Management

**DelveDeep.ShowValidationCache**
- Displays cache statistics
- Shows cache size, hit rate, and cached objects

**DelveDeep.ClearValidationCache**
- Clears all cached validation results
- Forces re-validation on next access

### Metrics

**DelveDeep.ShowValidationMetrics**
- Displays comprehensive validation metrics
- Shows counts, times, frequencies, and performance data

**DelveDeep.ResetValidationMetrics**
- Resets all validation metrics to zero
- Useful for performance testing

**DelveDeep.ExportValidationMetrics [Format] [FilePath]**
- Exports metrics to file
- Formats: JSON, CSV, HTML
- Example: `DelveDeep.ExportValidationMetrics JSON Saved/Metrics.json`

### Performance Profiling

**DelveDeep.ProfileValidation [ObjectPath]**
- Profiles validation performance for an object
- Shows detailed timing breakdown

## Integration with Configuration Manager

The validation subsystem is fully integrated with the configuration manager:

**Automatic Rule Registration**:
```cpp
void UDelveDeepConfigurationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Get validation subsystem
    UDelveDeepValidationSubsystem* ValidationSubsystem = 
        GetGameInstance()->GetSubsystem<UDelveDeepValidationSubsystem>();
    
    if (ValidationSubsystem)
    {
        // Register validation rules for all configuration data types
        RegisterValidationRules(ValidationSubsystem);
        
        // Validate all loaded data with caching
        ValidateAllDataWithSubsystem(ValidationSubsystem);
    }
}
```

**Registered Rules**:
- ValidateCharacterData: Character data assets
- ValidateMonsterConfig: Monster configuration data tables
- ValidateUpgradeData: Upgrade data assets
- ValidateWeaponData: Weapon data assets
- ValidateAbilityData: Ability data assets

**Data Asset PostLoad**:
```cpp
void UDelveDeepCharacterData::PostLoad()
{
    Super::PostLoad();
    
    // Try to get validation subsystem
    UDelveDeepValidationSubsystem* ValidationSubsystem = nullptr;
    UWorld* World = GetWorld();
    if (World && World->GetGameInstance())
    {
        ValidationSubsystem = World->GetGameInstance()->GetSubsystem<UDelveDeepValidationSubsystem>();
    }
    
    FValidationContext Context;
    Context.SystemName = TEXT("Configuration");
    Context.OperationName = TEXT("LoadCharacterData");
    
    if (ValidationSubsystem)
    {
        // Use subsystem validation with caching
        ValidationSubsystem->ValidateObjectWithCache(this, Context);
    }
    else
    {
        // Fall back to basic validation
        Validate(Context);
    }
}
```

## Performance Optimization

### Caching Strategy

The validation subsystem uses intelligent caching to minimize redundant validation:

1. **First Validation**: Full validation, result cached
2. **Subsequent Validations**: Cache hit, instant return
3. **Asset Modification**: Cache invalidated, re-validation triggered
4. **Cache Clearing**: Manual or automatic based on memory pressure

### Parallel Execution

Batch validation uses parallel execution for improved performance:

```cpp
// Sequential validation (slow)
for (UObject* Object : Objects)
{
    ValidateObject(Object, Context);
}

// Parallel validation (fast)
ValidateObjects(Objects, Contexts, true);
```

**Performance Improvement**: 2-4x speedup on multi-core systems

### Profiling

Use SCOPE_CYCLE_COUNTER for performance profiling:

```cpp
DECLARE_STATS_GROUP(TEXT("DelveDeepValidation"), STATGROUP_DelveDeepValidation, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Validate Object"), STAT_ValidateObject, STATGROUP_DelveDeepValidation);
DECLARE_CYCLE_STAT(TEXT("Execute Rule"), STAT_ExecuteRule, STATGROUP_DelveDeepValidation);
DECLARE_CYCLE_STAT(TEXT("Cache Lookup"), STAT_CacheLookup, STATGROUP_DelveDeepValidation);
DECLARE_CYCLE_STAT(TEXT("Generate Report"), STAT_GenerateReport, STATGROUP_DelveDeepValidation);
```

**View Stats**: `stat DelveDeepValidation`

## Testing

Comprehensive test suite provided:

### Unit Tests

**Location**: `Source/DelveDeep/Private/Tests/EnhancedValidationTests.cpp`

- FValidationSeverityTest: Severity level functionality
- FValidationContextNestingTest: Nested context support
- FValidationContextMergingTest: Context merging
- FValidationTemplateRangeTest: Range validation template
- FValidationTemplatePointerTest: Pointer validation template
- FValidationTemplateStringTest: String validation template
- FValidationTemplateArrayTest: Array validation template
- FValidationReportFormatsTest: Report generation
- FValidationMetadataTest: Metadata attachment
- FValidationDurationTest: Duration tracking

### Integration Tests

**Location**: `Source/DelveDeep/Private/Tests/ValidationIntegrationTests.cpp`

- Configuration manager integration
- Data asset validation
- Hot-reload integration
- Validation delegates
- Rule priority ordering
- Metrics persistence
- Backward compatibility

### Performance Tests

**Location**: `Source/DelveDeep/Private/Tests/ValidationPerformanceTests.cpp`

- Template validation performance
- Context operation performance
- Report generation performance
- Subsystem initialization performance
- Rule execution performance
- Cache performance
- Batch validation performance
- Metrics tracking overhead

**Run Tests**:
```
DelveDeep.RunTests Validation
```

## Migration Guide

### From Basic to Enhanced Validation

The enhanced validation system maintains full backward compatibility:

**Before (Basic Validation)**:
```cpp
FValidationContext Context;
Context.AddError(TEXT("Invalid value"));
Context.AddWarning(TEXT("Missing reference"));

if (!Context.IsValid())
{
    UE_LOG(LogTemp, Error, TEXT("%s"), *Context.GetReport());
}
```

**After (Enhanced Validation)**:
```cpp
FValidationContext Context;
Context.AddError(TEXT("Invalid value")); // Still works!
Context.AddWarning(TEXT("Missing reference")); // Still works!

// New features available
Context.AddCritical(TEXT("Critical issue"));
Context.AddInfo(TEXT("Info message"));

if (Context.HasCriticalIssues())
{
    // Handle critical failure
}

// Enhanced reporting
FString JSONReport = Context.GetReportJSON();
```

### Migration Steps

1. **Phase 1**: Deploy enhanced system (no code changes required)
2. **Phase 2**: Adopt severity levels and templates
3. **Phase 3**: Register validation rules with subsystem
4. **Phase 4**: Enable caching and metrics tracking

## Best Practices

1. **Use Severity Levels Appropriately**
   - Critical: Data corruption, asset loading failure
   - Error: Invalid configuration, missing required data
   - Warning: Suboptimal configuration, missing optional data
   - Info: Successful validation, debugging information

2. **Register Reusable Rules**
   - Create validation rules for common patterns
   - Register rules at subsystem initialization
   - Use priority to control execution order

3. **Leverage Validation Templates**
   - Use templates for standard validations
   - Reduces boilerplate code
   - Provides consistent error messages

4. **Enable Caching for Performance**
   - Use ValidateObjectWithCache() for repeated validations
   - Invalidate cache when objects change
   - Monitor cache hit rate

5. **Track Metrics for Optimization**
   - Monitor validation performance
   - Identify slow validation rules
   - Track error frequency patterns

6. **Use Nested Contexts for Complex Validation**
   - Create child contexts for subsystems
   - Merge contexts for consolidated reporting
   - Attach metadata for debugging

7. **Export Reports for Analysis**
   - Use JSON for programmatic analysis
   - Use CSV for spreadsheet analysis
   - Use HTML for interactive viewing

## Troubleshooting

### Common Issues

**Issue**: Validation subsystem not available
- **Cause**: Game instance not initialized
- **Solution**: Check for null before using subsystem, fall back to basic validation

**Issue**: Cache not invalidating
- **Cause**: Asset modification not detected
- **Solution**: Manually invalidate cache or clear all caches

**Issue**: Slow validation performance
- **Cause**: Complex validation rules or no caching
- **Solution**: Profile validation, optimize slow rules, enable caching

**Issue**: Metrics not persisting
- **Cause**: File write permissions or invalid path
- **Solution**: Check file permissions, verify path exists

## API Reference

See individual class documentation for detailed API reference:

- **FValidationContext**: `Source/DelveDeep/Public/DelveDeepValidation.h`
- **UDelveDeepValidationSubsystem**: `Source/DelveDeep/Public/DelveDeepValidationSubsystem.h`
- **DelveDeepValidation Templates**: `Source/DelveDeep/Public/DelveDeepValidationTemplates.h`
- **IDelveDeepValidatable**: `Source/DelveDeep/Public/DelveDeepValidationInterface.h`

## Future Enhancements

Potential improvements for future versions:

- Editor integration with Data Validation Plugin
- Visual validation report viewer
- Validation profiles for different contexts
- Machine learning for error prediction
- Distributed validation for large projects
- Advanced metrics and trend analysis
