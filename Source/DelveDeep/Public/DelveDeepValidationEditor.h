// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR

/**
 * Editor-specific validation functionality.
 * Provides asset save hooks, directory validation, and Message Log integration.
 */
class DELVEDEEP_API FDelveDeepValidationEditor
{
public:
	/** Initializes Editor validation hooks */
	static void Initialize();

	/** Shuts down Editor validation hooks */
	static void Shutdown();

	/** Validates an asset by path */
	static void ValidateAssetByPath(const FString& AssetPath);

	/** Validates all assets in a directory recursively */
	static void ValidateAssetsInDirectory(const FString& DirectoryPath);

	/** Validates all assets in the project */
	static void ValidateAllAssets();

private:
	/** Called before an asset is saved */
	static void OnAssetPreSave(UObject* Asset);

	/** Called after an asset is saved */
	static void OnAssetPostSave(UObject* Asset);

	/** Logs validation results to the Message Log */
	static void LogValidationResults(const class FValidationContext& Context, const FString& AssetPath);

	/** Delegate handle for asset pre-save */
	static FDelegateHandle PreSaveHandle;

	/** Delegate handle for asset post-save */
	static FDelegateHandle PostSaveHandle;
};

#endif // WITH_EDITOR
