# Blueprint Integration Guidelines

## Core Principle

All C++ classes must be Blueprint-ready. This enables future UE5 Editor integration and allows designers to extend functionality without code changes.

## Required UCLASS Specifiers

```cpp
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep")
class DELVEDEEP_API ADelveDeepActor : public AActor
{
    GENERATED_BODY()
    
public:
    ADelveDeepActor();
};
```

### Common UCLASS Specifiers

- **BlueprintType**: Allows the class to be used as a variable type in Blueprint
- **Blueprintable**: Allows creating Blueprint classes derived from this C++ class
- **Abstract**: Prevents direct instantiation (use for base classes)
- **NotBlueprintable**: Explicitly prevents Blueprint derivation
- **Category**: Organizes classes in the Blueprint palette

## UPROPERTY Specifiers

### Visibility Specifiers

```cpp
// Read-only in Blueprint, editable in Editor
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
float BaseHealth = 100.0f;

// Read and write in Blueprint, editable in Editor
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
float CurrentHealth = 100.0f;

// Read-only in Blueprint, not editable in Editor
UPROPERTY(BlueprintReadOnly, Category = "Stats")
float CalculatedDamage;

// Visible in Editor but not editable
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
int32 DebugCounter;
```

### Edit Conditions

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
bool bHasSpecialAbility = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", 
    meta = (EditCondition = "bHasSpecialAbility"))
float SpecialAbilityCooldown = 5.0f;
```

### Meta Specifiers for Properties

```cpp
// Clamp numeric values
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", 
    meta = (ClampMin = "1.0", ClampMax = "10000.0"))
float MaxHealth = 100.0f;

// UI slider range (different from clamp)
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", 
    meta = (UIMin = "0.0", UIMax = "100.0"))
float HealthPercentage = 100.0f;

// Multi-line text
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", 
    meta = (MultiLine = true))
FText Description;

// Asset bundles for soft references
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual", 
    meta = (AssetBundles = "Menu"))
TSoftClassPtr<class AGameMapTile> MapTileClass;

// Tooltip
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat",
    meta = (ToolTip = "Damage dealt per attack"))
float AttackDamage = 10.0f;
```

## UFUNCTION Specifiers

### BlueprintCallable vs BlueprintPure

```cpp
// BlueprintCallable: Can have side effects, shows execution pins
UFUNCTION(BlueprintCallable, Category = "DelveDeep|Combat")
void DealDamage(AActor* Target, float Damage);

// BlueprintPure: No side effects, no execution pins (for getters)
UFUNCTION(BlueprintPure, Category = "DelveDeep|Combat")
float GetCurrentHealth() const;

// Prevent const functions from being pure
UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "DelveDeep|Combat")
void ComplexCalculation() const
{
    // Expensive calculations that shouldn't be called multiple times
}
```

### Function Categories and Display Names

```cpp
// Organize functions in Blueprint palette
UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Stats")
void ModifyHealth(float Amount);

// Custom display name for return value
UFUNCTION(BlueprintCallable, Category = "DelveDeep|Combat", 
    meta = (ReturnDisplayName = "Success"))
bool TryAttackTarget(AActor* Target);

// Custom parameter display names
UFUNCTION(BlueprintCallable, Category = "DelveDeep|Combat")
void ApplyDamage(
    UPARAM(DisplayName = "Victim") AActor* Target,
    UPARAM(DisplayName = "Amount") float Damage
);
```

### Blueprint Implementable Events

```cpp
// Can only be implemented in Blueprint (no C++ implementation)
UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Events")
void OnHealthChanged(float NewHealth);

// Can be implemented in Blueprint, has C++ default implementation
UFUNCTION(BlueprintNativeEvent, Category = "DelveDeep|Events")
void OnDeath();

