# DelveDeep Project Development Prompt

## Project Context
I'm working on **DelveDeep**, a production-ready top-down 2D action RPG built in Unreal Engine 5.6. This is a comprehensive game development project with extensive C++ implementation, following UE5.6 best practices and modern game architecture patterns.

## Critical Development Constraints
**Code-Only Environment**: This development environment has NO Unreal Engine 5 Editor access. All work must be:
- Pure C++ implementation in the `Source/` directories
- Blueprint-ready design (all classes use `UCLASS(BlueprintType, Blueprintable)`)
- Focused on modular architecture and comprehensive documentation
- Designed for future UE5 Editor integration when available

**Never suggest**: Opening `.uproject` files, creating Blueprint assets, importing visual assets into the editor, or any UE5 Editor operations.

## Project Overview

### Game Description
DelveDeep is a polished roguelike action RPG where players control one of four distinct character classes (Warrior, Ranger, Mage, Necromancer) as they venture into procedurally-generated underground mines, fighting intelligent monsters, collecting coins, and upgrading their abilities. The game emphasizes streamlined gameplay with automatic combat systems while maintaining strategic depth.

### Technical Stack
- **Engine**: Unreal Engine 5.6
- **Language**: Modern C++ (C++20)
- **Rendering**: Paper2D for 2D sprite-based graphics
- **Input**: Enhanced Input System (UE5 standard)
- **AI**: Behavior Trees with AI Perception
- **Target Performance**: 60+ FPS

### Module Structure
```
Source/
├── DelveDeep/          # Main game module (DELVEDEEP_API)
│   ├── Private/        # Implementation files (.cpp)
│   └── Public/         # Header files (.h)
├── DelveDeepCore/      # Shared utilities and base classes
├── DelveDeepUI/        # User interface and HUD systems
└── DelveDeepAudio/     # Audio management and effects
```

## Core Game Systems to Implement

### Phase 1: Core Foundation
**Character System**
- Four distinct character classes: Warrior, Ranger, Mage, Necromancer
- Component-based architecture using `UDelveDeepCharacterComponent` base class
- Class-specific stats, abilities, and combat mechanics
- Character selection system

**Input & Movement**
- Enhanced Input System with multi-device support (keyboard, gamepad)
- Input buffering for responsive controls
- 2D movement with collision detection and wall sliding
- Animation state machine (idle, walk, attack, death)

**Architecture Foundation**
- Modular component-based design
- Blueprint-ready C++ classes with proper specifiers
- Performance-optimized tick functions
- Comprehensive validation patterns

### Phase 2: Combat & AI
**Combat System**
- Automatic targeting with configurable priorities (nearest, lowest health, highest threat)
- Damage calculation with type modifiers (physical, magical, true)
- Attack timing system with cooldowns
- Visual feedback through damage numbers and effects
- Hit validation and collision detection

**Monster AI**
- Behavior tree-driven AI using UE5's AIModule
- AI Perception for player detection
- Dynamic threat assessment and target selection
- Pathfinding integration with NavigationSystem
- Monster spawning system with lifecycle management
- LOD system for AI performance optimization

**Class-Specific Combat**
- **Warrior**: Melee attacks with cleave (area damage)
- **Ranger**: Ranged projectile attacks with piercing
- **Mage**: Spell-casting with area effects and mana management
- **Necromancer**: Minion summoning with soul energy system

### Phase 3: World & Environment
**Procedural Generation**
- Mine generation system with room-based layouts
- Depth-based difficulty progression
- Pathfinding integration for AI navigation
- Environmental hazards and interactive elements

**Visual Systems**
- Paper2D integration for 2D sprite rendering
- Atmospheric effects (lighting, particles, environmental)
- Environmental animation (machinery, structural elements, water)
- High-performance tile rendering with batching and LOD
- Texture streaming for memory optimization
- Parallax background layers for depth

**Asset Pipeline**
- Leonardo.ai integration for sprite generation
- Asset validation system (dimensions, transparency, format)
- Asset manifest for tracking generated content
- Automated checklist updates

