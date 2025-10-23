# Validation System Examples

This directory contains comprehensive examples demonstrating the DelveDeep Enhanced Validation System.

## Overview

The examples in `ValidationExamples.cpp` showcase all major features of the validation system:

1. **Basic Validation with Severity Levels** - Using Critical, Error, Warning, and Info severities
2. **Validation Templates** - Pre-built templates for common validation scenarios
3. **Custom Validation Rule Registration** - Creating and registering reusable validation rules
4. **Validation with Caching** - Performance optimization through result caching
5. **Validation Delegates** - Responding to validation events
6. **Report Export Formats** - Exporting reports as JSON, CSV, and HTML

## Using the Examples

The examples are provided for documentation and learning purposes. They are wrapped in `#if 0` to prevent compilation by default.

### To Compile and Run Examples

1. Open `ValidationExamples.cpp`
2. Change `#if 0` to `#if 1` at the top of the file
3. Call the example functions from your game code
4. Recompile the project

### Example Usage

```cpp
// In your game code (e.g., GameMode::BeginPlay())
#include "Examples/ValidationExamples.cpp"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Run basic validation example
    Example_BasicValidationWithSeverity();
    
    // Run validation with caching example
    UGameInstance* GameInstance = GetGameInstance();
    UDelveDeepCharacterData* TestData = NewObject<UDelveDeepCharacterData>();
    Example_ValidationWithCaching(GameInstance, TestData);
    
    // Run report export example
    Example_ReportExportFormats();
}
```

## Example Descriptions

### Example 1: Basic Validation with Severity Levels

Demonstrates:
- Adding issues with different severity levels (Critical, Error, Warning, Info)
- Checking for specific severity levels
- Getting issue counts by severity
- Overall validation status checking

**Key Takeaways**:
- Critical and Error severities fail validation
- Warning and Info severities allow validation to pass
- Use appropriate severity for each issue type

### Example 2: Using Validation Templates

Demonstrates:
- Numeric range validation
- Pointer validation
- Soft reference validation
- String validation
- Array size validation

**Key Takeaways**:
- Templates provide consistent error messages
- Templates reduce boilerplate code
- Templates support custom severity levels

### Example 3: Custom Validation Rule Registration

Demonstrates:
- Registering validation rules with the subsystem
- Using lambda expressions for rule logic
- Setting rule priority
- Automatic rule execution on validation

**Key Takeaways**:
- Rules are reusable across multiple validations
- Higher priority rules execute first
- Rules can be registered for specific classes

### Example 4: Validation with Caching

Demonstrates:
- Cache miss on first validation (full validation)
- Cache hit on subsequent validations (instant return)
- Cache invalidation when objects change
- Performance comparison between cached and uncached validation

**Key Takeaways**:
- Caching provides significant performance improvement
- Cache hit rate typically >90% for repeated validations
- Invalidate cache when objects are modified

### Example 5: Validation Delegates

Demonstrates:
- Pre-validation delegate (fires before validation)
- Post-validation delegate (fires after validation)
- Critical issue delegate (fires on critical/error issues)
- Delegate registration and unregistration

**Key Takeaways**:
- Delegates enable event-driven validation workflows
- Multiple delegates can be registered for same event
- Delegates receive object and context parameters

### Example 6: Report Export Formats

Demonstrates:
- Console report generation
- JSON export for programmatic analysis
- CSV export for spreadsheet analysis
- HTML export for interactive viewing

**Key Takeaways**:
- Different formats serve different purposes
- JSON is best for programmatic analysis
- CSV is best for spreadsheet analysis
- HTML is best for human viewing

## Best Practices Demonstrated

1. **Always Set Context Information**
   ```cpp
   Context.SystemName = TEXT("MySystem");
   Context.OperationName = TEXT("MyOperation");
   ```

2. **Use Appropriate Severity Levels**
   ```cpp
   Context.AddCritical(TEXT("Asset corrupted")); // Prevents operation
   Context.AddError(TEXT("Invalid value")); // Prevents operation
   Context.AddWarning(TEXT("Suboptimal setting")); // Allows operation
   Context.AddInfo(TEXT("Validation started")); // Informational
   ```

3. **Leverage Validation Templates**
   ```cpp
   DelveDeepValidation::ValidateRange(Value, Min, Max, TEXT("PropertyName"), Context);
   ```

4. **Enable Caching for Performance**
   ```cpp
   ValidationSubsystem->ValidateObjectWithCache(Object, Context);
   ```

5. **Use Delegates for Event-Driven Workflows**
   ```cpp
   ValidationSubsystem->OnPostValidation.AddLambda([](const UObject* Object, const FValidationContext& Context)
   {
       // Handle validation completion
   });
   ```

6. **Export Reports for Analysis**
   ```cpp
   FString JSONReport = Context.GetReportJSON();
   FFileHelper::SaveStringToFile(JSONReport, TEXT("Report.json"));
   ```

## Performance Considerations

The examples demonstrate performance-optimized validation:

- **Caching**: Example 4 shows 10-100x performance improvement with caching
- **Batch Validation**: Parallel execution for multiple objects
- **Template Usage**: Minimal overhead compared to manual validation
- **Delegate Overhead**: <5% performance impact

## Integration with Configuration Manager

The validation system is fully integrated with the configuration manager. See:

- `UDelveDeepConfigurationManager::RegisterValidationRules()`
- `UDelveDeepConfigurationManager::ValidateAllDataWithSubsystem()`
- `UDelveDeepCharacterData::PostLoad()`

## Additional Resources

- **Documentation**: `Documentation/Systems/EnhancedValidationSystem.md`
- **Tests**: `Source/DelveDeep/Private/Tests/EnhancedValidationTests.cpp`
- **API Reference**: See header files in `Source/DelveDeep/Public/`

## Console Commands for Testing

Try these console commands to test validation features:

```
DelveDeep.ValidateObject /Game/Data/Characters/DA_Character_Warrior
DelveDeep.ListValidationRules
DelveDeep.ShowValidationCache
DelveDeep.ShowValidationMetrics
DelveDeep.TestValidationSeverity
```

## Questions or Issues?

Refer to the comprehensive documentation in `Documentation/Systems/EnhancedValidationSystem.md` for detailed information about all validation system features.
