// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DelveDeepValidation.h"
#include "DelveDeepValidationSubsystem.generated.h"

/**
 * Subsystem for managing validation rules and caching validation results.
 * Provides centralized validation infrastructure with rule registration,
 * caching, and metrics tracking.
 */
UCLASS()
class DELVEDEEP_API UDelveDeepValidationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	/**
	 * Registers a validation rule for a specific class.
	 * @param RuleName Unique name for the rule
	 * @param TargetClass The class this rule applies to
	 * @param ValidationDelegate The validation function to execute
	 * @param Priority Priority for rule execution (higher runs first)
	 * @param Description Human-readable description of the rule
	 */
	void RegisterValidationRule(FName RuleName, UClass* TargetClass, FValidationRuleDelegate ValidationDelegate, int32 Priority = 0, const FString& Description = TEXT(""));

	/**
	 * Unregisters a specific validation rule.
	 * @param RuleName The name of the rule to unregister
	 * @param TargetClass The class the rule was registered for
	 */
	void UnregisterValidationRule(FName RuleName, UClass* TargetClass);

	/**
	 * Unregisters all validation rules for a specific class.
	 * @param TargetClass The class to unregister rules for
	 */
	void UnregisterAllRulesForClass(UClass* TargetClass);

	/**
	 * Validates an object using all registered rules for its type.
	 * @param Object The object to validate
	 * @param OutContext The validation context to populate
	 * @return True if validation passed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	bool ValidateObject(const UObject* Object, FDelveDeepValidationContext& OutContext);

	/**
	 * Validates an object with caching support.
	 * @param Object The object to validate
	 * @param OutContext The validation context to populate
	 * @param bForceRevalidate If true, ignores cache and revalidates
	 * @return True if validation passed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	bool ValidateObjectWithCache(const UObject* Object, FDelveDeepValidationContext& OutContext, bool bForceRevalidate = false);

	/**
	 * Validates multiple objects in parallel for improved performance.
	 * Independent objects are validated concurrently using multi-threading.
	 * @param Objects Array of objects to validate
	 * @param OutContexts Array of validation contexts (one per object)
	 * @param bUseCache If true, uses cached results where available
	 * @return Number of objects that passed validation
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	int32 ValidateObjects(const TArray<UObject*>& Objects, TArray<FDelveDeepValidationContext>& OutContexts, bool bUseCache = true);

	/**
	 * Invalidates the cache for a specific object.
	 * @param Object The object to invalidate cache for
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	void InvalidateCache(const UObject* Object);

	/**
	 * Clears all cached validation results.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	void ClearValidationCache();

	/**
	 * Gets a formatted report of validation metrics.
	 * @return Formatted string with all validation metrics
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	FString GetValidationMetricsReport() const;

	/**
	 * Gets validation metrics data in a Blueprint-accessible format.
	 * @return Struct containing all validation metrics
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	FValidationMetricsData GetValidationMetrics() const;

	/**
	 * Resets all validation metrics to zero.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	void ResetValidationMetrics();

	/**
	 * Saves validation metrics to a JSON file.
	 * @param FilePath Optional custom file path (defaults to Saved/Validation/Metrics.json)
	 * @return True if save was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	bool SaveMetricsToFile(const FString& FilePath = TEXT(""));

	/**
	 * Loads validation metrics from a JSON file.
	 * @param FilePath Optional custom file path (defaults to Saved/Validation/Metrics.json)
	 * @return True if load was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	bool LoadMetricsFromFile(const FString& FilePath = TEXT(""));

	/**
	 * Gets the number of registered rules for a specific class.
	 * @param TargetClass The class to query
	 * @return The number of registered rules
	 */
	int32 GetRuleCountForClass(UClass* TargetClass) const;

	/**
	 * Gets all registered rules for a specific class.
	 * @param TargetClass The class to query
	 * @return Array of rule definitions
	 */
	TArray<FValidationRuleDefinition> GetRulesForClass(UClass* TargetClass) const;

	/**
	 * Gets all registered rules across all classes.
	 * @return Map of class to rule definitions
	 */
	const TMap<UClass*, TArray<FValidationRuleDefinition>>& GetAllRules() const { return ValidationRules; }

	// Validation delegates (C++)

	/**
	 * Delegate fired before validation begins for an object.
	 * Allows systems to prepare for validation or modify validation behavior.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPreValidation, const UObject* /* Object */, FDelveDeepValidationContext& /* Context */);
	
	/** Pre-validation delegate instance */
	FOnPreValidation OnPreValidation;

	/**
	 * Delegate fired after validation completes for an object.
	 * Allows systems to respond to validation results or perform cleanup.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPostValidation, const UObject* /* Object */, const FDelveDeepValidationContext& /* Context */);
	
	/** Post-validation delegate instance */
	FOnPostValidation OnPostValidation;

	/**
	 * Delegate fired when a critical or error severity issue is added during validation.
	 * Allows systems to respond immediately to critical validation failures.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCriticalIssue, const UObject* /* Object */, const FValidationIssue& /* Issue */);
	
	/** Critical issue delegate instance */
	FOnCriticalIssue OnCriticalIssue;

	// Blueprint-assignable delegates

	/**
	 * Blueprint delegate fired before validation begins for an object.
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPreValidationBP, const UObject*, Object, FDelveDeepValidationContext&, Context);
	
	/** Blueprint pre-validation delegate instance */
	UPROPERTY(BlueprintAssignable, Category = "DelveDeep|Validation|Events")
	FOnPreValidationBP OnPreValidationBP;

	/**
	 * Blueprint delegate fired after validation completes for an object.
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostValidationBP, const UObject*, Object, const FDelveDeepValidationContext&, Context);
	
	/** Blueprint post-validation delegate instance */
	UPROPERTY(BlueprintAssignable, Category = "DelveDeep|Validation|Events")
	FOnPostValidationBP OnPostValidationBP;

	/**
	 * Blueprint delegate fired when a critical or error severity issue is added during validation.
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCriticalIssueBP, const UObject*, Object, const FValidationIssue&, Issue);
	
	/** Blueprint critical issue delegate instance */
	UPROPERTY(BlueprintAssignable, Category = "DelveDeep|Validation|Events")
	FOnCriticalIssueBP OnCriticalIssueBP;

