// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidationSubsystem.h"
#include "UObject/UObjectHash.h"
#include "Serialization/ArchiveCrc32.h"

void UDelveDeepValidationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation Subsystem initializing..."));
	
	// Initialize validation cache
	ValidationCache.Empty();
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation Subsystem initialized"));
}

void UDelveDeepValidationSubsystem::Deinitialize()
{
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation Subsystem shutting down..."));
	
	// Clear all validation rules
	ValidationRules.Empty();
	
	// Clear validation cache
	ValidationCache.Empty();
	
	Super::Deinitialize();
}

void UDelveDeepValidationSubsystem::RegisterValidationRule(FName RuleName, UClass* TargetClass, 
	FValidationRuleDelegate ValidationDelegate, int32 Priority, const FString& Description)
{
	if (!TargetClass)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Cannot register validation rule '%s': TargetClass is null"), *RuleName.ToString());
		return;
	}
	
	if (!ValidationDelegate.IsBound())
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Cannot register validation rule '%s': ValidationDelegate is not bound"), *RuleName.ToString());
		return;
	}
	
	// Get or create rule array for this class
	TArray<FValidationRuleDefinition>& Rules = ValidationRules.FindOrAdd(TargetClass);
	
	// Check if rule already exists
	for (const FValidationRuleDefinition& ExistingRule : Rules)
	{
		if (ExistingRule.RuleName == RuleName)
		{
			UE_LOG(LogDelveDeepConfig, Warning, TEXT("Validation rule '%s' already registered for class '%s', skipping"), 
				*RuleName.ToString(), *TargetClass->GetName());
			return;
		}
	}
	
	// Add new rule
	FValidationRuleDefinition NewRule(RuleName, TargetClass, ValidationDelegate, Priority, Description);
	Rules.Add(NewRule);
	
	// Sort rules by priority (higher priority first)
	Rules.Sort();
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Registered validation rule '%s' for class '%s' (Priority: %d)"), 
		*RuleName.ToString(), *TargetClass->GetName(), Priority);
}

void UDelveDeepValidationSubsystem::UnregisterValidationRule(FName RuleName, UClass* TargetClass)
{
	if (!TargetClass)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Cannot unregister validation rule '%s': TargetClass is null"), *RuleName.ToString());
		return;
	}
	
	TArray<FValidationRuleDefinition>* Rules = ValidationRules.Find(TargetClass);
	if (!Rules)
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("No validation rules registered for class '%s'"), *TargetClass->GetName());
		return;
	}
	
	int32 RemovedCount = Rules->RemoveAll([RuleName](const FValidationRuleDefinition& Rule)
	{
		return Rule.RuleName == RuleName;
	});
	
	if (RemovedCount > 0)
	{
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Unregistered validation rule '%s' from class '%s'"), 
			*RuleName.ToString(), *TargetClass->GetName());
		
		// Remove class entry if no rules remain
		if (Rules->Num() == 0)
		{
			ValidationRules.Remove(TargetClass);
		}
	}
	else
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("Validation rule '%s' not found for class '%s'"), 
			*RuleName.ToString(), *TargetClass->GetName());
	}
}

void UDelveDeepValidationSubsystem::UnregisterAllRulesForClass(UClass* TargetClass)
{
	if (!TargetClass)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Cannot unregister rules: TargetClass is null"));
		return;
	}
	
	int32 RemovedCount = ValidationRules.Remove(TargetClass);
	
	if (RemovedCount > 0)
	{
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Unregistered all validation rules for class '%s'"), *TargetClass->GetName());
	}
	else
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("No validation rules registered for class '%s'"), *TargetClass->GetName());
	}
}

bool UDelveDeepValidationSubsystem::ValidateObject(const UObject* Object, FValidationContext& OutContext)
{
	if (!Object || !IsValid(Object))
	{
		OutContext.AddError(TEXT("Cannot validate null or invalid object"));
		return false;
	}
	
	// Set context information
	OutContext.SystemName = TEXT("ValidationSubsystem");
	OutContext.OperationName = FString::Printf(TEXT("ValidateObject(%s)"), *Object->GetClass()->GetName());
	OutContext.CreationTime = FDateTime::Now();
	
	// Execute validation rules
	bool bResult = ExecuteRulesForObject(Object, OutContext);
	
	// Mark completion time
	OutContext.CompletionTime = FDateTime::Now();
	
	return bResult;
}

bool UDelveDeepValidationSubsystem::ValidateObjectWithCache(const UObject* Object, FValidationContext& OutContext, bool bForceRevalidate)
{
	if (!Object || !IsValid(Object))
	{
		OutContext.AddError(TEXT("Cannot validate null or invalid object"));
		return false;
	}
	
	// Check cache if not forcing revalidation
	if (!bForceRevalidate)
	{
		const FValidationCacheEntry* CacheEntry = ValidationCache.Find(Object);
		if (CacheEntry && IsCacheValid(Object, *CacheEntry))
		{
			// Return cached result
			OutContext = CacheEntry->Context;
			UE_LOG(LogDelveDeepConfig, Verbose, TEXT("Validation cache hit for object '%s'"), *Object->GetName());
			return OutContext.IsValid();
		}
	}
	
	// Perform validation
	bool bResult = ValidateObject(Object, OutContext);
	
	// Cache the result
	FValidationCacheEntry& CacheEntry = ValidationCache.FindOrAdd(Object);
	CacheEntry.Context = OutContext;
	CacheEntry.Timestamp = FDateTime::Now();
	CacheEntry.AssetHash = CalculateObjectHash(Object);
	
	UE_LOG(LogDelveDeepConfig, Verbose, TEXT("Validation result cached for object '%s'"), *Object->GetName());
	
	return bResult;
}

