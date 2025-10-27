# Design Document

## Overview

The Character System Foundation provides a component-based architecture for all playable characters in DelveDeep, supporting four distinct character classes (Warrior, Ranger, Mage, Necromancer) through modular design. The system leverages Unreal Engine 5's APaperCharacter for 2D sprite rendering, UActorComponent for modular functionality, and integrates seamlessly with existing DelveDeep systems (Configuration, Events, Telemetry).

The architecture follows a clear separation of concerns: ADelveDeepCharacter provides the base actor with core functionality, UDelveDeepCharacterComponent serves as the base for all character components, and specialized components (Stats, Abilities, Equipment) handle specific aspects of character behavior. Class-specific actors (ADelveDeepWarrior, ADelveDeepRanger, ADelveDeepMage, ADelveDeepNecromancer) inherit from the base and override virtual methods for unique mechanics.

This design prioritizes performance (tick disabled by default, cached calculations, object pooling), maintainability (component-based, data-driven), and extensibility (Blueprint-ready, virtual methods for overriding). All character data is loaded from UDelveDeepCharacterData assets via UDelveDeepConfigurationManager, ensuring designers can balance characters without code changes.

## Architecture

### System Components

```
Character System Foundation
├── Base Character Actor (ADelveDeepCharacter)
│   ├── Inherits from APaperCharacter
│   ├── Manages component lifecycle
│   ├── Loads data from UDelveDeepConfigurationManager
│   └── Broadcasts events via UDelveDeepEventSubsystem
├── Character Components
│   ├── Base Component (UDelveDeepCharacterComponent)
│   ├── Stats Component (UDelveDeepStatsComponent)
│   ├── Abilities Component (UDelveDeepAbilitiesComponent)
│   └── Equipment Component (UDelveDeepEquipmentComponent)
├── Character Classes
│   ├── Warrior (ADelveDeepWarrior) - Rage resource
│   ├── Ranger (ADelveDeepRanger) - Energy resource
│   ├── Mage (ADelveDeepMage) - Mana resource
│   └── Necromancer (ADelveDeepNecromancer) - Soul resource
├── Data Integration
│   ├── UDelveDeepCharacterData (from Configuration System)
│   ├── UDelveDeepWeaponData (from Configuration System)
│   └── UDelveDeepAbilityData (from Configuration System)
└── Event Integration
    ├── Character.Death
    ├── Character.Damaged
    ├── Character.Healed
    ├── Character.StatChanged
    ├── Character.AbilityUsed
    └── Character.WeaponEquipped
```

### Class Hierarchy

```
APaperCharacter (UE5)
└── ADelveDeepCharacter (Base)
    ├── ADelveDeepWarrior (Melee, Rage)
    ├── ADelveDeepRanger (Ranged, Energy)
    ├── ADelveDeepMage (Spells, Mana)
    └── ADelveDeepNecromancer (Minions, Souls)

UActorComponent (UE5)
└── UDelveDeepCharacterComponent (Base)
    ├── UDelveDeepStatsComponent
    ├── UDelveDeepAbilitiesComponent
    └── UDelveDeepEquipmentComponent
```


## Components and Interfaces

### Base Character Actor

```cpp
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepCharacter : public APaperCharacter
{
    GENERATED_BODY()

public:
    ADelveDeepCharacter();

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Character data
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
    const UDelveDeepCharacterData* GetCharacterData() const { return CharacterData; }

    // Stats access
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    UDelveDeepStatsComponent* GetStatsComponent() const { return StatsComponent; }

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetCurrentHealth() const;

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetCurrentResource() const;  // Mana/Energy/Rage/Souls

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetMaxResource() const;

    // Damage and healing
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Combat")
    virtual void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Combat")
    virtual void Heal(float HealAmount);

    // Death handling
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|State")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|State")
    virtual void Die();

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|State")
    virtual void Respawn();

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
    void OnDeath();

    UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
    void OnDamaged(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
    void OnHealed(float HealAmount);

    UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
    void OnStatChanged(FName StatName, float OldValue, float NewValue);

protected:
    // Character data
    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character")
    const UDelveDeepCharacterData* CharacterData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character")
    FName CharacterClassName;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character|Components")
    UDelveDeepStatsComponent* StatsComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character|Components")
    UDelveDeepAbilitiesComponent* AbilitiesComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character|Components")
    UDelveDeepEquipmentComponent* EquipmentComponent;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|State")
    bool bIsDead;

    // Initialization
    virtual void InitializeFromData();
    virtual void InitializeComponents();

    // Class-specific overrides
    virtual void OnResourceChanged(float OldValue, float NewValue) {}
    virtual void OnAbilityUsed(const UDelveDeepAbilityData* Ability) {}

private:
    // Event broadcasting
    void BroadcastDeathEvent();
    void BroadcastDamageEvent(float DamageAmount, AActor* DamageSource);
    void BroadcastHealEvent(float HealAmount);

    // Validation
    bool ValidateCharacterData(FValidationContext& Context) const;

    // Death timer
    FTimerHandle DeathTimerHandle;
    void DestroyAfterDeath();
};
```

