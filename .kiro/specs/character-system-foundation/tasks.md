# Implementation Plan

- [x] 1. Create base character component infrastructure




  - Create Source/DelveDeep/Public/Character/ directory structure
  - Create DelveDeepCharacterComponent.h with base component class
  - Implement GetCharacterOwner() method for owner access
  - Add virtual InitializeFromCharacterData() method for data loading
  - Implement ValidateComponent() method using FValidationContext
  - Disable tick by default with PrimaryComponentTick.bCanEverTick = false
  - Add DELVEDEEP_API macro and proper Blueprint specifiers
  - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5_

- [x] 2. Create base character actor class
  - Create DelveDeepCharacter.h and .cpp in Source/DelveDeep/Public/Character/
  - Inherit from APaperCharacter for 2D sprite support
  - Add CharacterClassName property for data asset lookup
  - Create component references (StatsComponent, AbilitiesComponent, EquipmentComponent)
  - Implement constructor with component creation
  - Disable tick by default with PrimaryActorTick.bCanEverTick = false
  - Add Blueprint specifiers (BlueprintType, Blueprintable)
  - _Requirements: 1.1, 1.2, 1.4_

- [x] 3. Implement character data loading
  - Implement InitializeFromData() method in ADelveDeepCharacter
  - Query UDelveDeepConfigurationManager for character data using CharacterClassName
  - Implement ValidateCharacterData() using FValidationContext
  - Add fallback values for missing or invalid data
  - Log errors with detailed context using UE_LOG
  - Call InitializeComponents() after data validation
  - _Requirements: 1.3, 5.1, 5.5, 17.1, 17.5_

- [x] 4. Implement stats component structure
  - Create DelveDeepStatsComponent.h and .cpp
  - Inherit from UDelveDeepCharacterComponent
  - Add properties for BaseHealth, BaseResource, BaseDamage, BaseMoveSpeed
  - Add properties for CurrentHealth, MaxHealth, CurrentResource, MaxResource
  - Create FDelveDeepStatModifier struct with Modifier, Duration, RemainingTime
  - Add TMap<FName, FDelveDeepStatModifier> for active modifiers
  - Add bStatsDirty flag for cached stat calculations
  - _Requirements: 3.1, 3.2, 3.3, 8.3_

- [ ] 5. Implement stats component initialization
  - Override InitializeFromCharacterData() in UDelveDeepStatsComponent
  - Load base stats from UDelveDeepCharacterData (BaseHealth, BaseMana, BaseDamage, BaseMoveSpeed)
  - Set CurrentHealth to MaxHealth on initialization
  - Set CurrentResource to MaxResource on initialization
  - Implement ValidateComponent() to check stat ranges
  - _Requirements: 3.1, 5.2, 17.2_

- [ ] 6. Implement health modification methods
  - Create ModifyHealth() method accepting float delta
  - Clamp CurrentHealth between 0 and MaxHealth
  - Broadcast StatChanged event through UDelveDeepEventSubsystem
  - Call OnStatChanged Blueprint event
  - Check for death condition (CurrentHealth <= 0)
  - Validate delta parameter using FValidationContext
  - _Requirements: 3.2, 3.4, 8.1, 10.2_

- [ ] 7. Implement resource modification methods
  - Create ModifyResource() method accepting float delta
  - Clamp CurrentResource between 0 and MaxResource
  - Broadcast StatChanged event through UDelveDeepEventSubsystem
  - Call OnResourceChanged Blueprint event
  - Support class-specific resource types (Mana, Energy, Rage, Souls)
  - Validate delta parameter using FValidationContext
  - _Requirements: 3.3, 3.4, 4.5, 8.2_

- [ ] 8. Implement stat modifier system
  - Create AddStatModifier() method accepting StatName, Modifier, Duration
  - Store modifiers in ActiveModifiers TMap
  - Implement RemoveStatModifier() method
  - Create RecalculateStats() method applying all modifiers
  - Implement ApplyModifiers() helper for calculating modified values
  - Mark stats dirty when modifiers change
  - Set up timer for CleanupExpiredModifiers() every 1 second
  - _Requirements: 8.3, 8.4, 8.5_

- [ ] 9. Implement stat caching and recalculation
  - Cache MaxHealth, MaxResource, MoveSpeed in component
  - Only recalculate when bStatsDirty is true
  - Implement GetModifiedStat() method returning cached values
  - Update UCharacterMovementComponent MaxWalkSpeed when MoveSpeed changes
  - Mark stats dirty when modifiers added/removed
  - _Requirements: 8.4, 12.2, 16.3_

