# Centralized Event System

## Overview

The Centralized Event System provides a robust, performant gameplay event bus using Unreal Engine's GameplayTag system. It enables loose coupling between game systems by allowing event-driven communication without direct dependencies.

**Status**: ✅ Implemented (October 24, 2025)

**Performance Targets**:
- Event broadcast: <1ms for 50 listeners ✓
- Listener invocation: <0.1ms per listener ✓
- System overhead: <0.1ms per event ✓
- Deferred processing: <10ms for 1000 events ✓

## Key Features

- **Hierarchical Event Filtering**: GameplayTag-based event matching with parent-child relationships
- **Priority-Based Execution**: High, Normal, and Low priority listeners
- **Deferred Event Processing**: Queue events for safe processing during critical operations
- **Spatial and Actor Filtering**: Filter events by location or specific actors
- **Comprehensive Validation**: FValidationContext integration with build-specific behavior
- **Performance Monitoring**: Built-in metrics tracking and profiling support
- **Blueprint Integration**: Full Blueprint support for event broadcasting and listening
- **Network Preparation**: Metadata for future multiplayer support

## Architecture

### Core Components

```
UDelveDeepEventSubsystem (UGameInstanceSubsystem)
├── Event Registry (TMap<FGameplayTag, FEventListenerList>)
├── Event Queue (TArray<FDeferredEvent>)
├── Event History (Circular buffer, last 100 events)
├── Performance Metrics (FEventSystemMetrics)
└── Debug Visualization (Console commands)
```

### Event Payload Types

All event payloads derive from `FDelveDeepEventPayload`:

- **FDelveDeepDamageEventPayload**: Damage events with attacker, victim, amount, type
- **FDelveDeepHealthChangeEventPayload**: Health changes with previous/new values
- **FDelveDeepKillEventPayload**: Kill events with experience awarded
- **FDelveDeepAttackEventPayload**: Attack events with location and radius

## Usage Examples

### Broadcasting Events (C++)

```cpp
// Get the event subsystem
UDelveDeepEventSubsystem* EventSubsystem = 
    GetWorld()->GetGameInstance()->GetSubsystem<UDelveDeepEventSubsystem>();

// Create and broadcast a damage event
FDelveDeepDamageEventPayload DamagePayload;
DamagePayload.EventTag = FGameplayTag::RequestGameplayTag("DelveDeep.Event.Combat.Damage.Dealt");
DamagePayload.Attacker = AttackerActor;
DamagePayload.Victim = VictimActor;
DamagePayload.DamageAmount = 50.0f;
DamagePayload.DamageType = FGameplayTag::RequestGameplayTag("DelveDeep.DamageType.Physical");

EventSubsystem->BroadcastEvent(DamagePayload);
```

### Listening for Events (C++)

```cpp
// Register a listener for damage events
FDelegateHandle ListenerHandle = EventSubsystem->RegisterListener(
    FGameplayTag::RequestGameplayTag("DelveDeep.Event.Combat.Damage"),
    [this](const FDelveDeepEventPayload& Payload)
    {
        // Cast to specific payload type
        const FDelveDeepDamageEventPayload& DamagePayload = 
            static_cast<const FDelveDeepDamageEventPayload&>(Payload);
        
        // Handle the event
        UpdateHealthBar(DamagePayload.Victim.Get(), DamagePayload.DamageAmount);
    },
    this, // Owner for automatic cleanup
    EDelveDeepEventPriority::High // Execute before normal priority listeners
);

// Unregister when done
EventSubsystem->UnregisterListener(ListenerHandle);

// Or unregister all listeners for this object
EventSubsystem->UnregisterAllListeners(this);
```

### Event Filtering

```cpp
// Filter by specific actor
FDelveDeepEventFilter ActorFilter;
ActorFilter.bUseActorFilter = true;
ActorFilter.SpecificActor = PlayerCharacter;

FDelegateHandle Handle = EventSubsystem->RegisterListener(
    FGameplayTag::RequestGameplayTag("DelveDeep.Event.Character.Health.Changed"),
    [this](const FDelveDeepEventPayload& Payload) { /* Handle event */ },
    this,
    EDelveDeepEventPriority::Normal,
    ActorFilter // Only receive events for PlayerCharacter
);

// Filter by spatial location
FDelveDeepEventFilter SpatialFilter;
SpatialFilter.bUseSpatialFilter = true;
SpatialFilter.SpatialLocation = GetActorLocation();
SpatialFilter.SpatialRadius = 1000.0f; // 10 meter radius

FDelegateHandle SpatialHandle = EventSubsystem->RegisterListener(
    FGameplayTag::RequestGameplayTag("DelveDeep.Event.Combat.Attack"),
    [this](const FDelveDeepEventPayload& Payload) { /* React to nearby attacks */ },
    this,
    EDelveDeepEventPriority::Normal,
    SpatialFilter
);
```

### Deferred Event Processing

