# Technical Stack

## Engine & Language

- **Engine**: Unreal Engine 5.6
- **Language**: Modern C++ (C++20)
- **Build System**: Unreal Build Tool (UBT)

## Core Technologies

- **Rendering**: Paper2D for 2D sprite-based graphics
- **Input**: Enhanced Input System (UE5 standard)
- **AI**: Behavior Trees with AI Perception
- **UI**: UMG (Unreal Motion Graphics)
- **Audio**: Unreal Audio Engine with spatial audio support

## Module Structure

```
Source/
├── DelveDeep/          # Main game module (DELVEDEEP_API)
├── DelveDeepCore/      # Shared utilities and base classes
├── DelveDeepUI/        # User interface and HUD systems
└── DelveDeepAudio/     # Audio management and effects
```

## Required Module Dependencies

All modules must include these dependencies in their `.Build.cs` files:

```cpp
PublicDependencyModuleNames.AddRange(new string[] 
{
    "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
    "Paper2D", "UMG", "AIModule", "NavigationSystem", "GameplayTasks",
    "GameplayTags"
});
```

## Common Commands

### Testing & Validation
```bash
DelveDeep.ValidateAllSystems          # Run comprehensive system validation
DelveDeep.TestInputSystem             # Validate input system
DelveDeep.TestCombatSystem            # Validate combat system
DelveDeep.TestSaveSystem              # Test save/load functionality
DelveDeep.ValidateAllData             # Validate configuration data
```

### Performance Monitoring
```bash
stat fps                              # Display frame rate
stat memory                           # Monitor memory usage
stat DelveDeep                        # Show DelveDeep-specific metrics
DelveDeep.ShowPerformanceMetrics      # Detailed performance analysis
```

### Debug Visualization
```bash
DelveDeep.ShowTargetingDebug          # Visualize targeting system
DelveDeep.ShowAIDebug                 # Display AI behavior
DelveDeep.ToggleDebugHUD              # Show/hide debug overlay
```

## Code Quality Tools

- **Formatting**: clang-format with UE5 style guide
- **Static Analysis**: clang-tidy
- **Documentation**: Doxygen
- **Testing**: Google Test or Catch2 for unit tests
