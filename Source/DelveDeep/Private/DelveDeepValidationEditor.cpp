// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidationEditor.h"

#if WITH_EDITOR

#include "DelveDeepValidationSubsystem.h"
#include "DelveDeepValidation.h"
#include "Editor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"

#define LOCTEXT_NAMESPACE "DelveDeepValidation"

FDelegateHandle FDelveDeepValidationEditor::PreSaveHandle;
FDelegateHandle FDelveDeepValidationEditor::PostSaveHandle;

void FDelveDeepValidationEditor::Initialize()
{
	if (!GEditor)
	{
		return;
	}

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Initializing Editor validation hooks"));

	// Register asset save hooks
	PreSaveHandle = FCoreUObjectDelegates::OnObjectPreSave.AddStatic(&FDelveDeepValidationEditor::OnAssetPreSave);
	PostSaveHandle = FCoreUObjectDelegates::OnObjectSaved.AddStatic(&FDelveDeepValidationEditor::OnAssetPostSave);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Editor validation hooks initialized"));
}

void FDelveDeepValidationEditor::Shutdown()
{
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Shutting down Editor validation hooks"));

	// Unregister asset save hooks
	if (PreSaveHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPreSave.Remove(PreSaveHandle);
		PreSaveHandle.Reset();
	}

	if (PostSaveHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectSaved.Remove(PostSaveHandle);
		PostSaveHandle.Reset();
	}

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Editor validation hooks shut down"));
}

void FDelveDeepValidationEditor::OnAssetPreSave(UObject* Asset)
{
	if (!Asset || !IsValid(Asset))
	{
		return;
	}

	// Get validation subsystem
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World || !World->GetGameInstance())
	{
		return;
	}

	UDelveDeepValidationSubsystem* ValidationSubsystem = World->GetGameInstance()->GetSubsystem<UDelveDeepValidationSubsystem>();
	if (!ValidationSubsystem)
	{
		return;
	}

	// Validate the asset
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("EditorValidation");
	Context.OperationName = TEXT("PreSave");

	bool bResult = ValidationSubsystem->ValidateObject(Asset, Context);

	// Log results to Message Log
	FString AssetPath = Asset->GetPathName();
	LogValidationResults(Context, AssetPath);

	// If validation failed with critical or error issues, log warning
	if (!bResult)
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("Asset validation failed for: %s"), *AssetPath);
		
		// Note: We don't prevent saving here, just warn
		// To prevent saving, you would need to integrate with the Data Validation Plugin
	}
}

void FDelveDeepValidationEditor::OnAssetPostSave(UObject* Asset)
{
	if (!Asset || !IsValid(Asset))
	{
		return;
	}

	FString AssetPath = Asset->GetPathName();
	UE_LOG(LogDelveDeepConfig, Verbose, TEXT("Asset saved: %s"), *AssetPath);
}

void FDelveDeepValidationEditor::ValidateAssetByPath(const FString& AssetPath)
{
	// Load the asset
	UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
	if (!Asset)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to load asset: %s"), *AssetPath);
		return;
	}

	// Get validation subsystem
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World || !World->GetGameInstance())
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to get game instance"));
		return;
	}

	UDelveDeepValidationSubsystem* ValidationSubsystem = World->GetGameInstance()->GetSubsystem<UDelveDeepValidationSubsystem>();
	if (!ValidationSubsystem)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to get validation subsystem"));
		return;
	}

	// Validate the asset
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("EditorValidation");
	Context.OperationName = TEXT("ValidateAsset");

	bool bResult = ValidationSubsystem->ValidateObject(Asset, Context);

	// Log results
	LogValidationResults(Context, AssetPath);

	if (bResult)
	{
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Asset validation PASSED: %s"), *AssetPath);
	}
	else
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Asset validation FAILED: %s"), *AssetPath);
	}
}

