// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepStats.h"

// Define cycle stats - Main
DEFINE_STAT(STAT_DelveDeep_FrameTotal);
DEFINE_STAT(STAT_DelveDeep_TelemetrySystem);

// Define cycle stats - Combat
DEFINE_STAT(STAT_DelveDeep_CombatSystem);
DEFINE_STAT(STAT_DelveDeep_DamageCalculation);
DEFINE_STAT(STAT_DelveDeep_TargetingSystem);

// Define cycle stats - AI
DEFINE_STAT(STAT_DelveDeep_AISystem);
DEFINE_STAT(STAT_DelveDeep_BehaviorTree);
DEFINE_STAT(STAT_DelveDeep_Pathfinding);

// Define cycle stats - World
DEFINE_STAT(STAT_DelveDeep_WorldSystem);
DEFINE_STAT(STAT_DelveDeep_ProceduralGeneration);
DEFINE_STAT(STAT_DelveDeep_CollisionDetection);

// Define cycle stats - UI
DEFINE_STAT(STAT_DelveDeep_UISystem);
DEFINE_STAT(STAT_DelveDeep_HUDUpdate);
DEFINE_STAT(STAT_DelveDeep_MenuRendering);

// Define cycle stats - Events
DEFINE_STAT(STAT_DelveDeep_EventSystem);
DEFINE_STAT(STAT_DelveDeep_EventBroadcast);
DEFINE_STAT(STAT_DelveDeep_EventProcessing);

// Define cycle stats - Config
DEFINE_STAT(STAT_DelveDeep_ConfigSystem);
DEFINE_STAT(STAT_DelveDeep_DataAssetQuery);
DEFINE_STAT(STAT_DelveDeep_Validation);

// Define memory stats
DEFINE_STAT(STAT_DelveDeep_TotalMemory);
DEFINE_STAT(STAT_DelveDeep_CombatMemory);
DEFINE_STAT(STAT_DelveDeep_AIMemory);
DEFINE_STAT(STAT_DelveDeep_WorldMemory);
DEFINE_STAT(STAT_DelveDeep_UIMemory);
DEFINE_STAT(STAT_DelveDeep_EventMemory);
DEFINE_STAT(STAT_DelveDeep_ConfigMemory);

// Define dword stats - Entity counts
DEFINE_STAT(STAT_DelveDeep_ActiveMonsters);
DEFINE_STAT(STAT_DelveDeep_ActiveProjectiles);
DEFINE_STAT(STAT_DelveDeep_ActiveParticles);
DEFINE_STAT(STAT_DelveDeep_ActivePickups);

// Define dword stats - Event system
DEFINE_STAT(STAT_DelveDeep_EventsPerFrame);
DEFINE_STAT(STAT_DelveDeep_ActiveListeners);
DEFINE_STAT(STAT_DelveDeep_DeferredEvents);

// Define dword stats - Configuration
DEFINE_STAT(STAT_DelveDeep_CacheHits);
DEFINE_STAT(STAT_DelveDeep_CacheMisses);
DEFINE_STAT(STAT_DelveDeep_LoadedAssets);

// Define CSV profiler category
CSV_DEFINE_CATEGORY(DelveDeep, true);
