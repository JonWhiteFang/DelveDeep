// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DelveDeepValidation.h"
#include "DelveDeepMonsterConfig.generated.h"

/**
 * Data table row structure for monster configuration.
 * Stores all monster stats, AI behavior parameters, and reward information.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepMonsterConfig : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Display information
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText MonsterName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	// Base stats
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0"))
	float Health = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float Damage = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float MoveSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float Armor = 0.0f;

	// AI behavior
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "100.0"))
	float DetectionRange = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "10.0"))
	float AttackRange = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	FName AIBehaviorType = "Melee";

	// Loot and rewards
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = "0"))
	int32 CoinDropMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = "0"))
	int32 CoinDropMax = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = "0"))
	int32 ExperienceReward = 10;

	/**
	 * Called after data is imported from CSV or other sources.
	 * Performs validation on the imported data.
	 * 
	 * @param InDataTable The data table this row belongs to
	 * @param InRowName The name of this row
	 * @param OutCollectedImportProblems Array to collect any import problems
	 */
	virtual void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, 
		TArray<FString>& OutCollectedImportProblems) override;

	/**
	 * Validates the monster configuration data.
	 * 
	 * @param Context The validation context for tracking errors and warnings
	 * @return True if validation passed (no errors), false otherwise
	 */
	bool Validate(FValidationContext& Context) const;
};
