// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"

/**
 * DelveDeep Performance Statistics
 * 
 * Defines stat groups and counters for comprehensive performance monitoring.
 * Use these stats with the 'stat' console command:
 * 
 * stat DelveDeep              - Show main DelveDeep stats
 * stat DelveDeep.Combat       - Show combat system stats
 * stat DelveDeep.AI           - Show AI system stats
 * stat DelveDeep.World        - Show world system stats
 * stat DelveDeep.UI           - Show UI system stats
 * stat DelveDeep.Events       - Show event system stats
 * stat DelveDeep.Config       - Show configuration system stats
 */

// Main stat group
DECLARE_STATS_GROUP(TEXT("DelveDeep"), STATGROUP_DelveDeep, STATCAT_Advanced);

// System-specific stat groups
DECLARE_STATS_GROUP(TEXT("DelveDeep.Combat"), STATGROUP_DelveDeepCombat, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("DelveDeep.AI"), STATGROUP_DelveDeepAI, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("DelveDeep.World"), STATGROUP_DelveDeepWorld, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("DelveDeep.UI"), STATGROUP_DelveDeepUI, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("DelveDeep.Events"), STATGROUP_DelveDeepEvents, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("DelveDeep.Config"), STATGROUP_DelveDeepConfig, STATCAT_Advanced);

// Cycle counters - Main
DECLARE_CYCLE_STAT_EXTERN(TEXT("Frame Total"), STAT_DelveDeep_FrameTotal, STATGROUP_DelveDeep, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Telemetry System"), STAT_DelveDeep_TelemetrySystem, STATGROUP_DelveDeep, DELVEDEEP_API);

// Cycle counters - Combat
DECLARE_CYCLE_STAT_EXTERN(TEXT("Combat System"), STAT_DelveDeep_CombatSystem, STATGROUP_DelveDeepCombat, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Damage Calculation"), STAT_DelveDeep_DamageCalculation, STATGROUP_DelveDeepCombat, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Targeting System"), STAT_DelveDeep_TargetingSystem, STATGROUP_DelveDeepCombat, DELVEDEEP_API);

// Cycle counters - AI
DECLARE_CYCLE_STAT_EXTERN(TEXT("AI System"), STAT_DelveDeep_AISystem, STATGROUP_DelveDeepAI, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Behavior Tree"), STAT_DelveDeep_BehaviorTree, STATGROUP_DelveDeepAI, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Pathfinding"), STAT_DelveDeep_Pathfinding, STATGROUP_DelveDeepAI, DELVEDEEP_API);

// Cycle counters - World
DECLARE_CYCLE_STAT_EXTERN(TEXT("World System"), STAT_DelveDeep_WorldSystem, STATGROUP_DelveDeepWorld, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Procedural Generation"), STAT_DelveDeep_ProceduralGeneration, STATGROUP_DelveDeepWorld, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Collision Detection"), STAT_DelveDeep_CollisionDetection, STATGROUP_DelveDeepWorld, DELVEDEEP_API);

// Cycle counters - UI
DECLARE_CYCLE_STAT_EXTERN(TEXT("UI System"), STAT_DelveDeep_UISystem, STATGROUP_DelveDeepUI, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("HUD Update"), STAT_DelveDeep_HUDUpdate, STATGROUP_DelveDeepUI, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Menu Rendering"), STAT_DelveDeep_MenuRendering, STATGROUP_DelveDeepUI, DELVEDEEP_API);

// Cycle counters - Events
DECLARE_CYCLE_STAT_EXTERN(TEXT("Event System"), STAT_DelveDeep_EventSystem, STATGROUP_DelveDeepEvents, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Event Broadcast"), STAT_DelveDeep_EventBroadcast, STATGROUP_DelveDeepEvents, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Event Processing"), STAT_DelveDeep_EventProcessing, STATGROUP_DelveDeepEvents, DELVEDEEP_API);

// Cycle counters - Config
DECLARE_CYCLE_STAT_EXTERN(TEXT("Config System"), STAT_DelveDeep_ConfigSystem, STATGROUP_DelveDeepConfig, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Data Asset Query"), STAT_DelveDeep_DataAssetQuery, STATGROUP_DelveDeepConfig, DELVEDEEP_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Validation"), STAT_DelveDeep_Validation, STATGROUP_DelveDeepConfig, DELVEDEEP_API);

// Memory counters
DECLARE_MEMORY_STAT_EXTERN(TEXT("Total Memory"), STAT_DelveDeep_TotalMemory, STATGROUP_DelveDeep, DELVEDEEP_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Combat Memory"), STAT_DelveDeep_CombatMemory, STATGROUP_DelveDeepCombat, DELVEDEEP_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("AI Memory"), STAT_DelveDeep_AIMemory, STATGROUP_DelveDeepAI, DELVEDEEP_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("World Memory"), STAT_DelveDeep_WorldMemory, STATGROUP_DelveDeepWorld, DELVEDEEP_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("UI Memory"), STAT_DelveDeep_UIMemory, STATGROUP_DelveDeepUI, DELVEDEEP_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Event Memory"), STAT_DelveDeep_EventMemory, STATGROUP_DelveDeepEvents, DELVEDEEP_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Config Memory"), STAT_DelveDeep_ConfigMemory, STATGROUP_DelveDeepConfig, DELVEDEEP_API);

// Dword counters - Entity counts
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active Monsters"), STAT_DelveDeep_ActiveMonsters, STATGROUP_DelveDeep, DELVEDEEP_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active Projectiles"), STAT_DelveDeep_ActiveProjectiles, STATGROUP_DelveDeep, DELVEDEEP_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active Particles"), STAT_DelveDeep_ActiveParticles, STATGROUP_DelveDeep, DELVEDEEP_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active Pickups"), STAT_DelveDeep_ActivePickups, STATGROUP_DelveDeep, DELVEDEEP_API);

// Dword counters - Event system
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Events Per Frame"), STAT_DelveDeep_EventsPerFrame, STATGROUP_DelveDeepEvents, DELVEDEEP_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active Listeners"), STAT_DelveDeep_ActiveListeners, STATGROUP_DelveDeepEvents, DELVEDEEP_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Deferred Events"), STAT_DelveDeep_DeferredEvents, STATGROUP_DelveDeepEvents, DELVEDEEP_API);

// Dword counters - Configuration
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Cache Hits"), STAT_DelveDeep_CacheHits, STATGROUP_DelveDeepConfig, DELVEDEEP_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Cache Misses"), STAT_DelveDeep_CacheMisses, STATGROUP_DelveDeepConfig, DELVEDEEP_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Loaded Assets"), STAT_DelveDeep_LoadedAssets, STATGROUP_DelveDeepConfig, DELVEDEEP_API);
