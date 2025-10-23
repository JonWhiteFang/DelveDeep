# Implementation Plan

- [x] 1. Set up core event system infrastructure
  - Create UDelveDeepEventSubsystem class inheriting from UGameInstanceSubsystem
  - Implement Initialize() and Deinitialize() lifecycle methods with logging
  - Add DELVEDEEP_API macro and proper UCLASS specifiers (BlueprintType)
  - Create dedicated log category LogDelveDeepEvents
  - _Requirements: 1.1, 2.1, 3.1, 4.1_

- [x] 2. Implement base event payload structures
  - Create FDelveDeepEventPayload base struct with EventTag, Timestamp, and Instigator fields
  - Implement virtual Validate() method using FValidationContext
  - Add BlueprintType specifier and proper UPROPERTY declarations
  - Create validation helper methods for common checks (null pointers, ranges)
  - _Requirements: 1.2, 8.1, 8.3_

- [x] 3. Create specific event payload types
  - Implement FDelveDeepDamageEventPayload with Attacker, Victim, DamageAmount, DamageType
  - Implement FDelveDeepHealthChangeEventPayload with Character, PreviousHealth, NewHealth, MaxHealth
  - Implement FDelveDeepKillEventPayload with Killer, Victim, ExperienceAwarded, VictimType
  - Implement FDelveDeepAttackEventPayload with Attacker, AttackLocation, AttackType, AttackRadius
  - Add Validate() override for each payload type with specific validation rules
  - _Requirements: 1.2, 2.4, 3.4, 4.4, 8.1, 8.2_

- [x] 4. Implement event priority and filtering systems
  - Create EDelveDeepEventPriority enum with High, Normal, Low values
  - Create FDelveDeepEventFilter struct with actor and spatial filtering fields
  - Implement PassesFilter() method for filter evaluation
  - Create FDelveDeepEventListener struct with Callback, Owner, Priority, Filter fields
  - Create FEventListenerList struct with priority-separated listener arrays
  - _Requirements: 2.2, 3.2, 4.2_

- [x] 5. Implement listener registration system
  - Create RegisterListener() method accepting EventTag, Callback, Owner, Priority, Filter
  - Implement unique FDelegateHandle generation for each listener
  - Create HandleToListenerMap for O(1) handle-to-listener lookup
  - Store listeners in EventRegistry TMap organized by GameplayTag
  - Add listeners to appropriate priority array within FEventListenerList
  - _Requirements: 2.1, 2.2, 3.2, 4.2_

- [x] 6. Implement listener unregistration and cleanup
  - Create UnregisterListener() method accepting FDelegateHandle
  - Implement UnregisterAllListeners() method accepting owner UObject pointer
  - Add automatic cleanup of stale listeners with invalid weak pointers
  - Implement lazy cleanup during low-activity frames
  - Log cleanup operations in verbose mode
  - _Requirements: 2.3, 3.5_

- [x] 7. Implement event broadcasting system
  - Create BroadcastEvent() method accepting const FDelveDeepEventPayload reference
  - Implement hierarchical GameplayTag matching for listener lookup
  - Invoke listeners in priority order (High → Normal → Low)
  - Apply event filters before invoking each listener
  - Add SCOPE_CYCLE_COUNTER for performance profiling
  - _Requirements: 1.1, 1.3, 1.5, 2.1, 3.1, 4.1_

- [x] 8. Implement error handling for event broadcasting
  - Validate event payload before broadcasting using FValidationContext
  - Wrap listener callbacks in try-catch blocks
  - Log errors with listener details when callbacks throw exceptions
  - Continue processing remaining listeners after callback failure
  - Track failed listener invocations in performance metrics
  - _Requirements: 2.5, 8.1, 8.2, 8.4_

- [ ] 9. Implement deferred event processing system
  - Create DeferredEventQueue TArray for queued events
  - Implement EnableDeferredMode() and DisableDeferredMode() methods
  - Create BroadcastEventDeferred() method to queue events
  - Implement ProcessDeferredEvents() to process queued events in order
  - Add queue overflow protection with MaxDeferredEvents limit (1000)
  - Log warnings when queue reaches 80% capacity
  - _Requirements: 4.5, 6.1, 6.2, 6.3, 6.4, 6.5_

- [ ] 10. Implement performance metrics tracking
  - Create FEventSystemMetrics struct with broadcast/listener timing fields
  - Track TotalEventsBroadcast, TotalListenerInvocations, AverageTimePerBroadcast
  - Implement RecordBroadcast() method to update metrics
  - Add FCriticalSection for thread-safe metrics access
  - Create GetPerformanceMetrics() method for metrics query
  - Log performance warnings when listener callbacks exceed 5ms
  - _Requirements: 9.1, 9.2, 9.3, 9.4_