### Base Character Component

```cpp
UCLASS(BlueprintType, Blueprintable, Abstract, Category = "DelveDeep|Character")
class DELVEDEEP_API UDelveDeepCharacterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDelveDeepCharacterComponent();

    // Owner access
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
    ADelveDeepCharacter* GetCharacterOwner() const;

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void BeginDestroy() override;

protected:
    // Cached owner reference
    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character")
    ADelveDeepCharacter* CharacterOwner;

    // Initialization
    virtual void InitializeComponent() override;
    virtual void InitializeFromCharacterData(const UDelveDeepCharacterData* CharacterData) {}

    // Validation
    virtual bool ValidateComponent(FValidationContext& Context) const;
};
```


### Stats Component

```cpp
UCLASS(BlueprintType, Category = "DelveDeep|Character|Stats")
class DELVEDEEP_API UDelveDeepStatsComponent : public UDelveDeepCharacterComponent
{
    GENERATED_BODY()

public:
    UDelveDeepStatsComponent();

    // Health
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Stats")
    void ModifyHealth(float Delta);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Stats")
    void SetHealth(float NewHealth);

    // Resource (Mana/Energy/Rage/Souls)
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetCurrentResource() const { return CurrentResource; }

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetMaxResource() const { return MaxResource; }

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Stats")
    void ModifyResource(float Delta);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Stats")
    void SetResource(float NewResource);

    // Damage and move speed
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetBaseDamage() const { return BaseDamage; }

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetMoveSpeed() const { return MoveSpeed; }

    // Stat modifiers
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Stats")
    void AddStatModifier(FName StatName, float Modifier, float Duration);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Stats")
    void RemoveStatModifier(FName StatName);

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
    float GetModifiedStat(FName StatName) const;

    // Reset
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Stats")
    void ResetToMaxValues();

protected:
    // Base stats (from data asset)
    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Stats")
    float BaseHealth;

    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Stats")
    float BaseResource;

    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Stats")
    float BaseDamage;

    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Stats")
    float BaseMoveSpeed;

    // Current stats
    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Stats")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Stats")
    float CurrentResource;

    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Stats")
    float MaxResource;

    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Stats")
    float MoveSpeed;

    // Stat modifiers
    UPROPERTY()
    TMap<FName, FDelveDeepStatModifier> ActiveModifiers;

    // Initialization
    virtual void InitializeFromCharacterData(const UDelveDeepCharacterData* CharacterData) override;

    // Stat calculation
    void RecalculateStats();
    float ApplyModifiers(FName StatName, float BaseValue) const;

    // Event broadcasting
    void BroadcastStatChanged(FName StatName, float OldValue, float NewValue);
    void BroadcastHealthChanged(float OldHealth, float NewHealth);
    void BroadcastResourceChanged(float OldResource, float NewResource);

    // Validation
    virtual bool ValidateComponent(FValidationContext& Context) const override;

private:
    // Cached calculated stats
    bool bStatsDirty;
    void MarkStatsDirty() { bStatsDirty = true; }

    // Modifier cleanup
    void CleanupExpiredModifiers();
    FTimerHandle ModifierCleanupTimerHandle;
};

// Stat modifier struct
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepStatModifier
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Modifier = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Duration = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float RemainingTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    bool bIsPercentage = false;

    bool IsExpired() const { return Duration > 0.0f && RemainingTime <= 0.0f; }
};
```


### Character Class Implementations