### Phase 4: Progression & Persistence
**Progression System**
- Dynamic upgrade cost calculation with balanced progression
- Achievement system with milestone detection
- Statistics tracking for player performance analytics
- Skill tree/upgrade tree system
- Experience and leveling system

**Save System**
- Robust data persistence with corruption protection
- Multiple backup strategies with automatic recovery
- Multi-layered validation and integrity checking
- Fast save/load times (target: sub-100ms)

**Settings Management**
- Comprehensive game settings (audio, video, controls, gameplay)
- Real-time settings application
- Settings persistence across sessions

### Phase 5: UI & Polish
**User Interface**
- Main menu and character selection screen
- In-game HUD (health, mana/energy, abilities, minimap)
- Pause menu with settings access
- Upgrade menu with skill tree visualization
- Death screen with statistics
- Damage numbers and combat feedback
- Tooltip and help system

**Audio System**
- Music manager with dynamic transitions
- Sound effects for combat, movement, UI
- Atmospheric audio for environments
- Audio occlusion based on environment
- Audio pooling for performance
- Spatial audio for 2D space

**Visual Effects**
- Projectile effects with trails
- Melee attack effects (slashes, impacts)
- Area of effect visuals (spell circles, explosions)
- Particle systems (dust, sparks, blood, magic)
- Screen effects (damage vignette, level up flash)

### Phase 6: Content & Balance
**Monster Variety**
- Basic melee monsters (goblins, zombies)
- Ranged monsters (skeleton archers)
- Flying monsters (bats, ghosts)
- Boss encounters with unique mechanics
- Elite/champion variants

**Items & Loot**
- Coin pickup system
- Equipment system (weapons, armor, accessories)
- Consumable items (health potions, buffs)
- Loot tables with rarity tiers
- Item effects and stat modifications

**Meta-Progression**
- Persistent upgrades between runs
- Character/ability unlock system
- Leaderboard and statistics tracking
- Achievement system with rewards

## Development Priorities & Improvements

### High Priority (Implement First)

**1. Data-Driven Configuration**
Move hardcoded values to data assets for easier balancing:
```cpp
// Create UDataAsset classes for configuration
UCLASS(BlueprintType)
class DELVEDEEP_API UDelveDeepCharacterData : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float BaseHealth = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float BaseDamage = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MoveSpeed = 300.0f;
};
```

Use data tables for:
- Character base stats and scaling
- Monster configurations
- Upgrade costs and effects
- Weapon and item definitions
- Ability parameters

**2. Automated Testing Framework**
Set up comprehensive testing infrastructure:
- Integrate Google Test or Catch2 for C++ unit tests
- Create automated test suite that runs without Editor
- Add performance regression testing
- Implement validation tests for all core systems

Example test structure:
```cpp
TEST(DelveDeepCombat, DamageCalculation)
{
    // Test damage calculation with various modifiers
    EXPECT_EQ(CalculateDamage(100, 0.5f), 50.0f);
}
```

**3. Centralized Event System**
Implement gameplay event bus for loose coupling:
```cpp
// Centralized event dispatcher
UCLASS()
class DELVEDEEP_API UDelveDeepEventSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDamaged, AActor*, Attacker, float, Damage);
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerDamaged OnPlayerDamaged;
    
    // Use gameplay tags for event categorization
    void BroadcastGameplayEvent(FGameplayTag EventTag, const FGameplayEventData& EventData);
};
```

**4. Enhanced Validation System**
Improve validation with context and detailed error reporting:
```cpp
struct FValidationContext
{
    FString SystemName;
    FString OperationName;
    TArray<FString> ValidationErrors;
    TArray<FString> ValidationWarnings;
    
    void AddError(const FString& Error);
    void AddWarning(const FString& Warning);
    bool IsValid() const { return ValidationErrors.Num() == 0; }
    FString GetReport() const;
};
```

