// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidationSubsystem.h"
#include "UObject/UObjectHash.h"
#include "Serialization/ArchiveCrc32.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

void UDelveDeepValidationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation Subsystem initializing..."));
	
	// Initialize validation cache
	ValidationCache.Empty();
	
	// Initialize metrics
	Metrics = FValidationMetrics();
	
	// Load persisted metrics
	LoadMetricsFromFile();
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation Subsystem initialized"));
}

void UDelveDeepValidationSubsystem::Deinitialize()
{
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation Subsystem shutting down..."));
	
	// Save metrics before shutdown
	SaveMetricsToFile();
	
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
	
	// Broadcast pre-validation delegate
	OnPreValidation.Broadcast(Object, OutContext);
	
	// Track start time for metrics
	double StartTime = FPlatformTime::Seconds();
	
	// Execute validation rules
	bool bResult = ExecuteRulesForObject(Object, OutContext);
	
	// Mark completion time
	OutContext.CompletionTime = FDateTime::Now();
	
	// Calculate execution time
	double ExecutionTime = FPlatformTime::Seconds() - StartTime;
	
	// Update metrics
	UpdateMetrics(OutContext, ExecutionTime);
	
	// Check for critical issues and broadcast if found
	for (const FValidationIssue& Issue : OutContext.Issues)
	{
		if (Issue.Severity == EValidationSeverity::Critical || Issue.Severity == EValidationSeverity::Error)
		{
			OnCriticalIssue.Broadcast(Object, Issue);
		}
	}
	
	// Broadcast post-validation delegate
	OnPostValidation.Broadcast(Object, OutContext);
	
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
			
			// Track rule execution time
			double RuleStartTime = FPlatformTime::Seconds();
			
			// Execute rule
			bool bRulePassed = Rule.ValidationDelegate.Execute(Object, RuleContext);
			
			// Calculate rule execution time
			double RuleExecutionTime = FPlatformTime::Seconds() - RuleStartTime;
			
			RuleContext.CompletionTime = FDateTime::Now();
			
			// Track rule execution metrics
			Metrics.RuleExecutionTimes.FindOrAdd(Rule.RuleName) += RuleExecutionTime;
			Metrics.RuleExecutionCounts.FindOrAdd(Rule.RuleName)++;
			
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

void UDelveDeepValidationSubsystem::UpdateMetrics(const FValidationContext& Context, double ExecutionTime)
{
	// Update total validation count
	Metrics.TotalValidations++;
	
	// Update passed/failed counts
	if (Context.IsValid())
	{
		Metrics.PassedValidations++;
	}
	else
	{
		Metrics.FailedValidations++;
	}
	
	// Track error frequency
	for (const FString& Error : Context.ValidationErrors)
	{
		Metrics.ErrorFrequency.FindOrAdd(Error)++;
	}
	
	// Track error frequency from new Issues array
	for (const FValidationIssue& Issue : Context.Issues)
	{
		if (Issue.Severity == EValidationSeverity::Error || Issue.Severity == EValidationSeverity::Critical)
		{
			Metrics.ErrorFrequency.FindOrAdd(Issue.Message)++;
		}
	}
	
	// Track system execution time
	if (!Context.SystemName.IsEmpty())
	{
		Metrics.SystemExecutionTimes.FindOrAdd(Context.SystemName) += ExecutionTime;
		Metrics.SystemExecutionCounts.FindOrAdd(Context.SystemName)++;
	}
	
	// Track child context metrics recursively
	for (const FValidationContext& ChildContext : Context.ChildContexts)
	{
		// Track error frequency from child contexts
		for (const FString& Error : ChildContext.ValidationErrors)
		{
			Metrics.ErrorFrequency.FindOrAdd(Error)++;
		}
		
		for (const FValidationIssue& Issue : ChildContext.Issues)
		{
			if (Issue.Severity == EValidationSeverity::Error || Issue.Severity == EValidationSeverity::Critical)
			{
				Metrics.ErrorFrequency.FindOrAdd(Issue.Message)++;
			}
		}
	}
}

FString UDelveDeepValidationSubsystem::GetValidationMetricsReport() const
{
	FString Report;
	Report += TEXT("=== Validation Metrics Report ===\n\n");
	
	// Overall statistics
	Report += FString::Printf(TEXT("Total Validations: %d\n"), Metrics.TotalValidations);
	Report += FString::Printf(TEXT("Passed: %d (%.1f%%)\n"), 
		Metrics.PassedValidations, 
		Metrics.TotalValidations > 0 ? (Metrics.PassedValidations * 100.0f / Metrics.TotalValidations) : 0.0f);
	Report += FString::Printf(TEXT("Failed: %d (%.1f%%)\n\n"), 
		Metrics.FailedValidations,
		Metrics.TotalValidations > 0 ? (Metrics.FailedValidations * 100.0f / Metrics.TotalValidations) : 0.0f);
	
	// Error frequency
	if (Metrics.ErrorFrequency.Num() > 0)
	{
		Report += TEXT("=== Most Common Errors ===\n");
		
		// Sort errors by frequency
		TArray<TPair<FString, int32>> SortedErrors;
		for (const auto& Pair : Metrics.ErrorFrequency)
		{
			SortedErrors.Add(TPair<FString, int32>(Pair.Key, Pair.Value));
		}
		SortedErrors.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B)
		{
			return A.Value > B.Value;
		});
		
		// Show top 10 errors
		int32 Count = 0;
		for (const auto& Pair : SortedErrors)
		{
			Report += FString::Printf(TEXT("  %d: %s\n"), Pair.Value, *Pair.Key);
			if (++Count >= 10)
				break;
		}
		Report += TEXT("\n");
	}
	
	// Rule execution times
	if (Metrics.RuleExecutionTimes.Num() > 0)
	{
		Report += TEXT("=== Rule Performance ===\n");
		
		// Calculate average times and sort
		TArray<TPair<FName, double>> SortedRules;
		for (const auto& Pair : Metrics.RuleExecutionTimes)
		{
			int32 ExecutionCount = Metrics.RuleExecutionCounts.FindRef(Pair.Key);
			double AvgTime = ExecutionCount > 0 ? (Pair.Value / ExecutionCount) : 0.0;
			SortedRules.Add(TPair<FName, double>(Pair.Key, AvgTime));
		}
		SortedRules.Sort([](const TPair<FName, double>& A, const TPair<FName, double>& B)
		{
			return A.Value > B.Value;
		});
		
		// Show top 10 slowest rules
		int32 Count = 0;
		for (const auto& Pair : SortedRules)
		{
			int32 ExecutionCount = Metrics.RuleExecutionCounts.FindRef(Pair.Key);
			Report += FString::Printf(TEXT("  %s: %.3f ms avg (%d executions)\n"), 
				*Pair.Key.ToString(), Pair.Value * 1000.0, ExecutionCount);
			if (++Count >= 10)
				break;
		}
		Report += TEXT("\n");
	}
	
	// System execution times
	if (Metrics.SystemExecutionTimes.Num() > 0)
	{
		Report += TEXT("=== System Performance ===\n");
		
		// Calculate average times and sort
		TArray<TPair<FString, double>> SortedSystems;
		for (const auto& Pair : Metrics.SystemExecutionTimes)
		{
			int32 ExecutionCount = Metrics.SystemExecutionCounts.FindRef(Pair.Key);
			double AvgTime = ExecutionCount > 0 ? (Pair.Value / ExecutionCount) : 0.0;
			SortedSystems.Add(TPair<FString, double>(Pair.Key, AvgTime));
		}
		SortedSystems.Sort([](const TPair<FString, double>& A, const TPair<FString, double>& B)
		{
			return A.Value > B.Value;
		});
		
		for (const auto& Pair : SortedSystems)
		{
			int32 ExecutionCount = Metrics.SystemExecutionCounts.FindRef(Pair.Key);
			Report += FString::Printf(TEXT("  %s: %.3f ms avg (%d validations)\n"), 
				*Pair.Key, Pair.Value * 1000.0, ExecutionCount);
		}
		Report += TEXT("\n");
	}
	
	return Report;
}

