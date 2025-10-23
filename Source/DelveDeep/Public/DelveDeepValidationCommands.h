// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Console commands for testing and debugging the DelveDeep configuration system.
 * 
 * Available commands:
 * - DelveDeep.ValidateAllData: Validates all loaded configuration data and logs the report
 * - DelveDeep.ShowConfigStats: Displays performance statistics for the configuration system
 * - DelveDeep.ListLoadedAssets: Lists all cached configuration assets by type
 * - DelveDeep.ReloadConfigData: Forces a full reload of all configuration data
 * - DelveDeep.DumpConfigData <AssetName>: Dumps all properties of a specified configuration asset
 */

DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepConfig, Log, All);