**5. Performance Telemetry**
Add production-ready performance monitoring:
```cpp
// Define stat groups for profiling
DECLARE_STATS_GROUP(TEXT("DelveDeep"), STATGROUP_DelveDeep, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Combat System"), STAT_CombatSystem, STATGROUP_DelveDeep);
DECLARE_CYCLE_STAT(TEXT("AI System"), STAT_AISystem, STATGROUP_DelveDeep);
DECLARE_CYCLE_STAT(TEXT("Rendering"), STAT_Rendering, STATGROUP_DelveDeep);

// Use in performance-critical code
void UDelveDeepCombatComponent::ProcessCombat()
{
    SCOPE_CYCLE_COUNTER(STAT_CombatSystem);
    // Combat logic
}
```

### Medium Priority (Implement Next)

**6. Code Quality Tooling**
Set up automated code quality checks:
- Configure clang-format with UE5 style guide
- Integrate clang-tidy for static analysis
- Add pre-commit hooks for formatting
- Set up Doxygen for documentation generation

Create `.clang-format` file:
```yaml
BasedOnStyle: LLVM
IndentWidth: 4
TabWidth: 4
UseTab: ForIndentation
ColumnLimit: 120
```

**7. Module Refactoring**
Split large modules for better organization:
```
Source/
├── DelveDeep/              # Core game logic
├── DelveDeepCharacter/     # Character-specific systems
├── DelveDeepCombat/        # Combat mechanics
├── DelveDeepAI/            # AI and monsters
├── DelveDeepWorld/         # World generation
├── DelveDeepCore/          # Shared utilities
├── DelveDeepUI/            # User interface
└── DelveDeepAudio/         # Audio systems
```

**8. Advanced Documentation**
Enhance documentation with:
- Architecture Decision Records (ADRs) for major design choices
- Troubleshooting guide for common issues
- Performance tuning guide with profiling examples
- Contribution guidelines for team expansion
- System integration diagrams

**9. Inventory & Loot System**
Implement item management:
- Inventory component with slot management
- Equipment system with stat modifications
- Loot drop system with rarity tiers
- Item pickup and UI integration

**10. Quest/Objective System**
Add structured objectives:
- Depth-based objectives (reach level X)
- Kill count objectives (defeat X monsters)
- Collection objectives (gather X coins)
- Achievement tracking with UI notifications

### Low Priority (Nice to Have)

**11. Localization System**
Prepare for internationalization:
- String table system using UE5's localization
- Text asset management
- Language switching support
- RTL language support

**12. Networking Foundation**
Add multiplayer capability:
- Implement replication for core systems
- Add client-server architecture
- Create dedicated server configuration
- Implement lag compensation

**13. Advanced Visual Effects**
Enhance visual polish:
- Screen space effects (bloom, vignette)
- Camera shake for impacts
- Slow-motion effects for critical hits
- Advanced particle systems

**14. Modding Support**
Enable community content:
- Plugin architecture for mods
- Data asset exposure for modding
- Custom character/monster support
- Level editor tools

## Development Guidelines

### Mandatory Code Patterns

**Class Declaration Template**:
```cpp
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep")
class DELVEDEEP_API ADelveDeepActor : public AActor
{
    GENERATED_BODY()

public:
    ADelveDeepActor();

protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "DelveDeep")
    void PerformAction();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DelveDeep")
    float ActionValue = 100.0f;

private:
    // Always disable unnecessary ticking
    // Set in constructor: PrimaryActorTick.bCanEverTick = false;
};
```

**Performance Requirements**:
- Always disable ticking unless specifically needed: `PrimaryActorTick.bCanEverTick = false`
- Use tick intervals for non-critical updates: `PrimaryActorTick.TickInterval = 0.2f`
- Pre-allocate containers when size is known: `Items.Reserve(ExpectedMaxSize)`
- Prefer UE5 containers (`TArray`, `TMap`, `TSet`) over STL
- Use object pooling for frequently spawned objects (projectiles, effects, monsters)

**Enhanced Validation Pattern**:
```cpp
bool IsValidGameObject(const UObject* Object) const
{
    return Object && IsValid(Object) && !Object->IsPendingKill();
}

bool ValidateGameplayAction(AActor* Actor, int32 Value, FValidationContext& Context)
{
    if (!IsValidGameObject(Actor))
    {
        Context.AddError(TEXT("Invalid actor reference"));
        return false;
    }
    
    if (Value <= 0)
    {
        Context.AddError(FString::Printf(TEXT("Invalid value: %d"), Value));
        return false;
    }
    
    return true;
}
```

