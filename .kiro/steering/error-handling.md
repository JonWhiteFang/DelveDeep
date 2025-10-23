# Error Handling & Validation

**✓ Implemented**: FValidationContext provides comprehensive validation infrastructure.

## Validation Pattern

Use `FValidationContext` for comprehensive validation with error/warning tracking:

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FValidationContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString OperationName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationWarnings;

    void AddError(const FString& Error);
    void AddWarning(const FString& Warning);
    bool IsValid() const { return ValidationErrors.Num() == 0; }
    FString GetReport() const;
    void Reset();
};
```

## Validation Usage

**Example from DelveDeep Configuration System:**

```cpp
bool ValidateGameplayAction(AActor* Actor, int32 Value, FValidationContext& Context)
{
    Context.SystemName = TEXT("Combat");
    Context.OperationName = TEXT("DealDamage");
    
    if (!IsValid(Actor))
    {
        Context.AddError(TEXT("Invalid actor reference"));
        return false;
    }
    
    if (Value <= 0)
    {
        Context.AddError(FString::Printf(TEXT("Invalid damage value: %d"), Value));
        return false;
    }
    
    if (Value > 10000)
    {
        Context.AddWarning(FString::Printf(TEXT("Unusually high damage value: %d"), Value));
    }
    
    return true;
}
```

## Null Pointer Checking

Always validate pointers before use:

```cpp
// Standard UObject validation
bool IsValidGameObject(const UObject* Object) const
{
    return Object && IsValid(Object) && !Object->IsPendingKill();
}

// Usage
if (!IsValidGameObject(CharacterData))
{
    UE_LOG(LogDelveDeepConfig, Error, TEXT("Invalid character data"));
    return nullptr;
}
```

## Data Asset Validation

**✓ Implemented**: All DelveDeep data assets validate in PostLoad().

All data assets should implement validation in `PostLoad()`:

```cpp
void UDelveDeepCharacterData::PostLoad()
{
    Super::PostLoad();
    
    FValidationContext Context;
    Context.SystemName = TEXT("Configuration");
    Context.OperationName = TEXT("LoadCharacterData");
    
    if (!Validate(Context))
    {
        UE_LOG(LogDelveDeepConfig, Error, TEXT("Character data validation failed: %s"), 
            *Context.GetReport());
    }
}