```cpp
// Warrior - Melee combat with Rage resource
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepWarrior : public ADelveDeepCharacter
{
    GENERATED_BODY()

public:
    ADelveDeepWarrior();

    // Rage generation
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Warrior")
    void GenerateRage(float Amount);

    // Cleave attack (hits multiple enemies)
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Warrior")
    void PerformCleaveAttack();

protected:
    virtual void TakeDamage(float DamageAmount, AActor* DamageSource) override;
    virtual void OnResourceChanged(float OldValue, float NewValue) override;

private:
    static constexpr float RagePerDamageDealt = 5.0f;
    static constexpr float RagePerDamageTaken = 10.0f;
    static constexpr float MaxRage = 100.0f;
};

// Ranger - Ranged combat with Energy resource
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepRanger : public ADelveDeepCharacter
{
    GENERATED_BODY()

public:
    ADelveDeepRanger();

    // Energy regeneration
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Ranger")
    void RegenerateEnergy(float DeltaTime);

    // Piercing shot (hits multiple enemies in line)
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Ranger")
    void PerformPiercingShot();

protected:
    virtual void BeginPlay() override;
    virtual void OnResourceChanged(float OldValue, float NewValue) override;

private:
    static constexpr float EnergyRegenRate = 10.0f;  // per second
    static constexpr float MaxEnergy = 100.0f;

    FTimerHandle EnergyRegenTimerHandle;
    void StartEnergyRegeneration();
};

// Mage - Spell casting with Mana resource
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepMage : public ADelveDeepCharacter
{
    GENERATED_BODY()

public:
    ADelveDeepMage();

    // Mana regeneration
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Mage")
    void RegenerateMana(float DeltaTime);

    // Fireball spell (AoE damage)
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Mage")
    void CastFireball(FVector TargetLocation);

protected:
    virtual void BeginPlay() override;
    virtual void OnResourceChanged(float OldValue, float NewValue) override;

private:
    static constexpr float ManaRegenRate = 5.0f;  // per second
    static constexpr float MaxMana = 100.0f;

    FTimerHandle ManaRegenTimerHandle;
    void StartManaRegeneration();
};

// Necromancer - Minion summoning with Soul resource
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepNecromancer : public ADelveDeepCharacter
{
    GENERATED_BODY()

public:
    ADelveDeepNecromancer();

    // Soul collection
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Necromancer")
    void CollectSoul();

    // Summon minion
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Necromancer")
    void SummonMinion();

    // Get active minions
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Necromancer")
    int32 GetActiveMinionCount() const { return ActiveMinions.Num(); }

protected:
    virtual void BeginPlay() override;
    virtual void OnResourceChanged(float OldValue, float NewValue) override;

private:
    static constexpr float MaxSouls = 10.0f;
    static constexpr int32 MaxMinions = 3;

    UPROPERTY()
    TArray<AActor*> ActiveMinions;

    void RegisterForEnemyDeathEvents();
    void OnEnemyDeath(const FDelveDeepEventPayload& Payload);
};
```


## Data Models

### Character Data Asset

```cpp
// Already exists in Configuration System, shown here for reference
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepCharacterData : public UDataAsset
{
    GENERATED_BODY()

public:
    // Display information
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
    FText Description;

    // Base stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", 
        meta = (ClampMin = "1.0", ClampMax = "10000.0"))
    float BaseHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", 
        meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float BaseMana = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", 
        meta = (ClampMin = "1.0", ClampMax = "1000.0"))
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", 
        meta = (ClampMin = "50.0", ClampMax = "1000.0"))
    float BaseMoveSpeed = 300.0f;

    // Starting equipment
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    TSoftObjectPtr<UDelveDeepWeaponData> StartingWeapon;

    // Starting abilities
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TArray<TSoftObjectPtr<UDelveDeepAbilityData>> StartingAbilities;

    // Visual
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<UPaperFlipbook> IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<UPaperFlipbook> WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<UPaperFlipbook> AttackAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<UPaperFlipbook> DeathAnimation;

    // Validation
    virtual void PostLoad() override;
    bool Validate(FValidationContext& Context) const;
};
```

### Event Payloads