**Event System Pattern**:
```cpp
// Use centralized event system for loose coupling
void UDelveDeepCombatComponent::DealDamage(AActor* Target, float Damage)
{
    // Apply damage
    ApplyDamageInternal(Target, Damage);
    
    // Broadcast event through event subsystem
    if (UDelveDeepEventSubsystem* EventSystem = GetGameInstance()->GetSubsystem<UDelveDeepEventSubsystem>())
    {
        EventSystem->OnDamageDealt.Broadcast(GetOwner(), Target, Damage);
    }
}
```

### Naming Conventions (Mandatory)
- **Classes**: `A` (Actors), `U` (Objects/Components), `F` (Structs), `E` (Enums), `I` (Interfaces)
- **API Exposure**: Use `DELVEDEEP_API` for all public classes
- **File Naming**: `DelveDeepClassName.h/.cpp`
- **Examples**:
  - `ADelveDeepPlayerCharacter` (Actor)
  - `UDelveDeepCombatComponent` (Component)
  - `FDelveDeepPlayerStats` (Struct)
  - `EDelveDeepCharacterClass` (Enum)
  - `IDelveDeepInteractable` (Interface)

### Required Module Dependencies
```cpp
// DelveDeep.Build.cs - Always include these modules
PublicDependencyModuleNames.AddRange(new string[] 
{
    "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
    "Paper2D", "UMG", "AIModule", "NavigationSystem", "GameplayTasks",
    "GameplayTags"  // For event system
});
```

## Console Commands for Testing

### System Testing
```bash
DelveDeep.ValidateAllSystems          # Run comprehensive system validation
DelveDeep.TestInputSystem             # Validate input system functionality
DelveDeep.TestAllInputDevices         # Test all supported input devices
DelveDeep.TestCombatSystem            # Validate combat system components
DelveDeep.TestTargetingSystem         # Test automatic targeting
DelveDeep.TestProgressionSystem       # Validate progression and upgrades
DelveDeep.TestSaveSystem              # Test save/load functionality
DelveDeep.TestAISystem                # Validate AI behavior and pathfinding
```

### Performance Monitoring
```bash
stat fps                              # Display frame rate information
stat memory                           # Monitor memory usage
stat DelveDeep                        # Show DelveDeep-specific metrics
DelveDeep.ShowPerformanceMetrics      # Display detailed performance analysis
DelveDeep.ShowTileMetrics             # Display tile rendering performance
DelveDeep.ShowEffectPerformance       # Display atmospheric effects performance
DelveDeep.ProfileFrame                # Detailed frame profiling
```

### Debug Visualization
```bash
DelveDeep.ShowTargetingDebug          # Visualize targeting system behavior
DelveDeep.ShowAIDebug                 # Display AI behavior and pathfinding
DelveDeep.ShowCollisionDebug          # Visualize collision detection
DelveDeep.ShowTileBounds              # Visualize tile rendering boundaries
DelveDeep.ShowEffectBounds            # Visualize atmospheric effect boundaries
DelveDeep.ToggleDebugHUD              # Show/hide comprehensive debug overlay
DelveDeep.ShowEventLog                # Display gameplay event log
```

## Character Classes

### Warrior
- **Role**: Melee tank with area damage
- **Combat Style**: Close-quarters with cleave attacks
- **Stats**: High health and armor, moderate damage
- **Resource**: Rage (builds with attacks and damage taken)
- **Component**: `UDelveDeepWarriorComponent`

### Ranger
- **Role**: Ranged DPS specialist
- **Combat Style**: Projectile-based attacks with piercing
- **Stats**: High mobility and attack speed, moderate health
- **Resource**: Focus (regenerates over time)
- **Component**: `UDelveDeepRangerComponent`

### Mage
- **Role**: Elemental caster with area effects
- **Combat Style**: Spell-casting with mana management
- **Stats**: High magic damage, low health
- **Resource**: Mana (regenerates slowly)
- **Component**: `UDelveDeepMageComponent`