```cpp
// Enable deferred mode (e.g., during save operation)
EventSubsystem->EnableDeferredMode();

// Events are now queued instead of processed immediately
EventSubsystem->BroadcastEvent(SomePayload); // Queued

// Perform critical operation
SaveGameData();

// Process all queued events
EventSubsystem->ProcessDeferredEvents();

// Disable deferred mode
EventSubsystem->DisableDeferredMode();
```

### Blueprint Integration

```cpp
// Blueprint helper functions available via UDelveDeepEventBlueprintLibrary

// Get event subsystem
UDelveDeepEventSubsystem* EventSubsystem = 
    UDelveDeepEventBlueprintLibrary::GetEventSubsystem(WorldContextObject);

// Broadcast damage event from Blueprint
UDelveDeepEventBlueprintLibrary::BroadcastDamageEvent(
    WorldContextObject,
    AttackerActor,
    VictimActor,
    50.0f,
    DamageTypeTag
);

// Broadcast health change event from Blueprint
UDelveDeepEventBlueprintLibrary::BroadcastHealthChangeEvent(
    WorldContextObject,
    CharacterActor,
    PreviousHealth,
    NewHealth,
    MaxHealth
);
```

## GameplayTag Hierarchy

The event system uses a hierarchical tag structure for flexible event matching:

```
DelveDeep.Event
├── Combat (Damage, Attack, Kill, Death)
├── Character (Health, Resource, Status)
├── Progression (Experience, Upgrade, Achievement)
├── World (Depth, Room, Item)
└── System (Save, Load, Pause)
```

**Hierarchical Matching Example**:
- Listener registered for `DelveDeep.Event.Combat` receives ALL combat events
- Listener registered for `DelveDeep.Event.Combat.Damage.Dealt` receives only that specific event
- Listener registered for `DelveDeep.Event` receives ALL events (global listener)

## Validation

### Build-Specific Behavior

**Development Builds** (`!UE_BUILD_SHIPPING`):
- Full payload validation enabled by default
- Comprehensive error checking and reporting
- Can be toggled on/off for testing

**Shipping Builds**:
- Validation completely disabled for performance
- No validation overhead
- Toggle commands have no effect

### Validation Control

```cpp
// Enable validation (development builds only)
EventSubsystem->EnableValidation();

// Disable validation for performance testing
EventSubsystem->DisableValidation();

// Check validation status
bool bIsEnabled = EventSubsystem->IsValidationEnabled();
```

### Console Commands

```bash
# Enable/disable validation
DelveDeep.Events.EnableValidation
DelveDeep.Events.DisableValidation

# Test payload validation
DelveDeep.Events.ValidateAllPayloads
```

## Performance Monitoring

### Metrics Tracking

```cpp
// Get performance metrics
const FEventSystemMetrics& Metrics = EventSubsystem->GetPerformanceMetrics();

// Access metrics
int32 TotalEvents = Metrics.TotalEventsBroadcast;
int32 TotalInvocations = Metrics.TotalListenerInvocations;
double AvgBroadcastTime = Metrics.AverageTimePerBroadcast;
double AvgListenerTime = Metrics.AverageTimePerListener;

// Reset metrics
EventSubsystem->ResetPerformanceMetrics();
```

### Console Commands

```bash
# Show performance metrics
DelveDeep.Events.ShowMetrics

# Reset metrics
DelveDeep.Events.ResetMetrics

# Show event history
DelveDeep.Events.ShowEventHistory

# Enable detailed logging
DelveDeep.Events.EnableEventLogging

# Disable logging
DelveDeep.Events.DisableEventLogging

# List listeners for specific tag
DelveDeep.Events.ListListeners DelveDeep.Event.Combat.Damage

# List all listeners
DelveDeep.Events.ListAllListeners

# Broadcast test event
DelveDeep.Events.BroadcastTestEvent DelveDeep.Event.Test
```

### Profiling

Use Unreal's stat system for detailed profiling:

```bash
# Show event system stats
stat DelveDeep.Events

# Available stats:
# - Broadcast Event (cycle counter)
# - Invoke Listeners (cycle counter)
# - Process Deferred (cycle counter)
# - Active Listeners (counter)
# - Events Per Frame (counter)
```

## Migration from Direct Coupling

### Before: Direct System Coupling

```cpp
void UCombatComponent::DealDamage(AActor* Target, float Damage)
{
    // Apply damage
    Target->TakeDamage(Damage);
    
    // Direct call to UI - tight coupling!
    UMyHUDWidget* HUD = GetHUD();
    HUD->UpdateHealthBar(Target);
    
    // Direct call to progression - tight coupling!
    UProgressionSystem* Progression = GetProgression();
    Progression->AwardExperience(10);
}
```

### After: Event-Driven Architecture