```cpp
// Character death event payload
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepCharacterDeathPayload : public FDelveDeepEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Event")
    ADelveDeepCharacter* Character = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Event")
    AActor* Killer = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Event")
    FVector DeathLocation = FVector::ZeroVector;
};

// Character stat changed event payload
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepStatChangedPayload : public FDelveDeepEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Event")
    ADelveDeepCharacter* Character = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Event")
    FName StatName;

    UPROPERTY(BlueprintReadWrite, Category = "Event")
    float OldValue = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Event")
    float NewValue = 0.0f;
};

// Character ability used event payload
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepAbilityUsedPayload : public FDelveDeepEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Event")
    ADelveDeepCharacter* Character = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Event")
    const UDelveDeepAbilityData* Ability = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Event")
    float ResourceCost = 0.0f;
};
```

## Error Handling

### Validation Patterns

```cpp
bool ADelveDeepCharacter::ValidateCharacterData(FValidationContext& Context) const
{
    Context.SystemName = TEXT("Character");
    Context.OperationName = TEXT("ValidateCharacterData");

    if (!CharacterData)
    {
        Context.AddError(FString::Printf(
            TEXT("Character '%s' has no character data asset"), 
            *GetName()));
        return false;
    }

    if (!StatsComponent)
    {
        Context.AddError(FString::Printf(
            TEXT("Character '%s' missing stats component"), 
            *GetName()));
        return false;
    }

    if (CharacterData->BaseHealth <= 0.0f)
    {
        Context.AddError(FString::Printf(
            TEXT("Character '%s' has invalid base health: %.2f"), 
            *GetName(), CharacterData->BaseHealth));
        return false;
    }

    return true;
}

void ADelveDeepCharacter::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    FValidationContext Context;
    Context.SystemName = TEXT("Character");
    Context.OperationName = TEXT("TakeDamage");

    if (DamageAmount < 0.0f)
    {
        Context.AddError(FString::Printf(
            TEXT("Invalid damage amount: %.2f"), DamageAmount));
        UE_LOG(LogDelveDeep, Error, TEXT("%s"), *Context.GetReport());
        return;
    }

    if (!DamageSource)
    {
        Context.AddWarning(TEXT("Damage source is null"));
    }

    // Apply damage
    if (StatsComponent)
    {
        StatsComponent->ModifyHealth(-DamageAmount);
    }

    // Broadcast event
    BroadcastDamageEvent(DamageAmount, DamageSource);

    // Blueprint event
    OnDamaged(DamageAmount, DamageSource);
}
```

### Fallback Values

```cpp
void ADelveDeepCharacter::InitializeFromData()
{
    // Load character data
    if (UDelveDeepConfigurationManager* ConfigManager = 
        GetWorld()->GetGameInstance()->GetSubsystem<UDelveDeepConfigurationManager>())
    {
        CharacterData = ConfigManager->GetCharacterData(CharacterClassName);
    }

    // Fallback if data not found
    if (!CharacterData)
    {
        UE_LOG(LogDelveDeep, Error, 
            TEXT("Failed to load character data for '%s', using defaults"), 
            *CharacterClassName.ToString());

        // Use hardcoded fallback values
        if (StatsComponent)
        {
            StatsComponent->SetHealth(100.0f);
            StatsComponent->SetResource(100.0f);
        }
        return;
    }

    // Initialize from data
    if (StatsComponent)
    {
        StatsComponent->InitializeFromCharacterData(CharacterData);
    }

    if (AbilitiesComponent)
    {
        AbilitiesComponent->InitializeFromCharacterData(CharacterData);
    }

    if (EquipmentComponent)
    {
        EquipmentComponent->InitializeFromCharacterData(CharacterData);
    }
}
```


## Testing Strategy

### Unit Tests