### Necromancer
- **Role**: Summoner with minion control
- **Combat Style**: Minion-based combat with life manipulation
- **Stats**: Balanced stats with minion scaling
- **Resource**: Soul Energy (gained from kills)
- **Component**: `UDelveDeepNecromancerComponent`

## Required Assets

**Assets will be provided externally.** A comprehensive list of all required assets is to be maintained in `ASSET_CREATION_LIST.md`.

### Asset Categories

**Visual Assets:**
1. **Character Sprites**: 64x64 pixels, 4-direction animations (idle, walk, attack, death) for all 4 classes
2. **Monster Sprites**: 48x48 pixels (basic), 128x128 pixels (bosses) with animations
3. **Environment Tiles**: 32x32 pixels, seamless tiling for floors, walls, transitions
4. **UI Elements**: Various sizes, 9-slice compatible buttons, panels, icons
5. **Combat Effects**: Projectiles, impacts, AoE visuals, particles
6. **Backgrounds**: Parallax layers at 1920x1080+ for depth variation

**Audio Assets:**
1. **Music Tracks**: Main menu, gameplay (depth-based), boss battles, victory/death themes
2. **Character SFX**: Footsteps, attacks, damage, death for all 4 classes
3. **Monster SFX**: Growls, attacks, hits, deaths for all monster types
4. **Combat SFX**: Melee hits, projectiles, explosions, critical hits
5. **Environmental SFX**: Ambient mine sounds, machinery, water, fire
6. **UI SFX**: Button clicks, menu navigation, pickups, level up

**Technical Specifications:**
- Sprite Resolution: 32x32 (tiles), 64x64 (characters), 48x48 (monsters)
- Color Depth: 32-bit RGBA with alpha transparency
- File Format: PNG for sprites, WAV/OGG for audio
- Animation Frame Rate: 8-12 FPS for characters, 12-15 FPS for effects
- Audio: 44.1kHz/48kHz, 16-bit/24-bit, Mono (SFX) / Stereo (Music)

**Asset Naming Convention:**
```
T_[AssetType]_[Name]_[Variant]

Examples:
- T_Character_Warrior_Idle_Down
- T_Monster_Goblin_Walk
- T_UI_Button_Normal
- T_Effect_Arrow_Flying
```

**Integration Notes:**
- All sprites designed for Paper2D (UPaperFlipbookComponent)
- Animations compatible with existing state machine
- Audio compatible with UDelveDeepAudioComponent
- UI elements designed for UMG widgets

**Reference:** See `ASSET_CREATION_LIST.md` for complete asset requirements with priorities and specifications.

## Common Development Tasks

### Adding a New System
1. Create header in `Source/DelveDeep/Public/DelveDeepSystemName.h`
2. Create implementation in `Source/DelveDeep/Private/DelveDeepSystemName.cpp`
3. Use `DELVEDEEP_API` macro for public classes
4. Implement Blueprint integration with `UFUNCTION` and `UPROPERTY`
5. Add console commands for testing
6. Create unit tests for core functionality
7. Create documentation in `Documentation/` appropriate category
8. Update `DOCUMENTATION_INDEX.md`

### Implementing Data-Driven Configuration
1. Create `UDataAsset` subclass for configuration
2. Define `UPROPERTY` fields with `EditAnywhere` specifier
3. Create data table if multiple entries needed
4. Load data assets in system initialization
5. Reference data assets in Blueprint-exposed properties

### Adding Automated Tests
1. Create test file in `Source/DelveDeep/Private/Tests/`
2. Include Google Test or Catch2 headers
3. Write test cases for core functionality
4. Add test to automated test suite
5. Run tests with console command or CI/CD pipeline

### Performance Optimization
1. Profile with `stat DelveDeep` and `SCOPE_CYCLE_COUNTER`
2. Check tick functions - disable if not needed
3. Review object pooling opportunities
4. Optimize container usage and pre-allocation
5. Consider LOD systems for expensive operations
6. Test with `DelveDeep.ShowPerformanceMetrics`
7. Add performance regression tests

## Project Files Structure

