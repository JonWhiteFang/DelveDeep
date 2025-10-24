# Design Document

## Overview

The Centralized Event System provides a robust, performant gameplay event bus using Unreal Engine's GameplayTag system. It enables loose coupling between game systems by allowing event-driven communication without direct dependencies. The system is implemented as a UGameInstanceSubsystem to persist across level transitions and provide global access.

The design follows the observer pattern with enhancements for game development: hierarchical event filtering via GameplayTags, priority-based listener execution, deferred event processing, spatial filtering, and comprehensive debugging tools.

## Architecture

### System Components

```
UDelveDeepEventSubsystem (UGameInstanceSubsystem)
├── Event Registry (TMap<FGameplayTag, FEventListenerList>)
├── Event Queue (TArray<FDeferredEvent>) - for deferred processing
├── Event History (TCircularBuffer<FEventRecord>) - last 100 events
├── Performance Metrics (FEventSystemMetrics)
└── Debug Visualization (FEventDebugInfo)

FEventListenerList
├── High Priority Listeners (TArray<FEventListener>)
├── Normal Priority Listeners (TArray<FEventListener>)
└── Low Priority Listeners (TArray<FEventListener>)

FEventListener
├── Callback (TFunction or Delegate)
├── Owner (Weak pointer to registering object)
├── Filter (Optional spatial/actor filter)
└── Metadata (Registration time, invocation count)

FEventPayload (Base struct)
├── Timestamp (FDateTime)
├── EventTag (FGameplayTag)
├── Instigator (TWeakObjectPtr<AActor>)
└── [Derived payload types with specific data]
```

### Class Hierarchy

```cpp
// Core subsystem
UDelveDeepEventSubsystem : public UGameInstanceSubsystem

// Event payload base
USTRUCT(BlueprintType)
struct FDelveDeepEventPayload
{
    FGameplayTag EventTag;
    FDateTime Timestamp;
    TWeakObjectPtr<AActor> Instigator;
};

// Specific event payloads
struct FDelveDeepDamageEventPayload : public FDelveDeepEventPayload
struct FDelveDeepHealthChangeEventPayload : public FDelveDeepEventPayload
struct FDelveDeepKillEventPayload : public FDelveDeepEventPayload
struct FDelveDeepAttackEventPayload : public FDelveDeepEventPayload

// Listener management
USTRUCT()
struct FDelveDeepEventListener
{
    TFunction<void(const FDelveDeepEventPayload&)> Callback;
    TWeakObjectPtr<UObject> Owner;
    EDelveDeepEventPriority Priority;
    FDelveDeepEventFilter Filter;
};

// Event filtering
USTRUCT(BlueprintType)
struct FDelveDeepEventFilter
{
    TWeakObjectPtr<AActor> SpecificActor;
    FVector SpatialLocation;
    float SpatialRadius;
    bool bUseSpatialFilter;
    bool bUseActorFilter;
};
```

## Components and Interfaces

### UDelveDeepEventSubsystem

Primary interface for event broadcasting and listener registration.

**Public API:**

```cpp
// Listener registration
FDelegateHandle RegisterListener(
    FGameplayTag EventTag,
    TFunction<void(const FDelveDeepEventPayload&)> Callback,
    UObject* Owner,
    EDelveDeepEventPriority Priority = EDelveDeepEventPriority::Normal,
    const FDelveDeepEventFilter& Filter = FDelveDeepEventFilter()
);

void UnregisterListener(FDelegateHandle Handle);
void UnregisterAllListeners(UObject* Owner);

// Event broadcasting
void BroadcastEvent(const FDelveDeepEventPayload& Payload);
void BroadcastEventDeferred(const FDelveDeepEventPayload& Payload);

// Deferred processing control
void EnableDeferredMode();
void DisableDeferredMode();
void ProcessDeferredEvents();

// Query and debugging
int32 GetListenerCount(FGameplayTag EventTag) const;
TArray<FGameplayTag> GetAllRegisteredEventTags() const;
const FEventSystemMetrics& GetPerformanceMetrics() const;
TArray<FEventRecord> GetEventHistory() const;
```