FValidationMetricsData UDelveDeepValidationSubsystem::GetValidationMetrics() const
{
	FValidationMetricsData Data;
	
	Data.TotalValidations = Metrics.TotalValidations;
	Data.PassedValidations = Metrics.PassedValidations;
	Data.FailedValidations = Metrics.FailedValidations;
	Data.ErrorFrequency = Metrics.ErrorFrequency;
	
	// Calculate average rule execution times
	for (const auto& Pair : Metrics.RuleExecutionTimes)
	{
		int32 ExecutionCount = Metrics.RuleExecutionCounts.FindRef(Pair.Key);
		if (ExecutionCount > 0)
		{
			float AvgTimeMs = static_cast<float>((Pair.Value / ExecutionCount) * 1000.0);
			Data.AverageRuleExecutionTime.Add(Pair.Key.ToString(), AvgTimeMs);
		}
	}
	
	// Calculate average system execution times
	for (const auto& Pair : Metrics.SystemExecutionTimes)
	{
		int32 ExecutionCount = Metrics.SystemExecutionCounts.FindRef(Pair.Key);
		if (ExecutionCount > 0)
		{
			float AvgTimeMs = static_cast<float>((Pair.Value / ExecutionCount) * 1000.0);
			Data.AverageSystemExecutionTime.Add(Pair.Key, AvgTimeMs);
		}
	}
	
	Data.LastResetTime = FDateTime::Now();
	
	return Data;
}