- [ ] 10. Implement character damage handling
  - Create TakeDamage() method in ADelveDeepCharacter
  - Accept DamageAmount and DamageSource parameters
  - Validate damage amount is positive using FValidationContext
  - Call StatsComponent->ModifyHealth(-DamageAmount)
  - Broadcast CharacterDamaged event with FDelveDeepDamagePayload
  - Call OnDamaged Blueprint event
  - Apply damage visual feedback (sprite flash)
  - _Requirements: 10.1, 10.2, 10.3, 10.4, 10.5_

- [ ] 11. Implement character healing
  - Create Heal() method in ADelveDeepCharacter
  - Accept HealAmount parameter
  - Validate heal amount is positive using FValidationContext
  - Call StatsComponent->ModifyHealth(HealAmount)
  - Broadcast CharacterHealed event with FDelveDeepHealthPayload
  - Call OnHealed Blueprint event
  - Apply healing visual effect (sprite glow)
  - _Requirements: 11.1, 11.2, 11.3, 11.4, 11.5_

- [ ] 12. Implement character death handling
  - Create Die() method in ADelveDeepCharacter
  - Set bIsDead flag to true
  - Disable input and collision
  - Broadcast CharacterDeath event with FDelveDeepCharacterDeathPayload
  - Call OnDeath Blueprint event
  - Play death animation if available
  - Set timer to destroy actor after 2 seconds
  - _Requirements: 1.5, 9.1, 9.2, 9.3, 9.4_

- [ ] 13. Implement character respawn
  - Create Respawn() method in ADelveDeepCharacter
  - Reset bIsDead flag to false
  - Call StatsComponent->ResetToMaxValues()
  - Clear all temporary stat modifiers
  - Re-enable input and collision
  - Reset sprite to idle animation
  - _Requirements: 9.5_

- [ ] 14. Create Warrior character class
  - Create DelveDeepWarrior.h and .cpp
  - Inherit from ADelveDeepCharacter
  - Set CharacterClassName to "Warrior" in constructor
  - Implement GenerateRage() method
  - Override TakeDamage() to generate Rage from damage taken
  - Implement PerformCleaveAttack() placeholder method
  - Set MaxRage constant to 100.0f
  - _Requirements: 4.1, 7.1, 7.5_

- [ ] 15. Create Ranger character class
  - Create DelveDeepRanger.h and .cpp
  - Inherit from ADelveDeepCharacter
  - Set CharacterClassName to "Ranger" in constructor
  - Implement RegenerateEnergy() method
  - Start energy regeneration timer in BeginPlay() with 0.1 second interval
  - Implement PerformPiercingShot() placeholder method
  - Set EnergyRegenRate to 10.0f per second
  - _Requirements: 4.2, 7.2, 7.5_

- [ ] 16. Create Mage character class
  - Create DelveDeepMage.h and .cpp
  - Inherit from ADelveDeepCharacter
  - Set CharacterClassName to "Mage" in constructor
  - Implement RegenerateMana() method
  - Start mana regeneration timer in BeginPlay() with 0.1 second interval
  - Implement CastFireball() placeholder method
  - Set ManaRegenRate to 5.0f per second
  - _Requirements: 4.3, 7.3, 7.5_

- [ ] 17. Create Necromancer character class
  - Create DelveDeepNecromancer.h and .cpp
  - Inherit from ADelveDeepCharacter
  - Set CharacterClassName to "Necromancer" in constructor
  - Implement CollectSoul() method
  - Register for enemy death events in BeginPlay()
  - Implement SummonMinion() placeholder method
  - Add ActiveMinions TArray for tracking summoned minions
  - Set MaxSouls to 10.0f
  - _Requirements: 4.4, 7.4, 7.5_

- [ ] 18. Implement event payload structures
  - Create FDelveDeepCharacterDeathPayload struct in DelveDeepEventPayload.h
  - Add Character, Killer, DeathLocation properties
  - Create FDelveDeepStatChangedPayload struct
  - Add Character, StatName, OldValue, NewValue properties
  - Create FDelveDeepAbilityUsedPayload struct
  - Add Character, Ability, ResourceCost properties
  - Mark all structs with BlueprintType specifier
  - _Requirements: 18.2, 18.4_