```cpp
// Test character initialization
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterInitializationTest,
    "DelveDeep.Character.Initialization",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCharacterInitializationTest::RunTest(const FString& Parameters)
{
    // Create test game instance
    UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    ASSERT_NOT_NULL(GameInstance);

    // Get configuration manager
    UDelveDeepConfigurationManager* ConfigManager = 
        DelveDeepTestUtils::GetTestSubsystem<UDelveDeepConfigurationManager>(GameInstance);
    ASSERT_NOT_NULL(ConfigManager);

    // Create test character
    ADelveDeepWarrior* Warrior = DelveDeepTestUtils::CreateTestObject<ADelveDeepWarrior>();
    ASSERT_NOT_NULL(Warrior);

    // Verify components created
    EXPECT_NOT_NULL(Warrior->GetStatsComponent());
    EXPECT_NOT_NULL(Warrior->GetAbilitiesComponent());
    EXPECT_NOT_NULL(Warrior->GetEquipmentComponent());

    // Verify stats initialized
    EXPECT_GT(Warrior->GetMaxHealth(), 0.0f);
    EXPECT_EQ(Warrior->GetCurrentHealth(), Warrior->GetMaxHealth());

    return true;
}

// Test damage handling
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterDamageTest,
    "DelveDeep.Character.Damage",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCharacterDamageTest::RunTest(const FString& Parameters)
{
    // Create test character
    ADelveDeepWarrior* Warrior = DelveDeepTestUtils::CreateTestObject<ADelveDeepWarrior>();
    ASSERT_NOT_NULL(Warrior);

    // Set initial health
    float InitialHealth = 100.0f;
    Warrior->GetStatsComponent()->SetHealth(InitialHealth);

    // Apply damage
    float DamageAmount = 25.0f;
    Warrior->TakeDamage(DamageAmount, nullptr);

    // Verify health reduced
    EXPECT_EQ(Warrior->GetCurrentHealth(), InitialHealth - DamageAmount);

    // Apply lethal damage
    Warrior->TakeDamage(InitialHealth, nullptr);

    // Verify death
    EXPECT_TRUE(Warrior->IsDead());
    EXPECT_EQ(Warrior->GetCurrentHealth(), 0.0f);

    return true;
}

// Test resource management
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterResourceTest,
    "DelveDeep.Character.Resource",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCharacterResourceTest::RunTest(const FString& Parameters)
{
    // Test Warrior Rage generation
    ADelveDeepWarrior* Warrior = DelveDeepTestUtils::CreateTestObject<ADelveDeepWarrior>();
    ASSERT_NOT_NULL(Warrior);

    float InitialRage = Warrior->GetCurrentResource();
    Warrior->GenerateRage(25.0f);
    EXPECT_EQ(Warrior->GetCurrentResource(), InitialRage + 25.0f);

    // Test Mage Mana regeneration
    ADelveDeepMage* Mage = DelveDeepTestUtils::CreateTestObject<ADelveDeepMage>();
    ASSERT_NOT_NULL(Mage);

    Mage->GetStatsComponent()->SetResource(50.0f);
    float InitialMana = Mage->GetCurrentResource();
    Mage->RegenerateMana(1.0f);  // 1 second
    EXPECT_GT(Mage->GetCurrentResource(), InitialMana);

    return true;
}
```

### Integration Tests

```cpp
// Test character event integration
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterEventIntegrationTest,
    "DelveDeep.Integration.CharacterEvents",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCharacterEventIntegrationTest::RunTest(const FString& Parameters)
{
    FIntegrationTestFixture Fixture;
    Fixture.BeforeEach();

    // Create test character
    ADelveDeepWarrior* Warrior = DelveDeepTestUtils::CreateTestObject<ADelveDeepWarrior>();
    ASSERT_NOT_NULL(Warrior);

    // Register event listener
    bool bDamageEventReceived = false;
    float ReceivedDamage = 0.0f;

    Fixture.EventSubsystem->RegisterListener(
        FGameplayTag::RequestGameplayTag("DelveDeep.Character.Damaged"),
        FDelveDeepEventDelegate::CreateLambda(
            [&](const FDelveDeepEventPayload& Payload)
            {
                bDamageEventReceived = true;
                if (const FDelveDeepDamagePayload* DamagePayload = 
                    static_cast<const FDelveDeepDamagePayload*>(&Payload))
                {
                    ReceivedDamage = DamagePayload->DamageAmount;
                }
            }));

    // Apply damage
    float DamageAmount = 25.0f;
    Warrior->TakeDamage(DamageAmount, nullptr);

    // Verify event received
    EXPECT_TRUE(bDamageEventReceived);
    EXPECT_EQ(ReceivedDamage, DamageAmount);

    Fixture.AfterEach();
    return true;
}
```

## Performance Considerations

### Tick Optimization

