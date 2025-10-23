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
	bool ValidateObject(const UObject* Object, FValidationContext& OutContext);

	/**
	 * Validates an object with caching support.
	 * @param Object The object to validate
	 * @param OutContext The validation context to populate
	 * @param bForceRevalidate If true, ignores cache and revalidates
	 * @return True if validation passed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Validation")
	bool ValidateObjectWithCache(const UObject* Object, FValidationContext& OutContext, bool bForceRevalidate = false);

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

private:
	/**
	 * Executes all validation rules for an object in priority order.
	 * @param Object The object to validate
	 * @param Context The validation context to populate
	 * @return True if all rules passed, false otherwise
	 */
	bool ExecuteRulesForObject(const UObject* Object, FValidationContext& Context);

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

	/** Map of class to validation rules */
	TMap<UClass*, TArray<FValidationRuleDefinition>> ValidationRules;

	/** Cache of validation results */
	TMap<const UObject*, FValidationCacheEntry> ValidationCache;
};