private:
	/**
	 * Executes all validation rules for an object in priority order.
	 * @param Object The object to validate
	 * @param Context The validation context to populate
	 * @return True if all rules passed, false otherwise
	 */
	bool ExecuteRulesForObject(const UObject* Object, FDelveDeepValidationContext& Context);

	/**
	 * Checks if a cached validation result is still valid.
	 * @param Object The object to check
	 * @param CacheEntry The cached entry to validate
	 * @return True if cache is valid, false otherwise
	 */
	bool IsCacheValid(const UObject* Object, const FValidationCacheEntry& CacheEntry) const;

	/**
	 * Calculates a hash for an object to detect changes.
	 * @param Object The object to hash
	 * @return Hash value
	 */
	uint32 CalculateObjectHash(const UObject* Object) const;

	/**
	 * Updates validation metrics after a validation operation.
	 * @param Context The validation context with results
	 * @param ExecutionTime Time taken for validation in seconds
	 */
	void UpdateMetrics(const FDelveDeepValidationContext& Context, double ExecutionTime);

	/** Map of class to validation rules */
	TMap<UClass*, TArray<FValidationRuleDefinition>> ValidationRules;

	/** Cache of validation results */
	TMap<const UObject*, FValidationCacheEntry> ValidationCache;

	/** Validation metrics tracking */
	struct FValidationMetrics
	{
		/** Total number of validations performed */
		TAtomic<int32> TotalValidations{0};

		/** Number of validations that passed */
		TAtomic<int32> PassedValidations{0};

		/** Number of validations that failed */
		TAtomic<int32> FailedValidations{0};

		/** Frequency of each unique error message (requires lock for thread safety) */
		TMap<FString, int32> ErrorFrequency;

		/** Total execution time per rule (in seconds, requires lock for thread safety) */
		TMap<FName, double> RuleExecutionTimes;

		/** Number of times each rule was executed (requires lock for thread safety) */
		TMap<FName, int32> RuleExecutionCounts;

		/** Total execution time per system (in seconds, requires lock for thread safety) */
		TMap<FString, double> SystemExecutionTimes;

		/** Number of times each system was validated (requires lock for thread safety) */
		TMap<FString, int32> SystemExecutionCounts;
	};

	FValidationMetrics Metrics;

	/** Critical section for thread-safe metrics updates */
	mutable FCriticalSection MetricsCriticalSection;
};