bool UDelveDeepCharacterData::Validate(FValidationContext& Context) const
{
    bool bIsValid = true;
    
    if (BaseHealth <= 0.0f || BaseHealth > 10000.0f)
    {
        Context.AddError(FString::Printf(
            TEXT("BaseHealth out of range: %.2f (expected 1-10000)"), BaseHealth));
        bIsValid = false;
    }
    
    if (BaseDamage <= 0.0f || BaseDamage > 1000.0f)
    {
        Context.AddError(FString::Printf(
            TEXT("BaseDamage out of range: %.2f (expected 1-1000)"), BaseDamage));
        bIsValid = false;
    }
    
    if (StartingWeapon.IsNull())
    {
        Context.AddWarning(TEXT("No starting weapon assigned"));
    }
    
    return bIsValid;
}
```

## Logging Severity Guidelines

### Error (Critical Issues)
Use for failures that prevent normal operation:

```cpp
UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to load character data: %s"), *CharacterName.ToString());
UE_LOG(LogDelveDeepCombat, Error, TEXT("Combat system initialization failed"));
```

### Warning (Potential Issues)
Use for issues that don't prevent operation but may cause problems:

```cpp
UE_LOG(LogDelveDeepConfig, Warning, TEXT("Data asset outside standard directory: %s"), *AssetPath);
UE_LOG(LogDelveDeepConfig, Warning, TEXT("Using fallback value for missing asset"));
```

### Display (Informational)
Use for normal operation messages:

```cpp
UE_LOG(LogDelveDeepConfig, Display, TEXT("Configuration system initialized: %d assets loaded"), AssetCount);
UE_LOG(LogDelveDeepCombat, Display, TEXT("Combat system ready"));
```

### Verbose (Debug Information)
Use for detailed debugging information:

```cpp
UE_LOG(LogDelveDeepConfig, Verbose, TEXT("Loading character data from: %s"), *AssetPath);
UE_LOG(LogDelveDeepCombat, Verbose, TEXT("Damage calculation: Base=%.2f, Modifier=%.2f"), BaseDamage, Modifier);
```

## Error Recovery Strategies

### Missing Assets
```cpp
const UDelveDeepCharacterData* GetCharacterData(FName CharacterName) const
{
    const UDelveDeepCharacterData* Data = CharacterDataCache.FindRef(CharacterName);
    
    if (!Data)
    {
        UE_LOG(LogDelveDeepConfig, Warning, 
            TEXT("Character data not found: %s. Using default."), *CharacterName.ToString());
        
        // Return default fallback
        return GetDefaultCharacterData();
    }
    
    return Data;
}
```

### Invalid Data
```cpp
void LoadMonsterConfig(const FDelveDeepMonsterConfig& Config)
{
    // Clamp invalid values to valid ranges
    float Health = FMath::Clamp(Config.Health, 1.0f, 100000.0f);
    float Damage = FMath::Max(Config.Damage, 0.0f);
    
    if (Health != Config.Health)
    {
        UE_LOG(LogDelveDeepConfig, Warning, 
            TEXT("Monster health clamped from %.2f to %.2f"), Config.Health, Health);
    }
    
    // Continue with clamped values
}
```

### Graceful Degradation
```cpp
void InitializeSystem()
{
    // Try to load optimal configuration
    if (!LoadOptimalConfig())
    {
        UE_LOG(LogDelveDeep, Warning, TEXT("Failed to load optimal config, using minimal config"));
        
        // Fall back to minimal configuration
        if (!LoadMinimalConfig())
        {
            UE_LOG(LogDelveDeep, Error, TEXT("Failed to load minimal config, system disabled"));
            bSystemEnabled = false;
            return;
        }
    }
    
    bSystemEnabled = true;
}
```

## Assertion Usage

Use `check()` for critical invariants that should never fail:

```cpp
void ProcessCombat()
{
    // This should never be null if system is initialized correctly
    check(CombatComponent != nullptr);
    
    // Process combat
}
```

Use `ensure()` for conditions that should be true but can be recovered from:

```cpp
void ApplyDamage(AActor* Target, float Damage)
{
    if (!ensure(Target != nullptr))
    {
        UE_LOG(LogDelveDeepCombat, Error, TEXT("Attempted to apply damage to null target"));
        return;
    }
    
    // Apply damage
}
```

## Data Validation Context Integration

Use Unreal's built-in data validation system:

```cpp
#if WITH_EDITOR
virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override
{
    EDataValidationResult Result = Super::IsDataValid(Context);
    
    if (BaseHealth <= 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("BaseHealth must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }
    
    if (StartingWeapon.IsNull())
    {
        Context.AddWarning(FText::FromString(TEXT("No starting weapon assigned")));
    }
    
    return Result;
}
#endif
```

## Error Handling Best Practices

**Based on DelveDeep Configuration System Implementation:**

1. **Always validate input parameters** before processing ✓
   - All data assets validate on load
   - Query functions validate parameters

2. **Log errors with context** (what failed, why, where) ✓
   - FValidationContext includes SystemName and OperationName
   - Formatted reports with detailed context

3. **Provide recovery paths** when possible ✓
   - Fallback to default values for missing assets
   - Graceful degradation on validation failures

4. **Use appropriate log severity** (Error, Warning, Display, Verbose) ✓
   - Errors for critical failures
   - Warnings for potential issues
   - Display for normal operations

5. **Return early** on validation failures to avoid cascading errors ✓
   - Validation checks return false immediately
   - Prevents invalid data from propagating

6. **Document expected behavior** in error messages ✓
   - Error messages include expected ranges
   - Clear indication of what went wrong

7. **Track validation metrics** for debugging and optimization ✓
   - Validation reports track error/warning counts
   - Console commands for validation testing

8. **Use FValidationContext for all validation** ✓
   - Consistent validation pattern across all systems
   - Formatted reporting for easy debugging

9. **Validate asset references** ✓
   - TSoftObjectPtr checked for null
   - Warnings for missing optional references

10. **Provide console commands for validation testing** ✓
    - DelveDeep.ValidateAllData
    - DelveDeep.ValidateSampleData
    - DelveDeep.TestValidationSystem