### Configuration
- `DelveDeep.uproject` - Unreal Engine project file
- `Config/` - Engine and game configuration files
- `.kiro/` - Kiro IDE configuration and steering rules
- `.clang-format` - Code formatting rules

### Source Code
- `Source/DelveDeep/` - Main game module
- `Source/DelveDeepCore/` - Shared utilities
- `Source/DelveDeepUI/` - UI systems
- `Source/DelveDeepAudio/` - Audio systems

### Documentation
- `Documentation/` - System documentation
- `README.md` - Project overview
- `DOCUMENTATION_INDEX.md` - Documentation navigation hub

### Asset Management
- `ASSET_CREATION_LIST.md` - Comprehensive list of required game assets with specifications
- `Content/` - Game assets (provided externally, organized by UE5 conventions)

### Testing
- `Source/DelveDeep/Private/Tests/` - Unit tests
- `.github/workflows/` - CI/CD pipeline configuration (if using GitHub)

## Recommended Development Sequence

### Week 1-2: Foundation
1. Set up project structure and module organization
2. Implement data-driven configuration system
3. Set up automated testing framework
4. Configure code quality tooling (clang-format, clang-tidy)
5. Create centralized event system
6. Implement enhanced validation system

### Week 3-4: Core Gameplay
1. Implement character system with component architecture
2. Create Enhanced Input System integration
3. Build movement system with collision
4. Implement animation state machine
5. Add console commands for testing

### Week 5-6: Combat & AI
1. Build combat system with automatic targeting
2. Implement damage calculation and hit validation
3. Create monster AI with behavior trees
4. Add monster spawning system
5. Implement class-specific combat mechanics

### Week 7-8: World & Visuals
1. Create procedural mine generation
2. Implement tile rendering system
3. Add atmospheric effects
4. Build environmental animation system
5. Integrate provided visual assets with Paper2D

### Week 9-10: Progression & Persistence
1. Implement progression manager
2. Create save system with validation
3. Build upgrade/skill tree system
4. Add statistics tracking
5. Implement settings manager

### Week 11-12: UI & Audio
1. Create main menu and character selection
2. Build in-game HUD
3. Implement upgrade menu UI
4. Integrate audio system
5. Add visual effects

### Week 13-14: Content & Polish
1. Integrate complete asset set
2. Create monster variety
3. Implement loot system
4. Add meta-progression
5. Balance and tune gameplay

### Week 15-16: Testing & Optimization
1. Comprehensive system testing
2. Performance optimization
3. Bug fixing
4. Documentation completion
5. Final polish

## Important Reminders

### Always Remember
- ❌ **NO UE5 Editor access** - code-only development
- ✅ **Blueprint-ready design** - all classes must be Blueprint-compatible
- ✅ **Performance-first** - optimize tick functions, use object pooling
- ✅ **Data-driven** - use data assets and tables for configuration
- ✅ **Component-based** - prefer composition over inheritance
- ✅ **Event-driven** - use centralized event system for loose coupling
- ✅ **Test-driven** - write automated tests for core systems
- ✅ **Documentation** - update docs when implementing systems
- ✅ **Validation** - use enhanced validation with context
- ✅ **UE5.6 patterns** - follow modern Unreal Engine best practices

### Never Suggest
- Opening `.uproject` files in UE5 Editor
- Creating or modifying Blueprint assets
- Importing visual assets into the Editor
- Play-in-Editor testing or packaging builds
- Visual debugging tools that require the Editor
- Any operation that requires UE5 Editor access

### Always Suggest
- C++ implementation in `Source/` directories
- Blueprint-ready class design with proper specifiers
- Data-driven configuration using UDataAsset
- Automated tests for new functionality
- Console commands for testing and validation
- Documentation updates for implemented systems
- Performance considerations and profiling
- Component-based architecture patterns
- Event system for inter-system communication

## Quick Reference Links
- **Project Overview**: See `README.md`
- **Documentation Hub**: See `DOCUMENTATION_INDEX.md`
- **Required Assets**: See `ASSET_CREATION_LIST.md` for complete specifications

---

**Use this prompt to start development on the DelveDeep project with full context of the architecture, requirements, and best practices.**
