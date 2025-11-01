// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DelveDeepValidation.h"
#include "DelveDeepPerformanceBudget.generated.h"

/**
 * Performance budget configuration for a single system
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FPerformanceBudgetEntry
{
	GENERATED_BODY()

	/** Name of the system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	FName SystemName;

	/** Performance budget in milliseconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance",
		meta = (ClampMin = "0.1", ClampMax = "10.0", ToolTip = "Performance budget in milliseconds"))
	float BudgetMilliseconds = 2.0f;

	/** Whether to enforce strict budget (log errors instead of warnings) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance",
		meta = (ToolTip = "If true, budget violations will be logged as errors"))
	bool bEnforceStrict = false;

	/** Color for visualization in performance overlay */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	FLinearColor VisualizationColor = FLinearColor::White;

	FPerformanceBudgetEntry()
		: SystemName(NAME_None)
		, BudgetMilliseconds(2.0f)
		, bEnforceStrict(false)
		, VisualizationColor(FLinearColor::White)
	{
	}

	FPerformanceBudgetEntry(FName InSystemName, float InBudgetMs)
		: SystemName(InSystemName)
		, BudgetMilliseconds(InBudgetMs)
		, bEnforceStrict(false)
		, VisualizationColor(FLinearColor::White)
	{
	}

	/**
	 * Validate budget entry
	 * @param Context Validation context for error reporting
	 * @return True if valid
	 */
	bool Validate(FDelveDeepValidationContext& Context) const;
};

/**
 * Performance Budget Configuration Data Asset
 * 
 * Defines performance budgets for game systems. Used by the telemetry
 * subsystem to monitor and enforce performance targets.
 * 
 * Example budgets:
 * - Combat System: 2.0ms
 * - AI System: 2.0ms
 * - World System: 1.5ms
 * - UI System: 1.0ms
 * - Event System: 0.5ms
 */
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepPerformanceBudget : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Display name for this budget configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText BudgetName;

	/** Description of this budget configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	/** Performance budget entries for each system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budgets")
	TArray<FPerformanceBudgetEntry> SystemBudgets;

	/** Total frame budget in milliseconds (target: 16.67ms for 60 FPS) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budgets",
		meta = (ClampMin = "10.0", ClampMax = "33.33", ToolTip = "Total frame budget in milliseconds"))
	float TotalFrameBudgetMs = 16.67f;

	/** Whether to enable automatic budget enforcement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budgets")
	bool bEnableAutomaticEnforcement = true;

	/**
	 * Get budget for a specific system
	 * @param SystemName Name of the system
	 * @return Budget entry, or nullptr if not found
	 */
	const FPerformanceBudgetEntry* GetSystemBudget(FName SystemName) const;

	/**
	 * Validate budget configuration
	 * @param Context Validation context for error reporting
	 * @return True if valid
	 */
	bool Validate(FDelveDeepValidationContext& Context) const;

	// UObject interface
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