```cpp
void UCombatComponent::DealDamage(AActor* Target, float Damage)
{
    // Apply damage
    Target->TakeDamage(Damage);
    
    // Broadcast event - loose coupling!
    FDelveDeepDamageEventPayload Payload;
    Payload.EventTag = FGameplayTag::RequestGameplayTag("DelveDeep.Event.Combat.Damage.Dealt");
    Payload.Attacker = GetOwner();
    Payload.Victim = Target;
    Payload.DamageAmount = Damage;
    
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

// Progression listens for kill events
void UProgressionSystem::Initialize()
{
    GetEventSubsystem()->RegisterListener(
        FGameplayTag::RequestGameplayTag("DelveDeep.Event.Combat.Kill"),
        [this](const FDelveDeepEventPayload& Payload)
        {
            const auto& KillPayload = static_cast<const FDelveDeepKillEventPayload&>(Payload);
            AwardExperience(KillPayload.ExperienceAwarded);
        },
        this,
        EDelveDeepEventPriority::Normal
    );
}
```

## Best Practices

### Event Broadcasting

1. **Always validate event tags** before broadcasting
2. **Use specific event tags** rather than generic ones when possible
3. **Include all relevant data** in the payload
4. **Consider deferred mode** for events during critical operations
5. **Mark network-relevant events** for future multiplayer support

### Event Listening

1. **Register listeners in Initialize()** or BeginPlay()
2. **Always provide an owner** for automatic cleanup
3. **Use appropriate priority** (High for critical systems, Normal for most, Low for analytics)
4. **Apply filters** to reduce unnecessary invocations
5. **Unregister explicitly** if needed before object destruction

### Performance

1. **Avoid expensive operations** in listener callbacks
2. **Use deferred processing** for batch operations
3. **Monitor metrics** regularly during development
4. **Profile with stat commands** to identify bottlenecks
5. **Disable validation** in shipping builds (automatic)

### Debugging

1. **Enable event logging** when debugging event flow
2. **Use console commands** to inspect listeners and history
3. **Check validation reports** for payload errors
4. **Monitor performance metrics** for slow listeners
5. **Use event history** to trace event sequences

## Troubleshooting

### Events Not Received

**Check**:
- Event tag is valid and matches exactly
- Listener is registered before event is broadcast
- Owner object is still valid
- Filter criteria are not too restrictive
- Event logging is enabled to see broadcasts

### Performance Issues

**Check**:
- Listener callbacks are not too expensive (>5ms warning)
- Number of listeners per event is reasonable (<50)
- Deferred mode is used appropriately
- Validation is disabled in shipping builds
- Stat commands show expected overhead

### Memory Leaks

**Check**:
- Listeners are unregistered when owners are destroyed
- Deferred event queue is not growing unbounded
- Event history is limited to 100 events
- Weak pointers are used for actor references

## Future Extensibility

### Network Replication (Planned)

Event payloads include metadata for future multiplayer:

```cpp
// Mark event as network-relevant
Payload.bNetworkRelevant = true;
Payload.bReliable = true;

// Query network-relevant events
TArray<FGameplayTag> NetworkEvents = EventSubsystem->GetNetworkRelevantEvents();
```

### Custom Event Types

Create custom payloads by deriving from `FDelveDeepEventPayload`:

```cpp
USTRUCT(BlueprintType)
struct FMyCustomEventPayload : public FDelveDeepEventPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Custom")
    FString CustomData;

    virtual bool Validate(FValidationContext& Context) const override
    {
        bool bIsValid = FDelveDeepEventPayload::Validate(Context);
        
        if (CustomData.IsEmpty())
        {
            Context.AddError(TEXT("CustomData cannot be empty"));
            bIsValid = false;
        }
        
        return bIsValid;
    }
};
```

## Related Documentation

- [ValidationSystem.md](ValidationSystem.md) - FValidationContext integration
- [DataDrivenConfiguration.md](DataDrivenConfiguration.md) - Configuration system patterns
- [Performance-Testing.md](Performance-Testing.md) - Performance testing guidelines

## Implementation Files

**Headers**:
- `Source/DelveDeep/Public/DelveDeepEventSubsystem.h`
- `Source/DelveDeep/Public/DelveDeepEventPayload.h`
- `Source/DelveDeep/Public/DelveDeepEventTypes.h`
- `Source/DelveDeep/Public/DelveDeepEventBlueprintLibrary.h`
- `Source/DelveDeep/Public/DelveDeepEventCommands.h`

**Implementation**:
- `Source/DelveDeep/Private/DelveDeepEventSubsystem.cpp`
- `Source/DelveDeep/Private/DelveDeepEventPayload.cpp`
- `Source/DelveDeep/Private/DelveDeepEventTypes.cpp`
- `Source/DelveDeep/Private/DelveDeepEventBlueprintLibrary.cpp`
- `Source/DelveDeep/Private/DelveDeepEventCommands.cpp`

**Configuration**:
- `Config/DefaultGameplayTags.ini` - GameplayTag hierarchy

---

**Last Updated**: October 24, 2025
**Status**: Implemented and Tested
**Performance**: All targets met