```cpp
ADelveDeepCharacter::ADelveDeepCharacter()
{
    // Disable tick by default
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // Create components
    StatsComponent = CreateDefaultSubobject<UDelveDeepStatsComponent>(TEXT("StatsComponent"));
    AbilitiesComponent = CreateDefaultSubobject<UDelveDeepAbilitiesComponent>(TEXT("AbilitiesComponent"));
    EquipmentComponent = CreateDefaultSubobject<UDelveDeepEquipmentComponent>(TEXT("EquipmentComponent"));

    // Disable component tick
    StatsComponent->PrimaryComponentTick.bCanEverTick = false;
    AbilitiesComponent->PrimaryComponentTick.bCanEverTick = false;
    EquipmentComponent->PrimaryComponentTick.bCanEverTick = false;
}
```

### Timer-Based Updates

```cpp
void ADelveDeepRanger::StartEnergyRegeneration()
{
    // Use timer for energy regeneration instead of tick
    GetWorld()->GetTimerManager().SetTimer(
        EnergyRegenTimerHandle,
        [this]()
        {
            RegenerateEnergy(0.1f);  // Update every 0.1 seconds
        },
        0.1f,
        true);  // Loop
}

void ADelveDeepRanger::RegenerateEnergy(float DeltaTime)
{
    if (StatsComponent)
    {
        float RegenAmount = EnergyRegenRate * DeltaTime;
        StatsComponent->ModifyResource(RegenAmount);
    }
}
```

### Stat Caching

```cpp
void UDelveDeepStatsComponent::RecalculateStats()
{
    if (!bStatsDirty)
    {
        return;  // Use cached values
    }

    // Recalculate modified stats
    MaxHealth = ApplyModifiers("Health", BaseHealth);
    MaxResource = ApplyModifiers("Resource", BaseResource);
    MoveSpeed = ApplyModifiers("MoveSpeed", BaseMoveSpeed);

    // Update character movement speed
    if (ADelveDeepCharacter* Character = GetCharacterOwner())
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = MoveSpeed;
        }
    }

    bStatsDirty = false;
}

float UDelveDeepStatsComponent::GetModifiedStat(FName StatName) const
{
    // Recalculate if dirty
    const_cast<UDelveDeepStatsComponent*>(this)->RecalculateStats();

    // Return cached value
    if (StatName == "Health")
        return MaxHealth;
    else if (StatName == "Resource")
        return MaxResource;
    else if (StatName == "MoveSpeed")
        return MoveSpeed;

    return 0.0f;
}
```

### Performance Profiling

```cpp
DECLARE_CYCLE_STAT(TEXT("Character TakeDamage"), STAT_CharacterTakeDamage, STATGROUP_DelveDeep);
DECLARE_CYCLE_STAT(TEXT("Stats RecalculateStats"), STAT_StatsRecalculate, STATGROUP_DelveDeep);

void ADelveDeepCharacter::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    SCOPE_CYCLE_COUNTER(STAT_CharacterTakeDamage);

    // Damage logic
}

void UDelveDeepStatsComponent::RecalculateStats()
{
    SCOPE_CYCLE_COUNTER(STAT_StatsRecalculate);

    // Recalculation logic
}
```


## Blueprint Integration

### Blueprint Function Library

```cpp
UCLASS()
class DELVEDEEP_API UDelveDeepCharacterBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Character spawning
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character", 
        meta = (WorldContext = "WorldContextObject"))
    static ADelveDeepCharacter* SpawnCharacter(
        const UObject* WorldContextObject,
        TSubclassOf<ADelveDeepCharacter> CharacterClass,
        FVector Location,
        FRotator Rotation);

    // Character queries
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
    static bool IsCharacterAlive(ADelveDeepCharacter* Character);

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
    static float GetHealthPercentage(ADelveDeepCharacter* Character);

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
    static float GetResourcePercentage(ADelveDeepCharacter* Character);

    // Character actions
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
    static void ApplyDamageToCharacter(
        ADelveDeepCharacter* Character,
        float DamageAmount,
        AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
    static void HealCharacter(ADelveDeepCharacter* Character, float HealAmount);

    // Stat modifiers
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
    static void AddTemporaryStatBoost(
        ADelveDeepCharacter* Character,
        FName StatName,
        float Modifier,
        float Duration);
};
```

### Blueprint Events

```cpp
// In ADelveDeepCharacter
UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
void OnDeath();

UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
void OnDamaged(float DamageAmount, AActor* DamageSource);

UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
void OnHealed(float HealAmount);

UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
void OnStatChanged(FName StatName, float OldValue, float NewValue);

UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
void OnResourceChanged(float OldValue, float NewValue);

UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
void OnAbilityUsed(const UDelveDeepAbilityData* Ability);

UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character|Events")
void OnWeaponEquipped(const UDelveDeepWeaponData* Weapon);
```

