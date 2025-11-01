// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepPerformanceBudget.h"
#include "DelveDeepTelemetrySubsystem.h"

bool FPerformanceBudgetEntry::Validate(FDelveDeepValidationContext& Context) const
{
	bool bIsValid = true;

	// Validate system name
	if (SystemName.IsNone())
	{
		Context.AddError(TEXT("System name cannot be empty"));
		bIsValid = false;
	}

	// Validate budget value
	if (BudgetMilliseconds <= 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("Budget for system '%s' must be positive (got %.2fms)"),
			*SystemName.ToString(), BudgetMilliseconds));
		bIsValid = false;
	}

	if (BudgetMilliseconds > 10.0f)
	{
		Context.AddWarning(FString::Printf(
			TEXT("Budget for system '%s' is unusually high (%.2fms)"),
			*SystemName.ToString(), BudgetMilliseconds));
	}

	return bIsValid;
}

const FPerformanceBudgetEntry* UDelveDeepPerformanceBudget::GetSystemBudget(FName SystemName) const
{
	for (const FPerformanceBudgetEntry& Entry : SystemBudgets)
	{
		if (Entry.SystemName == SystemName)
		{
			return &Entry;
		}
	}

	return nullptr;
}

bool UDelveDeepPerformanceBudget::Validate(FDelveDeepValidationContext& Context) const
{
	bool bIsValid = true;

	// Validate total frame budget
	if (TotalFrameBudgetMs <= 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("Total frame budget must be positive (got %.2fms)"),
			TotalFrameBudgetMs));
		bIsValid = false;
	}

	if (TotalFrameBudgetMs < 10.0f)
	{
		Context.AddWarning(FString::Printf(
			TEXT("Total frame budget is very low (%.2fms, target: 16.67ms for 60 FPS)"),
			TotalFrameBudgetMs));
	}

	// Validate system budgets
	float TotalSystemBudgets = 0.0f;
	TSet<FName> SeenSystems;

	for (const FPerformanceBudgetEntry& Entry : SystemBudgets)
	{
		// Validate individual entry
		if (!Entry.Validate(Context))
		{
			bIsValid = false;
		}

		// Check for duplicates
		if (SeenSystems.Contains(Entry.SystemName))
		{
			Context.AddError(FString::Printf(
				TEXT("Duplicate budget entry for system '%s'"),
				*Entry.SystemName.ToString()));
			bIsValid = false;
		}
		else
		{
			SeenSystems.Add(Entry.SystemName);
		}

		// Accumulate total
		TotalSystemBudgets += Entry.BudgetMilliseconds;
	}

	// Warn if total system budgets exceed frame budget
	if (TotalSystemBudgets > TotalFrameBudgetMs)
	{
		Context.AddWarning(FString::Printf(
			TEXT("Total system budgets (%.2fms) exceed frame budget (%.2fms)"),
			TotalSystemBudgets, TotalFrameBudgetMs));
	}

	// Warn if no budgets defined
	if (SystemBudgets.Num() == 0)
	{
		Context.AddWarning(TEXT("No system budgets defined"));
	}

	return bIsValid;
}

void UDelveDeepPerformanceBudget::PostLoad()
{
	Super::PostLoad();

	// Validate on load
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("LoadPerformanceBudget");

	if (!Validate(Context))
	{
		UE_LOG(LogDelveDeepTelemetry, Error,
			TEXT("Performance budget validation failed: %s"),
			*Context.GetReport());
	}
	else if (Context.ValidationWarnings.Num() > 0)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning,
			TEXT("Performance budget validation warnings: %s"),
			*Context.GetReport());
	}
}

#if WITH_EDITOR
EDataValidationResult UDelveDeepPerformanceBudget::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Validate using our validation context
	FDelveDeepValidationContext ValidationContext;
	ValidationContext.SystemName = TEXT("Configuration");
	ValidationContext.OperationName = TEXT("ValidatePerformanceBudget");

	if (!Validate(ValidationContext))
	{
		for (const FString& Error : ValidationContext.ValidationErrors)
		{
			Context.AddError(FText::FromString(Error));
		}
		Result = EDataValidationResult::Invalid;
	}

	for (const FString& Warning : ValidationContext.ValidationWarnings)
	{
		Context.AddWarning(FText::FromString(Warning));
	}

	return Result;
}
#endif