void FDelveDeepValidationEditor::ValidateAssetsInDirectory(const FString& DirectoryPath)
{
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validating assets in directory: %s"), *DirectoryPath);

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Get all assets in directory
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByPath(FName(*DirectoryPath), AssetDataList, true);

	if (AssetDataList.Num() == 0)
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("No assets found in directory: %s"), *DirectoryPath);
		return;
	}

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Found %d assets to validate"), AssetDataList.Num());

	// Get validation subsystem
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World || !World->GetGameInstance())
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to get game instance"));
		return;
	}

	UDelveDeepValidationSubsystem* ValidationSubsystem = World->GetGameInstance()->GetSubsystem<UDelveDeepValidationSubsystem>();
	if (!ValidationSubsystem)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to get validation subsystem"));
		return;
	}

	// Validate each asset
	int32 PassedCount = 0;
	int32 FailedCount = 0;

	for (const FAssetData& AssetData : AssetDataList)
	{
		UObject* Asset = AssetData.GetAsset();
		if (!Asset)
		{
			continue;
		}

		FDelveDeepValidationContext Context;
		Context.SystemName = TEXT("EditorValidation");
		Context.OperationName = TEXT("ValidateDirectory");

		bool bResult = ValidationSubsystem->ValidateObject(Asset, Context);

		if (bResult)
		{
			PassedCount++;
		}
		else
		{
			FailedCount++;
			LogValidationResults(Context, AssetData.ObjectPath.ToString());
		}
	}

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Directory validation complete: %d passed, %d failed"), PassedCount, FailedCount);
}

void FDelveDeepValidationEditor::ValidateAllAssets()
{
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validating all assets in project..."));

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Get all assets
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAllAssets(AssetDataList);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Found %d assets to validate"), AssetDataList.Num());

	// Get validation subsystem
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World || !World->GetGameInstance())
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to get game instance"));
		return;
	}

	UDelveDeepValidationSubsystem* ValidationSubsystem = World->GetGameInstance()->GetSubsystem<UDelveDeepValidationSubsystem>();
	if (!ValidationSubsystem)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to get validation subsystem"));
		return;
	}

	// Validate each asset
	int32 PassedCount = 0;
	int32 FailedCount = 0;
	int32 SkippedCount = 0;

	for (const FAssetData& AssetData : AssetDataList)
	{
		// Skip engine content
		if (AssetData.PackageName.ToString().StartsWith(TEXT("/Engine/")))
		{
			SkippedCount++;
			continue;
		}

		UObject* Asset = AssetData.GetAsset();
		if (!Asset)
		{
			SkippedCount++;
			continue;
		}

		FDelveDeepValidationContext Context;
		Context.SystemName = TEXT("EditorValidation");
		Context.OperationName = TEXT("ValidateAll");

		bool bResult = ValidationSubsystem->ValidateObject(Asset, Context);

		if (bResult)
		{
			PassedCount++;
		}
		else
		{
			FailedCount++;
			LogValidationResults(Context, AssetData.ObjectPath.ToString());
		}
	}

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Project validation complete: %d passed, %d failed, %d skipped"), 
		PassedCount, FailedCount, SkippedCount);
}

void FDelveDeepValidationEditor::LogValidationResults(const FDelveDeepValidationContext& Context, const FString& AssetPath)
{
	// Create Message Log category if it doesn't exist
	FMessageLog ValidationLog("DelveDeepValidation");

	// Add asset reference
	ValidationLog.Info(FText::FromString(FString::Printf(TEXT("Validation results for: %s"), *AssetPath)));

	// Log critical issues
	for (const FValidationIssue& Issue : Context.Issues)
	{
		if (Issue.Severity == EValidationSeverity::Critical)
		{
			ValidationLog.Error(FText::FromString(Issue.Message));
		}
	}

	// Log errors
	for (const FValidationIssue& Issue : Context.Issues)
	{
		if (Issue.Severity == EValidationSeverity::Error)
		{
			ValidationLog.Error(FText::FromString(Issue.Message));
		}
	}

	// Log warnings
	for (const FValidationIssue& Issue : Context.Issues)
	{
		if (Issue.Severity == EValidationSeverity::Warning)
		{
			ValidationLog.Warning(FText::FromString(Issue.Message));
		}
	}

	// Log info messages
	for (const FValidationIssue& Issue : Context.Issues)
	{
		if (Issue.Severity == EValidationSeverity::Info)
		{
			ValidationLog.Info(FText::FromString(Issue.Message));
		}
	}

	// Notify the Message Log
	ValidationLog.Notify();
}

#undef LOCTEXT_NAMESPACE

#endif // WITH_EDITOR