## Console Commands

### Character Debug Commands

```cpp
// Spawn character
static FAutoConsoleCommand SpawnCharacterCmd(
    TEXT("DelveDeep.SpawnCharacter"),
    TEXT("Spawns a character at player location. Usage: DelveDeep.SpawnCharacter <ClassName>"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&UDelveDeepCharacterBlueprintLibrary::SpawnCharacterCommand));

// Modify character stats
static FAutoConsoleCommand ModifyHealthCmd(
    TEXT("DelveDeep.ModifyHealth"),
    TEXT("Modifies character health. Usage: DelveDeep.ModifyHealth <Amount>"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&UDelveDeepCharacterBlueprintLibrary::ModifyHealthCommand));

static FAutoConsoleCommand ModifyResourceCmd(
    TEXT("DelveDeep.ModifyResource"),
    TEXT("Modifies character resource. Usage: DelveDeep.ModifyResource <Amount>"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&UDelveDeepCharacterBlueprintLibrary::ModifyResourceCommand));

// Deal damage
static FAutoConsoleCommand DealDamageCmd(
    TEXT("DelveDeep.DealDamage"),
    TEXT("Deals damage to character. Usage: DelveDeep.DealDamage <Amount>"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&UDelveDeepCharacterBlueprintLibrary::DealDamageCommand));

// Display character state
static FAutoConsoleCommand ShowCharacterStateCmd(
    TEXT("DelveDeep.ShowCharacterState"),
    TEXT("Displays character state (stats, abilities, equipment)"),
    FConsoleCommandDelegate::CreateStatic(&UDelveDeepCharacterBlueprintLibrary::ShowCharacterStateCommand));

// Kill character
static FAutoConsoleCommand KillCharacterCmd(
    TEXT("DelveDeep.KillCharacter"),
    TEXT("Kills the player character"),
    FConsoleCommandDelegate::CreateStatic(&UDelveDeepCharacterBlueprintLibrary::KillCharacterCommand));

// Respawn character
static FAutoConsoleCommand RespawnCharacterCmd(
    TEXT("DelveDeep.RespawnCharacter"),
    TEXT("Respawns the player character"),
    FConsoleCommandDelegate::CreateStatic(&UDelveDeepCharacterBlueprintLibrary::RespawnCharacterCommand));
```

## Future Extensibility

### Ability Component (Placeholder)

```cpp
UCLASS(BlueprintType, Category = "DelveDeep|Character|Abilities")
class DELVEDEEP_API UDelveDeepAbilitiesComponent : public UDelveDeepCharacterComponent
{
    GENERATED_BODY()

public:
    UDelveDeepAbilitiesComponent();

    // Ability management
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Abilities")
    void AddAbility(const UDelveDeepAbilityData* Ability);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Abilities")
    void RemoveAbility(const UDelveDeepAbilityData* Ability);

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Abilities")
    int32 GetAbilityCount() const { return Abilities.Num(); }

    // Ability usage
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Abilities")
    bool UseAbility(int32 AbilityIndex);

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Abilities")
    bool CanUseAbility(int32 AbilityIndex) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Abilities")
    TArray<const UDelveDeepAbilityData*> Abilities;

    UPROPERTY()
    TMap<const UDelveDeepAbilityData*, float> AbilityCooldowns;

    virtual void InitializeFromCharacterData(const UDelveDeepCharacterData* CharacterData) override;

    // Cooldown management
    void StartAbilityCooldown(const UDelveDeepAbilityData* Ability);
    bool IsAbilityOnCooldown(const UDelveDeepAbilityData* Ability) const;
};
```

### Equipment Component (Placeholder)