- [ ] 19. Implement event broadcasting
  - Create BroadcastDeathEvent() in ADelveDeepCharacter
  - Create BroadcastDamageEvent() with damage details
  - Create BroadcastHealEvent() with heal amount
  - Use UDelveDeepEventSubsystem->BroadcastEvent()
  - Use appropriate GameplayTags (DelveDeep.Character.Death, DelveDeep.Character.Damaged, etc.)
  - Validate event payloads using FValidationContext
  - _Requirements: 3.4, 3.5, 18.1, 18.3, 18.5_

- [ ] 20. Implement Blueprint events
  - Add OnDeath BlueprintImplementableEvent to ADelveDeepCharacter
  - Add OnDamaged BlueprintImplementableEvent with DamageAmount and DamageSource
  - Add OnHealed BlueprintImplementableEvent with HealAmount
  - Add OnStatChanged BlueprintImplementableEvent with StatName, OldValue, NewValue
  - Add OnResourceChanged BlueprintImplementableEvent with OldValue, NewValue
  - Add OnAbilityUsed BlueprintImplementableEvent with Ability
  - Add OnWeaponEquipped BlueprintImplementableEvent with Weapon
  - _Requirements: 15.4_

- [ ] 21. Implement Blueprint-callable methods
  - Mark GetCurrentHealth(), GetMaxHealth() with BlueprintPure
  - Mark GetCurrentResource(), GetMaxResource() with BlueprintPure
  - Mark TakeDamage(), Heal() with BlueprintCallable
  - Mark Die(), Respawn() with BlueprintCallable
  - Mark IsDead() with BlueprintPure
  - Add Category meta specifier to all Blueprint-exposed members
  - _Requirements: 15.1, 15.2, 15.3, 15.5_

- [ ] 22. Create abilities component placeholder
  - Create DelveDeepAbilitiesComponent.h and .cpp
  - Inherit from UDelveDeepCharacterComponent
  - Add Abilities TArray for storing ability references
  - Add AbilityCooldowns TMap for tracking cooldowns
  - Implement AddAbility() and RemoveAbility() methods
  - Implement UseAbility() and CanUseAbility() placeholder methods
  - Override InitializeFromCharacterData() to load starting abilities
  - _Requirements: 13.1, 13.2, 13.3, 13.4_

- [ ] 23. Create equipment component placeholder
  - Create DelveDeepEquipmentComponent.h and .cpp
  - Inherit from UDelveDeepCharacterComponent
  - Add CurrentWeapon property for storing weapon reference
  - Implement EquipWeapon() method
  - Implement GetEquipmentStatModifier() placeholder method
  - Override InitializeFromCharacterData() to load starting weapon
  - Add ApplyWeaponModifiers() and RemoveWeaponModifiers() placeholder methods
  - _Requirements: 14.1, 14.2, 14.3, 14.4_

- [ ] 24. Implement movement integration
  - Access UCharacterMovementComponent from APaperCharacter
  - Apply MoveSpeed stat to MaxWalkSpeed in RecalculateStats()
  - Support AddMovementInput() for movement control
  - Update sprite facing direction based on velocity
  - Use capsule collision component for character collision
  - _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5_

- [ ] 25. Implement sprite rendering integration
  - Access UPaperFlipbookComponent from APaperCharacter
  - Implement PlayIdleAnimation() method loading IdleAnimation from data
  - Implement PlayWalkAnimation() method loading WalkAnimation from data
  - Implement PlayAttackAnimation() method loading AttackAnimation from data
  - Implement PlayDeathAnimation() method loading DeathAnimation from data
  - Support sprite flipping based on movement direction
  - Support sprite color tinting for damage feedback
  - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5_

- [ ] 26. Implement performance profiling
  - Add DECLARE_CYCLE_STAT for TakeDamage method
  - Add DECLARE_CYCLE_STAT for RecalculateStats method
  - Add SCOPE_CYCLE_COUNTER in performance-critical methods
  - Register with UDelveDeepTelemetrySubsystem in BeginPlay()
  - Increment/decrement entity count for character tracking
  - _Requirements: 16.5_

- [ ] 27. Create Blueprint function library
  - Create DelveDeepCharacterBlueprintLibrary.h and .cpp
  - Inherit from UBlueprintFunctionLibrary
  - Implement SpawnCharacter() with WorldContext parameter
  - Implement IsCharacterAlive() helper
  - Implement GetHealthPercentage() and GetResourcePercentage() helpers
  - Implement ApplyDamageToCharacter() and HealCharacter() wrappers
  - Implement AddTemporaryStatBoost() helper
  - _Requirements: 15.1, 15.2, 15.3_