void UDelveDeepValidationSubsystem::ResetValidationMetrics()
{
	Metrics = FValidationMetrics();
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation metrics reset"));
}

bool UDelveDeepValidationSubsystem::SaveMetricsToFile(const FString& FilePath)
{
	FString SavePath = FilePath;
	if (SavePath.IsEmpty())
	{
		SavePath = FPaths::ProjectSavedDir() / TEXT("Validation") / TEXT("Metrics.json");
	}
	
	// Ensure directory exists
	FString Directory = FPaths::GetPath(SavePath);
	if (!FPaths::DirectoryExists(Directory))
	{
		if (!IFileManager::Get().MakeDirectory(*Directory, true))
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to create directory: %s"), *Directory);
			return false;
		}
	}
	
	// Build JSON object
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	JsonObject->SetNumberField(TEXT("TotalValidations"), Metrics.TotalValidations);
	JsonObject->SetNumberField(TEXT("PassedValidations"), Metrics.PassedValidations);
	JsonObject->SetNumberField(TEXT("FailedValidations"), Metrics.FailedValidations);
	JsonObject->SetStringField(TEXT("Timestamp"), FDateTime::Now().ToString());
	
	// Error frequency
	TSharedPtr<FJsonObject> ErrorFreqObj = MakeShareable(new FJsonObject);
	for (const auto& Pair : Metrics.ErrorFrequency)
	{
		ErrorFreqObj->SetNumberField(Pair.Key, Pair.Value);
	}
	JsonObject->SetObjectField(TEXT("ErrorFrequency"), ErrorFreqObj);
	
	// Rule execution times
	TSharedPtr<FJsonObject> RuleTimesObj = MakeShareable(new FJsonObject);
	for (const auto& Pair : Metrics.RuleExecutionTimes)
	{
		int32 ExecutionCount = Metrics.RuleExecutionCounts.FindRef(Pair.Key);
		double AvgTime = ExecutionCount > 0 ? (Pair.Value / ExecutionCount) : 0.0;
		
		TSharedPtr<FJsonObject> RuleObj = MakeShareable(new FJsonObject);
		RuleObj->SetNumberField(TEXT("TotalTime"), Pair.Value);
		RuleObj->SetNumberField(TEXT("ExecutionCount"), ExecutionCount);
		RuleObj->SetNumberField(TEXT("AverageTime"), AvgTime);
		
		RuleTimesObj->SetObjectField(Pair.Key.ToString(), RuleObj);
	}
	JsonObject->SetObjectField(TEXT("RuleExecutionTimes"), RuleTimesObj);
	
	// System execution times
	TSharedPtr<FJsonObject> SystemTimesObj = MakeShareable(new FJsonObject);
	for (const auto& Pair : Metrics.SystemExecutionTimes)
	{
		int32 ExecutionCount = Metrics.SystemExecutionCounts.FindRef(Pair.Key);
		double AvgTime = ExecutionCount > 0 ? (Pair.Value / ExecutionCount) : 0.0;
		
		TSharedPtr<FJsonObject> SystemObj = MakeShareable(new FJsonObject);
		SystemObj->SetNumberField(TEXT("TotalTime"), Pair.Value);
		SystemObj->SetNumberField(TEXT("ExecutionCount"), ExecutionCount);
		SystemObj->SetNumberField(TEXT("AverageTime"), AvgTime);
		
		SystemTimesObj->SetObjectField(Pair.Key, SystemObj);
	}
	JsonObject->SetObjectField(TEXT("SystemExecutionTimes"), SystemTimesObj);
	
	// Serialize to string
	FString JsonString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to serialize metrics to JSON"));
		return false;
	}
	
	// Write to file
	if (!FFileHelper::SaveStringToFile(JsonString, *SavePath))
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to save metrics to file: %s"), *SavePath);
		return false;
	}
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation metrics saved to: %s"), *SavePath);
	return true;
}