```cpp
UCLASS(BlueprintType, Category = "DelveDeep|Character|Equipment")
class DELVEDEEP_API UDelveDeepEquipmentComponent : public UDelveDeepCharacterComponent
{
    GENERATED_BODY()

public:
    UDelveDeepEquipmentComponent();

    // Weapon management
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Equipment")
    void EquipWeapon(const UDelveDeepWeaponData* Weapon);

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Equipment")
    const UDelveDeepWeaponData* GetCurrentWeapon() const { return CurrentWeapon; }

    // Stat modifiers from equipment
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Equipment")
    float GetEquipmentStatModifier(FName StatName) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Equipment")
    const UDelveDeepWeaponData* CurrentWeapon;

    virtual void InitializeFromCharacterData(const UDelveDeepCharacterData* CharacterData) override;

    // Apply/remove equipment modifiers
    void ApplyWeaponModifiers(const UDelveDeepWeaponData* Weapon);
    void RemoveWeaponModifiers(const UDelveDeepWeaponData* Weapon);
};
```

### Animation State Machine Integration

```cpp
// Future integration with Animation State Machine spec
void ADelveDeepCharacter::UpdateAnimationState()
{
    if (!GetSprite())
        return;

    // Determine animation state
    if (bIsDead)
    {
        PlayDeathAnimation();
    }
    else if (GetVelocity().SizeSquared() > 0.0f)
    {
        PlayWalkAnimation();
    }
    else
    {
        PlayIdleAnimation();
    }
}

void ADelveDeepCharacter::PlayIdleAnimation()
{
    if (CharacterData && !CharacterData->IdleAnimation.IsNull())
    {
        if (UPaperFlipbook* Flipbook = CharacterData->IdleAnimation.LoadSynchronous())
        {
            GetSprite()->SetFlipbook(Flipbook);
        }
    }
}
```

## Dependencies

- **Unreal Engine 5.6**: APaperCharacter, UPaperFlipbookComponent, UCharacterMovementComponent
- **DelveDeep Configuration System**: UDelveDeepConfigurationManager, UDelveDeepCharacterData
- **DelveDeep Event System**: UDelveDeepEventSubsystem, FDelveDeepEventPayload
- **DelveDeep Validation System**: FValidationContext
- **DelveDeep Telemetry System**: Performance profiling with SCOPE_CYCLE_COUNTER
- **Paper2D**: Sprite rendering and animation

## Integration Points

### Configuration System Integration

```cpp
void ADelveDeepCharacter::InitializeFromData()
{
    // Get configuration manager
    UDelveDeepConfigurationManager* ConfigManager = nullptr;
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        ConfigManager = GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
    }

    if (!ConfigManager)
    {
        UE_LOG(LogDelveDeep, Error, 
            TEXT("Failed to get configuration manager for character '%s'"), 
            *GetName());
        return;
    }

    // Load character data
    CharacterData = ConfigManager->GetCharacterData(CharacterClassName);

    // Validate and initialize
    FValidationContext Context;
    if (ValidateCharacterData(Context))
    {
        InitializeComponents();
    }
    else
    {
        UE_LOG(LogDelveDeep, Error, 
            TEXT("Character data validation failed: %s"), 
            *Context.GetReport());
    }
}
```

### Event System Integration

```cpp
void ADelveDeepCharacter::BroadcastDamageEvent(float DamageAmount, AActor* DamageSource)
{
    // Get event subsystem
    UDelveDeepEventSubsystem* EventSubsystem = nullptr;
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        EventSubsystem = GameInstance->GetSubsystem<UDelveDeepEventSubsystem>();
    }

    if (!EventSubsystem)
    {
        return;
    }

    // Create damage payload
    FDelveDeepDamagePayload Payload;
    Payload.EventTag = FGameplayTag::RequestGameplayTag("DelveDeep.Character.Damaged");
    Payload.Instigator = this;
    Payload.Target = this;
    Payload.DamageAmount = DamageAmount;
    Payload.DamageSource = DamageSource;

    // Broadcast event
    EventSubsystem->BroadcastEvent(Payload);
}
```

### Telemetry System Integration

```cpp
void ADelveDeepCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Register with telemetry
    if (UDelveDeepTelemetrySubsystem* Telemetry = 
        GetWorld()->GetGameInstance()->GetSubsystem<UDelveDeepTelemetrySubsystem>())
    {
        Telemetry->IncrementEntityCount("Characters");
    }

    InitializeFromData();
}

void ADelveDeepCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unregister from telemetry
    if (UDelveDeepTelemetrySubsystem* Telemetry = 
        GetWorld()->GetGameInstance()->GetSubsystem<UDelveDeepTelemetrySubsystem>())
    {
        Telemetry->DecrementEntityCount("Characters");
    }

    Super::EndPlay(EndPlayReason);
}
```