- [ ] 28. Implement console commands
  - Create SpawnCharacter console command accepting class name
  - Create ModifyHealth console command accepting amount
  - Create ModifyResource console command accepting amount
  - Create DealDamage console command accepting amount
  - Create ShowCharacterState console command displaying stats
  - Create KillCharacter console command
  - Create RespawnCharacter console command
  - _Requirements: 20.1, 20.2, 20.3, 20.4, 20.5_

- [ ] 29. Create character test utilities
  - Add CreateTestCharacter() to DelveDeepTestUtilities
  - Add SetCharacterStats() helper for testing
  - Add SimulateDamage() helper for damage testing
  - Add SimulateHealing() helper for healing testing
  - Add TriggerAbility() helper for ability testing
  - Add MockEventListener for event verification
  - _Requirements: 19.1, 19.2, 19.3, 19.4, 19.5_

- [ ] 30. Write character initialization tests
  - Test character spawns with all components created
  - Test character loads data from configuration manager
  - Test character initializes stats from data asset
  - Test character handles missing data gracefully with fallbacks
  - Test character validates data using FValidationContext
  - _Requirements: 1.3, 5.1, 5.5, 17.1_

- [ ] 31. Write stats component tests
  - Test health modification clamps to valid range
  - Test resource modification clamps to valid range
  - Test stat modifiers apply correctly
  - Test expired modifiers are cleaned up
  - Test stat recalculation only occurs when dirty
  - Test stat change events are broadcast
  - _Requirements: 3.2, 3.3, 8.3, 8.4, 8.5_

- [ ] 32. Write damage and healing tests
  - Test TakeDamage reduces health correctly
  - Test lethal damage triggers death
  - Test Heal increases health correctly
  - Test healing clamps to MaxHealth
  - Test damage and heal events are broadcast
  - Test damage visual feedback is applied
  - _Requirements: 10.1, 10.2, 10.3, 11.1, 11.2, 11.3_

- [ ] 33. Write death and respawn tests
  - Test Die() sets bIsDead flag
  - Test Die() disables input and collision
  - Test Die() broadcasts death event
  - Test actor destroys after death timer
  - Test Respawn() resets stats to max
  - Test Respawn() clears temporary modifiers
  - _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5_

- [ ] 34. Write character class tests
  - Test Warrior generates Rage from damage
  - Test Ranger regenerates Energy over time
  - Test Mage regenerates Mana over time
  - Test Necromancer collects Souls from enemy deaths
  - Test resource values clamp to maximum
  - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5_

- [ ] 35. Write event integration tests
  - Test character damage broadcasts event to listeners
  - Test character death broadcasts event to listeners
  - Test character stat changes broadcast events
  - Test event payloads contain correct data
  - Test event filtering by GameplayTag works correctly
  - _Requirements: 18.1, 18.2, 18.3, 18.4, 18.5_

- [ ] 36. Write performance tests
  - Test TakeDamage executes in <1ms
  - Test RecalculateStats executes in <0.5ms
  - Test stat queries use cached values
  - Test character spawning completes in <10ms
  - Verify tick is disabled for character and components
  - _Requirements: 16.1, 16.2, 16.3, 16.4_

- [ ] 37. Write Blueprint integration tests
  - Test Blueprint-callable methods work correctly
  - Test Blueprint events are triggered
  - Test Blueprint function library helpers work
  - Test character can be spawned from Blueprint
  - Test stat modification from Blueprint works
  - _Requirements: 15.1, 15.2, 15.3, 15.4_

- [ ] 38. Write validation tests
  - Test character validates data asset on initialization
  - Test stats component validates stat ranges
  - Test damage validation rejects negative values
  - Test healing validation rejects negative values
  - Test validation errors are logged with context
  - _Requirements: 17.1, 17.2, 17.3, 17.4, 17.5_

- [ ] 39. Write console command tests
  - Test SpawnCharacter command creates character
  - Test ModifyHealth command changes health
  - Test DealDamage command applies damage
  - Test ShowCharacterState command displays stats
  - Test KillCharacter command triggers death
  - _Requirements: 20.1, 20.2, 20.3, 20.4, 20.5_

- [ ] 40. Create character system documentation
  - Create CharacterSystem.md in Documentation/Systems/
  - Document component-based architecture
  - Document character class differences and resource systems
  - Document integration with Configuration, Event, Telemetry systems
  - Provide examples for creating new character classes
  - Document Blueprint integration patterns
  - Update DOCUMENTATION_INDEX.md with character system entry
  - _Requirements: 21.1, 21.2, 21.3, 21.4, 21.5_