bool UDelveDeepValidationSubsystem::LoadMetricsFromFile(const FString& FilePath)
{
	FString LoadPath = FilePath;
	if (LoadPath.IsEmpty())
	{
		LoadPath = FPaths::ProjectSavedDir() / TEXT("Validation") / TEXT("Metrics.json");
	}
	
	// Check if file exists
	if (!FPaths::FileExists(LoadPath))
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("Metrics file not found: %s"), *LoadPath);
		return false;
	}
	
	// Load file content
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *LoadPath))
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to load metrics file: %s"), *LoadPath);
		return false;
	}
	
	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to parse metrics JSON"));
		return false;
	}
	
	// Reset current metrics
	Metrics = FValidationMetrics();
	
	// Load basic metrics
	Metrics.TotalValidations = JsonObject->GetIntegerField(TEXT("TotalValidations"));
	Metrics.PassedValidations = JsonObject->GetIntegerField(TEXT("PassedValidations"));
	Metrics.FailedValidations = JsonObject->GetIntegerField(TEXT("FailedValidations"));
	
	// Load error frequency
	const TSharedPtr<FJsonObject>* ErrorFreqObj;
	if (JsonObject->TryGetObjectField(TEXT("ErrorFrequency"), ErrorFreqObj))
	{
		for (const auto& Pair : (*ErrorFreqObj)->Values)
		{
			Metrics.ErrorFrequency.Add(Pair.Key, static_cast<int32>(Pair.Value->AsNumber()));
		}
	}
	
	// Load rule execution times
	const TSharedPtr<FJsonObject>* RuleTimesObj;
	if (JsonObject->TryGetObjectField(TEXT("RuleExecutionTimes"), RuleTimesObj))
	{
		for (const auto& Pair : (*RuleTimesObj)->Values)
		{
			const TSharedPtr<FJsonObject>* RuleObj;
			if (Pair.Value->TryGetObject(RuleObj))
			{
				FName RuleName(*Pair.Key);
				Metrics.RuleExecutionTimes.Add(RuleName, (*RuleObj)->GetNumberField(TEXT("TotalTime")));
				Metrics.RuleExecutionCounts.Add(RuleName, static_cast<int32>((*RuleObj)->GetNumberField(TEXT("ExecutionCount"))));
			}
		}
	}
	
	// Load system execution times
	const TSharedPtr<FJsonObject>* SystemTimesObj;
	if (JsonObject->TryGetObjectField(TEXT("SystemExecutionTimes"), SystemTimesObj))
	{
		for (const auto& Pair : (*SystemTimesObj)->Values)
		{
			const TSharedPtr<FJsonObject>* SystemObj;
			if (Pair.Value->TryGetObject(SystemObj))
			{
				Metrics.SystemExecutionTimes.Add(Pair.Key, (*SystemObj)->GetNumberField(TEXT("TotalTime")));
				Metrics.SystemExecutionCounts.Add(Pair.Key, static_cast<int32>((*SystemObj)->GetNumberField(TEXT("ExecutionCount"))));
			}
		}
	}
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation metrics loaded from: %s"), *LoadPath);
	return true;
}