void UDelveDeepValidationSubsystem::InvalidateCache(const UObject* Object)
{
	if (!Object)
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("Cannot invalidate cache: Object is null"));
		return;
	}
	
	int32 RemovedCount = ValidationCache.Remove(Object);
	
	if (RemovedCount > 0)
	{
		UE_LOG(LogDelveDeepConfig, Verbose, TEXT("Invalidated validation cache for object '%s'"), *Object->GetName());
	}
}

void UDelveDeepValidationSubsystem::ClearValidationCache()
{
	int32 ClearedCount = ValidationCache.Num();
	ValidationCache.Empty();
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Cleared validation cache (%d entries)"), ClearedCount);
}

int32 UDelveDeepValidationSubsystem::GetRuleCountForClass(UClass* TargetClass) const
{
	if (!TargetClass)
	{
		return 0;
	}
	
	const TArray<FValidationRuleDefinition>* Rules = ValidationRules.Find(TargetClass);
	return Rules ? Rules->Num() : 0;
}

TArray<FValidationRuleDefinition> UDelveDeepValidationSubsystem::GetRulesForClass(UClass* TargetClass) const
{
	if (!TargetClass)
	{
		return TArray<FValidationRuleDefinition>();
	}
	
	const TArray<FValidationRuleDefinition>* Rules = ValidationRules.Find(TargetClass);
	return Rules ? *Rules : TArray<FValidationRuleDefinition>();
}

bool UDelveDeepValidationSubsystem::ExecuteRulesForObject(const UObject* Object, FValidationContext& Context)
{
	if (!Object)
	{
		return false;
	}
	
	UClass* ObjectClass = Object->GetClass();
	
	// Find rules for this class and all parent classes
	TArray<FValidationRuleDefinition> ApplicableRules;
	
	for (const auto& Pair : ValidationRules)
	{
		UClass* RuleClass = Pair.Key;
		if (ObjectClass->IsChildOf(RuleClass))
		{
			ApplicableRules.Append(Pair.Value);
		}
	}
	
	// Sort all applicable rules by priority
	ApplicableRules.Sort();
	
	if (ApplicableRules.Num() == 0)
	{
		UE_LOG(LogDelveDeepConfig, Verbose, TEXT("No validation rules found for class '%s'"), *ObjectClass->GetName());
		return true; // No rules means validation passes
	}
	
	UE_LOG(LogDelveDeepConfig, Verbose, TEXT("Executing %d validation rules for object '%s'"), 
		ApplicableRules.Num(), *Object->GetName());
	
	bool bAllRulesPassed = true;
	
	// Execute each rule
	for (const FValidationRuleDefinition& Rule : ApplicableRules)
	{
		if (Rule.ValidationDelegate.IsBound())
		{
			// Create child context for this rule
			FValidationContext RuleContext;
			RuleContext.SystemName = TEXT("ValidationRule");
			RuleContext.OperationName = Rule.RuleName.ToString();
			RuleContext.CreationTime = FDateTime::Now();
			
			// Execute rule
			bool bRulePassed = Rule.ValidationDelegate.Execute(Object, RuleContext);
			
			RuleContext.CompletionTime = FDateTime::Now();
			
			// Add rule context as child
			Context.AddChildContext(RuleContext);
			
			if (!bRulePassed)
			{
				bAllRulesPassed = false;
				UE_LOG(LogDelveDeepConfig, Verbose, TEXT("Validation rule '%s' failed for object '%s'"), 
					*Rule.RuleName.ToString(), *Object->GetName());
			}
		}
	}
	
	return bAllRulesPassed;
}

bool UDelveDeepValidationSubsystem::IsCacheValid(const UObject* Object, const FValidationCacheEntry& CacheEntry) const
{
	if (!Object)
	{
		return false;
	}
	
	// Calculate current hash
	uint32 CurrentHash = CalculateObjectHash(Object);
	
	// Compare with cached hash
	return CurrentHash == CacheEntry.AssetHash;
}

uint32 UDelveDeepValidationSubsystem::CalculateObjectHash(const UObject* Object) const
{
	if (!Object)
	{
		return 0;
	}
	
	// Use CRC32 archive to calculate hash
	FArchiveCrc32 Ar;
	
	// Serialize object properties to calculate hash
	// Note: This is a simplified hash calculation
	// In production, you might want a more sophisticated approach
	const_cast<UObject*>(Object)->Serialize(Ar);
	
	return Ar.GetCrc();
}