**Private Implementation:**

```cpp
// Event registry: Tag -> Listener lists
TMap<FGameplayTag, FEventListenerList> EventRegistry;

// Deferred event queue
TArray<FDeferredEvent> DeferredEventQueue;
bool bDeferredMode = false;
static constexpr int32 MaxDeferredEvents = 1000;

// Event history for debugging
TCircularBuffer<FEventRecord> EventHistory;
static constexpr int32 MaxHistorySize = 100;

// Performance tracking
FEventSystemMetrics PerformanceMetrics;
mutable FCriticalSection MetricsMutex;

// Handle generation
FDelegateHandle GenerateUniqueHandle();
TMap<FDelegateHandle, FListenerLocation> HandleToListenerMap;
```

### Event Payload Types

**Base Payload:**

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Event")
    FGameplayTag EventTag;

    UPROPERTY(BlueprintReadOnly, Category = "Event")
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Event")
    TWeakObjectPtr<AActor> Instigator;

    FDelveDeepEventPayload()
        : Timestamp(FDateTime::Now())
    {}

    virtual ~FDelveDeepEventPayload() = default;
    virtual bool Validate(FValidationContext& Context) const;
};
```

**Damage Event Payload:**

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepDamageEventPayload : public FDelveDeepEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    TWeakObjectPtr<AActor> Attacker;

    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    TWeakObjectPtr<AActor> Victim;

    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    float DamageAmount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Damage")
    FGameplayTag DamageType;

    virtual bool Validate(FValidationContext& Context) const override;
};
```

**Health Change Event Payload:**

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepHealthChangeEventPayload : public FDelveDeepEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    TWeakObjectPtr<AActor> Character;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float PreviousHealth = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float NewHealth = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float MaxHealth = 0.0f;

    virtual bool Validate(FValidationContext& Context) const override;
};
```

**Kill Event Payload:**

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepKillEventPayload : public FDelveDeepEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Kill")
    TWeakObjectPtr<AActor> Killer;

    UPROPERTY(BlueprintReadOnly, Category = "Kill")
    TWeakObjectPtr<AActor> Victim;

    UPROPERTY(BlueprintReadOnly, Category = "Kill")
    int32 ExperienceAwarded = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Kill")
    FGameplayTag VictimType;

    virtual bool Validate(FValidationContext& Context) const override;
};
```

**Attack Event Payload:**

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepAttackEventPayload : public FDelveDeepEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    TWeakObjectPtr<AActor> Attacker;

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    FVector AttackLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    FGameplayTag AttackType;

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    float AttackRadius = 0.0f;

    virtual bool Validate(FValidationContext& Context) const override;
};
```

### Event Priority System

```cpp
UENUM(BlueprintType)
enum class EDelveDeepEventPriority : uint8
{
    High    UMETA(DisplayName = "High Priority"),
    Normal  UMETA(DisplayName = "Normal Priority"),
    Low     UMETA(DisplayName = "Low Priority")
};
```

Listeners are invoked in priority order: High → Normal → Low. Within the same priority, listeners are invoked in registration order.

### Event Filtering

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepEventFilter
{
    GENERATED_BODY()

    // Actor-specific filtering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
    TWeakObjectPtr<AActor> SpecificActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
    bool bUseActorFilter = false;

    // Spatial filtering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
    FVector SpatialLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
    float SpatialRadius = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
    bool bUseSpatialFilter = false;

    bool PassesFilter(const FDelveDeepEventPayload& Payload) const;
};
```

## Data Models

### Event Registry Structure

