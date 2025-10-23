# Validation System

## Overview

The DelveDeep validation system provides a comprehensive framework for tracking and reporting validation errors and warnings during data loading, configuration, and runtime operations. The system uses `FValidationContext` to collect validation issues with detailed context information.

## Core Components

### FValidationContext

A Blueprint-compatible struct that tracks validation state and provides formatted reporting.

**Location**: `Source/DelveDeep/Public/DelveDeepValidation.h`

**Key Features**:
- Error and warning tracking with automatic logging
- Formatted report generation
- System and operation context tracking
- Blueprint integration support

## Usage

### Basic Validation

```cpp
#include "DelveDeepValidation.h"

bool ValidateCharacterData(const UDelveDeepCharacterData* Data)
{
    FValidationContext Context;
    Context.SystemName = TEXT("Configuration");
    Context.OperationName = TEXT("LoadCharacterData");
    
    // Validate health
    if (Data->BaseHealth <= 0.0f || Data->BaseHealth > 10000.0f)
    {
        Context.AddError(FString::Printf(
            TEXT("BaseHealth out of range: %.2f (expected 1-10000)"), 
            Data->BaseHealth));
    }
    
    // Validate damage
    if (Data->BaseDamage <= 0.0f)
    {
        Context.AddError(TEXT("BaseDamage must be positive"));
    }
    
    // Add warnings for potential issues
    if (Data->StartingWeapon.IsNull())
    {
        Context.AddWarning(TEXT("No starting weapon assigned"));
    }
    
    // Check validation result
    if (!Context.IsValid())
    {
        UE_LOG(LogDelveDeepConfig, Error, TEXT("Validation failed:\n%s"), 
            *Context.GetReport());
        return false;
    }
    
    return true;
}
```

### Data Asset Validation

```cpp
void UDelveDeepCharacterData::PostLoad()
{
    Super::PostLoad();
    
    FValidationContext Context;
    Context.SystemName = TEXT("Configuration");
    Context.OperationName = TEXT("LoadCharacterData");
    
    if (!Validate(Context))
    {
        UE_LOG(LogDelveDeepConfig, Error, 
            TEXT("Character data validation failed: %s"), *Context.GetReport());
    }
}

bool UDelveDeepCharacterData::Validate(FValidationContext& Context) const
{
    bool bIsValid = true;
    
    // Validate numeric ranges
    if (BaseHealth <= 0.0f || BaseHealth > 10000.0f)
    {
        Context.AddError(FString::Printf(
            TEXT("BaseHealth out of range: %.2f (expected 1-10000)"), BaseHealth));
        bIsValid = false;
    }
    
    // Validate asset references
    if (StartingWeapon.IsNull())
    {
        Context.AddWarning(TEXT("No starting weapon assigned"));
    }
    
    return bIsValid;
}
```

### Report Generation

```cpp
FValidationContext Context;
Context.SystemName = TEXT("Configuration");
Context.OperationName = TEXT("ValidateAllData");

// Add validation issues
Context.AddError(TEXT("Invalid health value"));
Context.AddWarning(TEXT("Missing weapon reference"));

// Generate formatted report
FString Report = Context.GetReport();
UE_LOG(LogDelveDeepConfig, Display, TEXT("\n%s"), *Report);
```

**Example Report Output**:
```
=== Validation Report ===
System: Configuration
Operation: ValidateAllData

Errors (1):
  1. Invalid health value

Warnings (1):
  1. Missing weapon reference

=== Summary ===
Status: FAILED
Total Errors: 1
Total Warnings: 1
```

## Console Commands

### DelveDeep.TestValidationSystem

Tests the validation system by creating a sample validation context with errors and warnings.

**Usage**:
```
DelveDeep.TestValidationSystem
```

### DelveDeep.ValidateSampleData

Validates sample data to demonstrate validation system usage.

**Usage**:
```
DelveDeep.ValidateSampleData
```

## API Reference

### FValidationContext

#### Properties

- **SystemName** (FString): Name of the system performing validation
- **OperationName** (FString): Name of the specific operation being validated
- **ValidationErrors** (TArray<FString>): Collection of validation errors
- **ValidationWarnings** (TArray<FString>): Collection of validation warnings

#### Methods

##### AddError(const FString& Error)

Adds an error to the validation context and logs it.

**Parameters**:
- `Error`: The error message to add

**Example**:
```cpp
Context.AddError(TEXT("Invalid health value"));
```

##### AddWarning(const FString& Warning)

Adds a warning to the validation context and logs it.

**Parameters**:
- `Warning`: The warning message to add

**Example**:
```cpp
Context.AddWarning(TEXT("Missing weapon reference"));
```

##### IsValid() const

Checks if the validation context has no errors.

**Returns**: `true` if there are no validation errors, `false` otherwise

**Example**:
```cpp
if (!Context.IsValid())
{
    // Handle validation failure
}
```

##### GetReport() const

Generates a formatted validation report with all errors and warnings.

**Returns**: A formatted string containing the validation report

**Example**:
```cpp
FString Report = Context.GetReport();
UE_LOG(LogDelveDeepConfig, Display, TEXT("\n%s"), *Report);
```

##### Reset()

Resets the validation context, clearing all errors and warnings.

**Example**:
```cpp
Context.Reset();
```

## Logging

The validation system uses the `LogDelveDeepConfig` logging category.

**Declaration**: `DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepConfig, Log, All)`

**Usage**:
```cpp
UE_LOG(LogDelveDeepConfig, Error, TEXT("Validation failed"));
UE_LOG(LogDelveDeepConfig, Warning, TEXT("Using fallback value"));
UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation passed"));
```

## Best Practices

1. **Always provide context**: Set `SystemName` and `OperationName` before validation
2. **Use descriptive error messages**: Include expected values and actual values
3. **Distinguish errors from warnings**: Errors prevent operation, warnings are informational
4. **Generate reports for debugging**: Use `GetReport()` to log comprehensive validation results
5. **Validate early**: Perform validation in `PostLoad()` for data assets
6. **Check IsValid()**: Always check validation result before proceeding

## Integration with Other Systems

The validation system is designed to integrate with:

- **Configuration Manager**: Validates all loaded data assets and tables
- **Data Assets**: Provides validation in `PostLoad()` and custom validation methods
- **Save System**: Validates save data integrity
- **Progression System**: Validates upgrade costs and dependencies

## Testing

Automated tests are provided in `Source/DelveDeep/Private/Tests/ValidationTests.cpp`:

- **FDelveDeepValidationContextErrorTest**: Tests error tracking functionality
- **FDelveDeepValidationContextReportTest**: Tests report generation
- **FDelveDeepValidationContextResetTest**: Tests reset functionality
- **FDelveDeepValidationContextMultipleIssuesTest**: Tests multiple errors and warnings

**Run tests**:
```
DelveDeep.RunTests Validation
```

## Performance Considerations

- Validation is performed during initialization and data loading (not per-frame)
- Error and warning messages are logged immediately for debugging
- Report generation is on-demand and not cached
- Minimal memory overhead (only stores error/warning strings)

## Future Enhancements

Potential improvements for future versions:

- Validation severity levels (Critical, Error, Warning, Info)
- Validation rule registration system
- Automatic validation on asset save (Editor integration)
- Validation metrics tracking (error frequency, common issues)
- Custom validation delegates for extensibility