// C++ implementation (note the _Implementation suffix)
void ADelveDeepCharacter::OnDeath_Implementation()
{
    // Default C++ behavior
    UE_LOG(LogDelveDeep, Display, TEXT("Character died"));
}
```

### Editor-Only Functions

```cpp
// Callable from Editor UI (appears as button in Details panel)
UFUNCTION(CallInEditor, BlueprintCallable, Category = "DelveDeep|Debug")
void DebugPrintStats();
```

## Const Correctness for Blueprint

```cpp
// Const functions are automatically BlueprintPure unless specified otherwise
UFUNCTION(BlueprintPure, Category = "DelveDeep|Stats")
float GetMaxHealth() const { return MaxHealth; }

// Override automatic pure behavior for expensive const functions
UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "DelveDeep|Stats")
float CalculateComplexStat() const
{
    // Expensive calculation
    return ComplexValue;
}
```

## Soft Object References

Use `TSoftObjectPtr` and `TSoftClassPtr` for asset references to reduce memory footprint:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
TSoftObjectPtr<UDelveDeepWeaponData> StartingWeapon;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
TSoftClassPtr<ADelveDeepProjectile> ProjectileClass;

// Loading soft references
void LoadWeapon()
{
    if (!StartingWeapon.IsNull())
    {
        UDelveDeepWeaponData* LoadedWeapon = StartingWeapon.LoadSynchronous();
        if (LoadedWeapon)
        {
            // Use loaded weapon
        }
    }
}
```

## Blueprint Function Libraries

Create static function libraries for utility functions:

```cpp
UCLASS()
class DELVEDEEP_API UDelveDeepBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Utilities")
    static float CalculateDamageWithModifiers(float BaseDamage, float Modifier);
    
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Utilities")
    static bool IsValidTarget(AActor* Target);
};
```

## Enums for Blueprint

```cpp
UENUM(BlueprintType)
enum class EDelveDeepCharacterClass : uint8
{
    Warrior     UMETA(DisplayName = "Warrior"),
    Ranger      UMETA(DisplayName = "Ranger"),
    Mage        UMETA(DisplayName = "Mage"),
    Necromancer UMETA(DisplayName = "Necromancer")
};

// Usage in properties
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
EDelveDeepCharacterClass CharacterClass;
```

## Structs for Blueprint

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepCharacterStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Damage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MoveSpeed = 300.0f;
};
```

## Interfaces for Blueprint

```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class UDelveDeepInteractable : public UInterface
{
    GENERATED_BODY()
};

class IDelveDeepInteractable
{
    GENERATED_BODY()

public:
    // Can be implemented in C++ or Blueprint
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Instigator);

    // Blueprint-only implementation
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
    void OnInteractionComplete();
};
```

## Data Assets for Blueprint

```cpp
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepCharacterData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", 
        meta = (ClampMin = "1.0", ClampMax = "10000.0"))
    float BaseHealth = 100.0f;
};
```

## Best Practices

1. **Always use Category** to organize Blueprint nodes
2. **Use descriptive names** that make sense in visual scripting
3. **Provide tooltips** via meta specifiers for complex functions
4. **Use const correctness** - const functions become pure by default
5. **Avoid BlueprintPure for expensive operations** - use BlueprintPure=false
6. **Use soft references** (TSoftObjectPtr) to reduce memory usage
7. **Validate inputs** in Blueprint-callable functions
8. **Return meaningful values** - use bool for success/failure, provide error context
9. **Use UPARAM(DisplayName)** for clearer parameter names in Blueprint
10. **Document expected behavior** in function comments (visible in Blueprint tooltips)

## Common Pitfalls to Avoid

❌ **Don't** expose raw pointers without validation
❌ **Don't** make expensive const functions automatically pure
❌ **Don't** forget DELVEDEEP_API macro for public classes
❌ **Don't** use TArray/TMap in UPROPERTY without BlueprintReadWrite/ReadOnly
❌ **Don't** forget to add Category to UFUNCTION/UPROPERTY

✅ **Do** validate all Blueprint-callable function inputs
✅ **Do** use appropriate specifiers (EditAnywhere, BlueprintReadOnly, etc.)
✅ **Do** provide meaningful return values and output parameters
✅ **Do** use meta specifiers for better Editor experience
✅ **Do** test Blueprint integration even in code-only environment