```cpp
// Primary storage: GameplayTag -> Listener lists
TMap<FGameplayTag, FEventListenerList> EventRegistry;

struct FEventListenerList
{
    TArray<FDelveDeepEventListener> HighPriorityListeners;
    TArray<FDelveDeepEventListener> NormalPriorityListeners;
    TArray<FDelveDeepEventListener> LowPriorityListeners;

    int32 GetTotalListenerCount() const
    {
        return HighPriorityListeners.Num() + 
               NormalPriorityListeners.Num() + 
               LowPriorityListeners.Num();
    }
};
```

### Handle Management

```cpp
// Handle to listener location mapping
struct FListenerLocation
{
    FGameplayTag EventTag;
    EDelveDeepEventPriority Priority;
    int32 IndexInArray;
};

TMap<FDelegateHandle, FListenerLocation> HandleToListenerMap;
```

### Performance Metrics

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FEventSystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalEventsBroadcast = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalListenerInvocations = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    double AverageTimePerBroadcast = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    double AverageTimePerListener = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    double TotalEventSystemOverhead = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DeferredEventsProcessed = 0;

    void Reset();
    void RecordBroadcast(double Duration, int32 ListenerCount);
};
```

### Event History

```cpp
struct FEventRecord
{
    FGameplayTag EventTag;
    FDateTime Timestamp;
    int32 ListenerCount;
    double ProcessingTime;
    TSharedPtr<FDelveDeepEventPayload> Payload;
};

// Circular buffer for last 100 events
TCircularBuffer<FEventRecord> EventHistory;
```

## Error Handling

### Validation Strategy

All event payloads implement validation using FValidationContext:

```cpp
bool FDelveDeepDamageEventPayload::Validate(FValidationContext& Context) const
{
    Context.SystemName = TEXT("EventSystem");
    Context.OperationName = TEXT("ValidateDamageEvent");

    bool bIsValid = true;

    if (!Attacker.IsValid())
    {
        Context.AddError(TEXT("Attacker reference is invalid"));
        bIsValid = false;
    }

    if (!Victim.IsValid())
    {
        Context.AddError(TEXT("Victim reference is invalid"));
        bIsValid = false;
    }

    if (DamageAmount < 0.0f)
    {
        Context.AddError(FString::Printf(
            TEXT("Damage amount is negative: %.2f"), DamageAmount));
        bIsValid = false;
    }

    if (!DamageType.IsValid())
    {
        Context.AddWarning(TEXT("Damage type tag is not set"));
    }

    return bIsValid;
}
```

### Error Recovery

**Invalid Payload:**
- Log error with validation context
- Skip event broadcast
- Return false from BroadcastEvent

**Listener Callback Exception:**
- Catch exception in try-catch block
- Log error with listener details
- Continue processing remaining listeners
- Track failed listener in metrics

**Queue Overflow:**
- Log warning when queue reaches 80% capacity
- Discard oldest events when queue exceeds limit
- Track overflow count in metrics

**Stale Listener:**
- Check weak pointer validity before invocation
- Automatically remove listeners with invalid owners
- Log cleanup operations in verbose mode

## Testing Strategy

### Unit Tests

**Event Broadcasting:**
- Test event delivery to single listener
- Test event delivery to multiple listeners
- Test priority-based execution order
- Test hierarchical tag matching
- Test event with no listeners

**Listener Management:**
- Test listener registration
- Test listener unregistration by handle
- Test bulk unregistration by owner
- Test automatic cleanup of stale listeners
- Test handle uniqueness

**Filtering:**
- Test actor-specific filtering
- Test spatial filtering
- Test combined filters
- Test filter edge cases

**Deferred Processing:**
- Test deferred mode enable/disable
- Test queue management
- Test queue overflow handling
- Test deferred event ordering

**Validation:**
- Test payload validation
- Test invalid payload rejection
- Test validation error reporting

### Integration Tests

**Combat Integration:**
- Broadcast damage event from combat system
- Verify UI receives health update
- Verify progression system receives kill event
- Verify AI receives attack notification

**Cross-System Communication:**
- Test event flow between 3+ systems
- Verify event ordering across systems
- Test performance with realistic event load

### Performance Tests

**Throughput:**
- Measure events per second capacity
- Test with 1, 10, 50, 100 listeners
- Verify <1ms broadcast time target

**Memory:**
- Monitor memory usage with 1000+ listeners
- Test event history memory footprint
- Verify no memory leaks

**Latency:**
- Measure listener invocation latency
- Test deferred processing latency
- Verify <0.1ms overhead target

### Console Commands

```cpp
// Listener management
DelveDeep.Events.ListListeners [EventTag]
DelveDeep.Events.ListAllListeners
DelveDeep.Events.ClearAllListeners