- [ ] 11. Implement event history and debugging
  - Create FEventRecord struct with EventTag, Timestamp, ListenerCount, ProcessingTime
  - Implement TCircularBuffer for last 100 events (MaxHistorySize)
  - Store event records during broadcasting
  - Create GetEventHistory() method for history query
  - Implement event logging with configurable verbosity
  - _Requirements: 7.1, 7.4_

- [ ] 12. Implement console commands for debugging
  - Create DelveDeep.Events.ListListeners command to display listeners by tag
  - Create DelveDeep.Events.ListAllListeners command to display all registered listeners
  - Create DelveDeep.Events.ShowMetrics command to display performance statistics
  - Create DelveDeep.Events.ShowEventHistory command to display recent events
  - Create DelveDeep.Events.EnableEventLogging and DisableEventLogging commands
  - Create DelveDeep.Events.BroadcastTestEvent command for testing
  - Create DelveDeep.Events.ValidateAllPayloads command for validation testing
  - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5_

- [ ] 13. Implement stat groups for profiling
  - Create STATGROUP_DelveDeepEvents stat group
  - Add STAT_BroadcastEvent cycle counter for event broadcasting
  - Add STAT_InvokeListeners cycle counter for listener invocation
  - Add STAT_ProcessDeferred cycle counter for deferred processing
  - Add STAT_ActiveListeners counter for active listener count
  - Add STAT_EventsPerFrame counter for events per frame tracking
  - _Requirements: 9.5_

- [ ] 14. Implement GameplayTag hierarchy
  - Define DelveDeep.Event root tag with combat, character, progression, world, system subtags
  - Create combat event tags (Damage, Attack, Kill, Death)
  - Create character event tags (Health, Resource, Status)
  - Create progression event tags (Experience, Upgrade, Achievement)
  - Create world event tags (Depth, Room, Item)
  - Create system event tags (Save, Load, Pause)
  - Document tag hierarchy in design document
  - _Requirements: 1.5_

- [ ] 15. Implement Blueprint integration
  - Create UDelveDeepEventBlueprintLibrary function library class
  - Implement GetEventSubsystem() Blueprint-callable function with WorldContext
  - Create BroadcastDamageEvent() Blueprint helper function
  - Create BroadcastHealthChangeEvent() Blueprint helper function
  - Create FDelveDeepEventDelegate dynamic delegate for Blueprint callbacks
  - Implement RegisterListenerBlueprint() for Blueprint event registration
  - _Requirements: 1.1, 2.1, 3.1_

- [ ] 16. Implement network replication metadata
  - Add bNetworkRelevant and bReliable fields to FDelveDeepEventPayload
  - Create ShouldReplicate() virtual method for replication check
  - Add Serialize() virtual method stub for future network support
  - Validate that network-relevant events use serializable payload types
  - Create GetNetworkRelevantEvents() query method
  - _Requirements: 10.1, 10.2, 10.3, 10.4, 10.5_

- [ ] 17. Implement conditional validation for build configurations
  - Add preprocessor guards for development vs shipping builds
  - Enable comprehensive validation in development builds (!UE_BUILD_SHIPPING)
  - Disable validation checks in shipping builds for performance
  - Add validation toggle console command for testing
  - Document validation behavior differences between build types
  - _Requirements: 8.4, 8.5_

- [ ] 18. Create integration examples and documentation
  - Write example code for combat system event broadcasting
  - Write example code for UI system event listening
  - Write example code for progression system event handling
  - Document migration path from direct coupling to event-driven
  - Create troubleshooting guide for common event system issues
  - Update DOCUMENTATION_INDEX.md with event system documentation
  - _Requirements: 1.1, 2.1, 3.1, 4.1_

- [ ] 19. Write unit tests for core functionality
  - Test event broadcasting to single and multiple listeners
  - Test priority-based listener execution order
  - Test hierarchical GameplayTag matching
  - Test listener registration and unregistration
  - Test automatic cleanup of stale listeners
  - Test event filtering (actor-specific and spatial)
  - Test deferred event processing and queue management
  - Test payload validation and error handling
  - _Requirements: 1.1, 1.3, 1.5, 2.1, 2.2, 2.3, 3.2, 4.2, 4.5, 6.1, 6.3, 8.1_

- [ ] 20. Write performance tests
  - Measure event broadcast time with 1, 10, 50, 100 listeners
  - Verify <1ms broadcast time for 50 listeners
  - Verify <0.1ms overhead per event
  - Test deferred processing time for 1000 queued events
  - Measure memory usage with 1000+ listeners
  - Verify no memory leaks during stress testing
  - Profile with stat DelveDeep.Events command
  - _Requirements: 1.3, 4.1, 6.5, 9.1, 9.3, 9.4, 9.5_

- [ ] 21. Write integration tests
  - Test event flow from combat system to UI and progression systems
  - Test cross-system communication with 3+ systems
  - Verify event ordering across multiple systems
  - Test realistic event load scenarios
  - Verify performance with realistic listener counts
  - _Requirements: 1.1, 2.1, 3.1, 4.1_