// Event testing
DelveDeep.Events.BroadcastTestEvent <EventTag>
DelveDeep.Events.EnableEventLogging
DelveDeep.Events.DisableEventLogging

// Performance
DelveDeep.Events.ShowMetrics
DelveDeep.Events.ResetMetrics
DelveDeep.Events.ShowEventHistory

// Debugging
DelveDeep.Events.EnableVisualization
DelveDeep.Events.DisableVisualization
DelveDeep.Events.DumpEventRegistry
DelveDeep.Events.ValidateAllPayloads
```

## Performance Considerations

### Optimization Strategies

**Fast Lookup:**
- Use TMap for O(1) event tag lookup
- Cache listener counts per tag
- Pre-allocate listener arrays

**Minimal Allocations:**
- Reuse payload objects where possible
- Use object pooling for frequent events
- Reserve array capacity on registration

**Efficient Iteration:**
- Store listeners in contiguous arrays
- Iterate by priority level
- Early exit on empty listener lists

**Lazy Cleanup:**
- Batch stale listener removal
- Clean up during low-activity frames
- Use generation counters for validity

**Conditional Validation:**
- Full validation in development builds
- Minimal validation in shipping builds
- Use preprocessor guards (#if !UE_BUILD_SHIPPING)

### Performance Targets

- Event broadcast: <1ms for 50 listeners
- Listener invocation: <0.1ms per listener
- System overhead: <0.1ms per event
- Deferred processing: <10ms for 1000 events
- Memory per listener: <200 bytes

### Profiling Integration

```cpp
DECLARE_STATS_GROUP(TEXT("DelveDeep.Events"), STATGROUP_DelveDeepEvents, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Broadcast Event"), STAT_BroadcastEvent, STATGROUP_DelveDeepEvents);
DECLARE_CYCLE_STAT(TEXT("Invoke Listeners"), STAT_InvokeListeners, STATGROUP_DelveDeepEvents);
DECLARE_CYCLE_STAT(TEXT("Process Deferred"), STAT_ProcessDeferred, STATGROUP_DelveDeepEvents);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Listeners"), STAT_ActiveListeners, STATGROUP_DelveDeepEvents);
DECLARE_DWORD_COUNTER_STAT(TEXT("Events Per Frame"), STAT_EventsPerFrame, STATGROUP_DelveDeepEvents);
```

## GameplayTag Hierarchy

**Implementation Status:** ✅ Implemented in `Config/DefaultGameplayTags.ini`

The complete GameplayTag hierarchy has been defined in the project configuration file. All tags are automatically loaded by Unreal Engine's GameplayTags system on startup.

### Tag Structure

```
DelveDeep.Event
├── DelveDeep.Event.Combat
│   ├── DelveDeep.Event.Combat.Damage
│   │   ├── DelveDeep.Event.Combat.Damage.Dealt
│   │   ├── DelveDeep.Event.Combat.Damage.Received
│   │   └── DelveDeep.Event.Combat.Damage.Blocked
│   ├── DelveDeep.Event.Combat.Attack
│   │   ├── DelveDeep.Event.Combat.Attack.Melee
│   │   ├── DelveDeep.Event.Combat.Attack.Ranged
│   │   └── DelveDeep.Event.Combat.Attack.Ability
│   ├── DelveDeep.Event.Combat.Kill
│   │   ├── DelveDeep.Event.Combat.Kill.Player
│   │   └── DelveDeep.Event.Combat.Kill.Enemy
│   └── DelveDeep.Event.Combat.Death
│       ├── DelveDeep.Event.Combat.Death.Player
│       └── DelveDeep.Event.Combat.Death.Enemy
├── DelveDeep.Event.Character
│   ├── DelveDeep.Event.Character.Health
│   │   ├── DelveDeep.Event.Character.Health.Changed
│   │   ├── DelveDeep.Event.Character.Health.Depleted
│   │   └── DelveDeep.Event.Character.Health.Restored
│   ├── DelveDeep.Event.Character.Resource
│   │   ├── DelveDeep.Event.Character.Resource.Mana
│   │   ├── DelveDeep.Event.Character.Resource.Energy
│   │   └── DelveDeep.Event.Character.Resource.Souls
│   └── DelveDeep.Event.Character.Status
│       ├── DelveDeep.Event.Character.Status.Stunned
│       ├── DelveDeep.Event.Character.Status.Slowed
│       └── DelveDeep.Event.Character.Status.Buffed
├── DelveDeep.Event.Progression
│   ├── DelveDeep.Event.Progression.Experience
│   │   ├── DelveDeep.Event.Progression.Experience.Gained
│   │   └── DelveDeep.Event.Progression.Experience.LevelUp
│   ├── DelveDeep.Event.Progression.Upgrade
│   │   ├── DelveDeep.Event.Progression.Upgrade.Purchased
│   │   └── DelveDeep.Event.Progression.Upgrade.Unlocked
│   └── DelveDeep.Event.Progression.Achievement
│       └── DelveDeep.Event.Progression.Achievement.Unlocked
├── DelveDeep.Event.World
│   ├── DelveDeep.Event.World.Depth
│   │   ├── DelveDeep.Event.World.Depth.Changed
│   │   └── DelveDeep.Event.World.Depth.Milestone
│   ├── DelveDeep.Event.World.Room
│   │   ├── DelveDeep.Event.World.Room.Entered
│   │   ├── DelveDeep.Event.World.Room.Cleared
│   │   └── DelveDeep.Event.World.Room.Exited
│   └── DelveDeep.Event.World.Item
│       ├── DelveDeep.Event.World.Item.Collected
│       └── DelveDeep.Event.World.Item.Dropped
└── DelveDeep.Event.System
    ├── DelveDeep.Event.System.Save
    │   ├── DelveDeep.Event.System.Save.Started
    │   └── DelveDeep.Event.System.Save.Completed
    ├── DelveDeep.Event.System.Load
    │   ├── DelveDeep.Event.System.Load.Started
    │   └── DelveDeep.Event.System.Load.Completed
    └── DelveDeep.Event.System.Pause
        ├── DelveDeep.Event.System.Pause.Paused
        └── DelveDeep.Event.System.Pause.Resumed
```

### Tag Matching Rules

- Exact match: Listener registered for "DelveDeep.Event.Combat.Damage.Dealt" receives only that specific event
- Parent match: Listener registered for "DelveDeep.Event.Combat" receives all combat-related events
- Wildcard: Listener registered for "DelveDeep.Event" receives all events (global listener)

## Blueprint Integration

### Blueprint Function Library

```cpp
UCLASS()
class DELVEDEEP_API UDelveDeepEventBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Events", 
        meta = (WorldContext = "WorldContextObject"))
    static UDelveDeepEventSubsystem* GetEventSubsystem(
        const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
    static void BroadcastDamageEvent(
        UObject* WorldContextObject,
        AActor* Attacker,
        AActor* Victim,
        float DamageAmount,
        FGameplayTag DamageType);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
    static void BroadcastHealthChangeEvent(
        UObject* WorldContextObject,
        AActor* Character,
        float PreviousHealth,
        float NewHealth,
        float MaxHealth);
};
```

### Blueprint-Callable Events

While C++ uses TFunction callbacks, Blueprint integration uses dynamic delegates:

```cpp
DECLARE_DYNAMIC_DELEGATE_OneParam(FDelveDeepEventDelegate, 
    const FDelveDeepEventPayload&, Payload);

UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
FDelegateHandle RegisterListenerBlueprint(
    FGameplayTag EventTag,
    FDelveDeepEventDelegate Callback,
    UObject* Owner,
    EDelveDeepEventPriority Priority = EDelveDeepEventPriority::Normal);
```

## Future Extensibility

### Network Replication Support

Event payloads include metadata for future multiplayer:

```cpp
struct FDelveDeepEventPayload
{
    // ... existing fields ...

    UPROPERTY()
    bool bNetworkRelevant = false;

    UPROPERTY()
    bool bReliable = true;

    virtual bool ShouldReplicate() const { return bNetworkRelevant; }
    virtual void Serialize(FArchive& Ar) { /* Implement when needed */ }
};
```

### Event Recording/Replay

Event history can be extended for replay functionality:

```cpp
class FEventRecorder
{
    TArray<FEventRecord> RecordedEvents;
    bool bRecording = false;

    void StartRecording();
    void StopRecording();
    void SaveToFile(const FString& Filename);
    void LoadFromFile(const FString& Filename);
    void Replay(float TimeScale = 1.0f);
};
```

### Custom Event Types

Systems can define custom event payloads by deriving from FDelveDeepEventPayload:

```cpp
USTRUCT(BlueprintType)
struct FMyCustomEventPayload : public FDelveDeepEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Custom")
    FString CustomData;

    virtual bool Validate(FValidationContext& Context) const override
    {
        // Custom validation logic
        return !CustomData.IsEmpty();
    }
};
```

## Dependencies

- **Unreal Engine GameplayTags Plugin**: Required for hierarchical event tagging
- **FValidationContext**: From Enhanced Validation System (Spec #2)
- **UGameInstanceSubsystem**: Core Unreal Engine subsystem framework
- **No external dependencies**: Self-contained system

## Migration Path

For existing systems with direct coupling:

1. Identify direct system-to-system calls
2. Define appropriate GameplayTags for events
3. Create event payload structures
4. Replace direct calls with event broadcasts
5. Register listeners in dependent systems
6. Test event flow and timing
7. Remove direct dependencies

Example migration:

```cpp
// Before: Direct coupling
void UCombatComponent::DealDamage(AActor* Target, float Damage)
{
    // Apply damage
    Target->TakeDamage(Damage);
    
    // Direct call to UI
    UMyHUDWidget* HUD = GetHUD();
    HUD->UpdateHealthBar(Target);
    
    // Direct call to progression
    UProgressionSystem* Progression = GetProgression();
    Progression->AwardExperience(10);
}

// After: Event-driven
void UCombatComponent::DealDamage(AActor* Target, float Damage)
{
    // Apply damage
    Target->TakeDamage(Damage);
    
    // Broadcast event
    FDelveDeepDamageEventPayload Payload;
    Payload.Attacker = GetOwner();
    Payload.Victim = Target;
    Payload.DamageAmount = Damage;
    Payload.EventTag = FGameplayTag::RequestGameplayTag(
        "DelveDeep.Event.Combat.Damage.Dealt");
    
    GetEventSubsystem()->BroadcastEvent(Payload);
}

// UI listens for damage events
void UMyHUDWidget::Initialize()
{
    GetEventSubsystem()->RegisterListener(
        FGameplayTag::RequestGameplayTag("DelveDeep.Event.Combat.Damage"),
        [this](const FDelveDeepEventPayload& Payload)
        {
            const auto& DamagePayload = static_cast<const FDelveDeepDamageEventPayload&>(Payload);
            UpdateHealthBar(DamagePayload.Victim.Get());
        },
        this,
        EDelveDeepEventPriority::High
    );
}
```
