// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestUtilities.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

namespace DelveDeepTestUtils
{
	// ========================================
	// Blueprint Testing Utilities
	// ========================================

	bool CallBlueprintFunction(
		UObject* Object,
		const FString& FunctionName,
		const TArray<FString>& Params)
	{
		if (!IsValid(Object))
		{
			UE_LOG(LogTemp, Error, TEXT("CallBlueprintFunction: Object is null"));
			return false;
		}

		// Find the function
		UFunction* Function = Object->FindFunction(FName(*FunctionName));
		if (!Function)
		{
			UE_LOG(LogTemp, Error, TEXT("CallBlueprintFunction: Function not found: %s"), *FunctionName);
			return false;
		}

		// For now, we support calling functions without parameters
		// Full parameter support would require parsing and converting string params to proper types
		if (Params.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("CallBlueprintFunction: Parameter passing not yet implemented"));
		}

		// Call the function
		Object->ProcessEvent(Function, nullptr);
		return true;
	}

	bool ReadBlueprintProperty(
		UObject* Object,
		const FString& PropertyName,
		FString& OutValue)
	{
		if (!IsValid(Object))
		{
			UE_LOG(LogTemp, Error, TEXT("ReadBlueprintProperty: Object is null"));
			return false;
		}

		// Find the property
		FProperty* Property = Object->GetClass()->FindPropertyByName(FName(*PropertyName));
		if (!Property)
		{
			UE_LOG(LogTemp, Error, TEXT("ReadBlueprintProperty: Property not found: %s"), *PropertyName);
			return false;
		}

		// Export property value to string
		const void* PropertyValue = Property->ContainerPtrToValuePtr<void>(Object);
		Property->ExportTextItem_Direct(OutValue, PropertyValue, nullptr, Object, PPF_None);
		return true;
	}

	bool WriteBlueprintProperty(
		UObject* Object,
		const FString& PropertyName,
		const FString& Value)
	{
		if (!IsValid(Object))
		{
			UE_LOG(LogTemp, Error, TEXT("WriteBlueprintProperty: Object is null"));
			return false;
		}

		// Find the property
		FProperty* Property = Object->GetClass()->FindPropertyByName(FName(*PropertyName));
		if (!Property)
		{
			UE_LOG(LogTemp, Error, TEXT("WriteBlueprintProperty: Property not found: %s"), *PropertyName);
			return false;
		}

		// Import property value from string
		void* PropertyValue = Property->ContainerPtrToValuePtr<void>(Object);
		Property->ImportText_Direct(*Value, PropertyValue, Object, PPF_None);
		return true;
	}

	bool TriggerBlueprintEvent(
		UObject* Object,
		const FString& EventName,
		bool& OutEventTriggered)
	{
		OutEventTriggered = false;

		if (!IsValid(Object))
		{
			UE_LOG(LogTemp, Error, TEXT("TriggerBlueprintEvent: Object is null"));
			return false;
		}

		// Find the event function
		UFunction* EventFunction = Object->FindFunction(FName(*EventName));
		if (!EventFunction)
		{
			UE_LOG(LogTemp, Error, TEXT("TriggerBlueprintEvent: Event not found: %s"), *EventName);
			return false;
		}

		// Trigger the event
		Object->ProcessEvent(EventFunction, nullptr);
		OutEventTriggered = true;
		return true;
	}

	bool TestBlueprintFunctionInvalidInputs(
		UObject* Object,
		const FString& FunctionName,
		const TArray<FString>& InvalidParams)
	{
		if (!IsValid(Object))
		{
			UE_LOG(LogTemp, Error, TEXT("TestBlueprintFunctionInvalidInputs: Object is null"));
			return false;
		}

		// Find the function
		UFunction* Function = Object->FindFunction(FName(*FunctionName));
		if (!Function)
		{
			UE_LOG(LogTemp, Error, TEXT("TestBlueprintFunctionInvalidInputs: Function not found: %s"), *FunctionName);
			return false;
		}

		// Try calling with invalid inputs
		// The function should handle gracefully without crashing
		try
		{
			// For now, we just call with null parameters
			// Full implementation would require parameter marshalling
			Object->ProcessEvent(Function, nullptr);
			return true;
		}
		catch (...)
		{
			UE_LOG(LogTemp, Error, TEXT("TestBlueprintFunctionInvalidInputs: Function crashed with invalid inputs"));
			return false;
		}
	}

	bool TestBlueprintLibraryFunction(
		UClass* LibraryClass,
		const FString& FunctionName,
		const TArray<FString>& Params)
	{
		if (!LibraryClass)
		{
			UE_LOG(LogTemp, Error, TEXT("TestBlueprintLibraryFunction: LibraryClass is null"));
			return false;
		}

		// Find the static function
		UFunction* Function = LibraryClass->FindFunctionByName(FName(*FunctionName));
		if (!Function)
		{
			UE_LOG(LogTemp, Error, TEXT("TestBlueprintLibraryFunction: Function not found: %s"), *FunctionName);
			return false;
		}

		// Get the default object for static function calls
		UObject* CDO = LibraryClass->GetDefaultObject();
		if (!CDO)
		{
			UE_LOG(LogTemp, Error, TEXT("TestBlueprintLibraryFunction: Failed to get CDO"));
			return false;
		}

		// Call the static function
		CDO->ProcessEvent(Function, nullptr);
		return true;
	}

	// ========================================
	// Error Handling Testing Utilities
	// ========================================

	bool SimulateErrorScenario(
		const FString& ErrorType,
		FDelveDeepValidationContext& Context)
	{
		Context.SystemName = TEXT("TestUtilities");
		Context.OperationName = TEXT("SimulateErrorScenario");

		if (ErrorType == TEXT("NullPointer"))
		{
			Context.AddError(TEXT("Simulated null pointer error"));
			return true;
		}
		else if (ErrorType == TEXT("InvalidData"))
		{
			Context.AddError(TEXT("Simulated invalid data error"));
			return true;
		}
		else if (ErrorType == TEXT("OutOfRange"))
		{
			Context.AddError(TEXT("Simulated out of range error"));
			return true;
		}
		else if (ErrorType == TEXT("FileNotFound"))
		{
			Context.AddError(TEXT("Simulated file not found error"));
			return true;
		}
		else
		{
			Context.AddWarning(FString::Printf(TEXT("Unknown error type: %s"), *ErrorType));
			return false;
		}
	}

	bool VerifyErrorLogged(
		const FString& ExpectedMessage,
		const FString& ExpectedSeverity,
		const TArray<FString>& CapturedOutput)
	{
		// Search for the expected message in captured output
		for (const FString& Line : CapturedOutput)
		{
			if (Line.Contains(ExpectedMessage))
			{
				// Check severity if specified
				if (!ExpectedSeverity.IsEmpty())
				{
					if (ExpectedSeverity == TEXT("Error") && Line.Contains(TEXT("Error")))
					{
						return true;
					}
					else if (ExpectedSeverity == TEXT("Warning") && Line.Contains(TEXT("Warning")))
					{
						return true;
					}
					else if (ExpectedSeverity == TEXT("Display") && !Line.Contains(TEXT("Error")) && !Line.Contains(TEXT("Warning")))
					{
						return true;
					}
				}
				else
				{
					// No severity check, just message match
					return true;
				}
			}
		}

		UE_LOG(LogTemp, Error, TEXT("VerifyErrorLogged: Expected message not found: %s"), *ExpectedMessage);
		return false;
	}

	bool TestErrorRecovery(
		TFunction<void()> ErrorFunc,
		TFunction<void()> RecoveryFunc,
		TFunction<bool()> VerificationFunc)
	{
		if (!ErrorFunc || !RecoveryFunc || !VerificationFunc)
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorRecovery: One or more functions are null"));
			return false;
		}

		// Trigger error
		ErrorFunc();

		// Attempt recovery
		RecoveryFunc();

		// Verify recovery succeeded
		bool bRecovered = VerificationFunc();
		if (!bRecovered)
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorRecovery: Recovery verification failed"));
		}

		return bRecovered;
	}

	bool VerifyValidationErrorQuality(
		const FDelveDeepValidationContext& Context,
		const TArray<FString>& RequiredElements)
	{
		if (Context.ValidationErrors.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("VerifyValidationErrorQuality: No errors to verify"));
			return true;
		}

		// Check that all error messages contain required elements
		for (const FString& Error : Context.ValidationErrors)
		{
			for (const FString& RequiredElement : RequiredElements)
			{
				if (!Error.Contains(RequiredElement))
				{
					UE_LOG(LogTemp, Error, TEXT("VerifyValidationErrorQuality: Error missing required element '%s': %s"),
						*RequiredElement, *Error);
					return false;
				}
			}
		}

		return true;
	}

	bool TestErrorPropagation(
		const FDelveDeepValidationContext& SourceContext,
		FDelveDeepValidationContext& TargetContext,
		TFunction<void(const FDelveDeepValidationContext&, FDelveDeepValidationContext&)> PropagationFunc)
	{
		if (!PropagationFunc)
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorPropagation: Propagation function is null"));
			return false;
		}

		// Record initial error count
		int32 InitialErrorCount = TargetContext.ValidationErrors.Num();

		// Propagate errors
		PropagationFunc(SourceContext, TargetContext);

		// Verify errors were propagated
		int32 FinalErrorCount = TargetContext.ValidationErrors.Num();
		int32 PropagatedErrors = FinalErrorCount - InitialErrorCount;

		if (PropagatedErrors != SourceContext.ValidationErrors.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorPropagation: Expected %d errors, got %d"),
				SourceContext.ValidationErrors.Num(), PropagatedErrors);
			return false;
		}

		return true;
	}

	// ========================================
	// Test Data Loading Utilities
	// ========================================

	bool LoadTestDataFromJSON(
		const FString& FilePath,
		TSharedPtr<FJsonObject>& OutJsonObject)
	{
		// Build full path
		FString FullPath = FPaths::ProjectDir() / FilePath;

		// Read file
		FString JsonString;
		if (!FFileHelper::LoadFileToString(JsonString, *FullPath))
		{
			UE_LOG(LogTemp, Error, TEXT("LoadTestDataFromJSON: Failed to read file: %s"), *FullPath);
			return false;
		}

		// Parse JSON
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		if (!FJsonSerializer::Deserialize(Reader, OutJsonObject))
		{
			UE_LOG(LogTemp, Error, TEXT("LoadTestDataFromJSON: Failed to parse JSON: %s"), *FullPath);
			return false;
		}

		return true;
	}

	bool LoadTestDataFromCSV(
		const FString& FilePath,
		TArray<TArray<FString>>& OutRows,
		bool bHasHeader)
	{
		// Build full path
		FString FullPath = FPaths::ProjectDir() / FilePath;

		// Read file
		FString CSVString;
		if (!FFileHelper::LoadFileToString(CSVString, *FullPath))
		{
			UE_LOG(LogTemp, Error, TEXT("LoadTestDataFromCSV: Failed to read file: %s"), *FullPath);
			return false;
		}

		// Parse CSV
		TArray<FString> Lines;
		CSVString.ParseIntoArrayLines(Lines);

		// Skip header if present
		int32 StartIndex = bHasHeader ? 1 : 0;

		for (int32 i = StartIndex; i < Lines.Num(); ++i)
		{
			TArray<FString> Columns;
			Lines[i].ParseIntoArray(Columns, TEXT(","));

			// Trim whitespace from each column
			for (FString& Column : Columns)
			{
				Column.TrimStartAndEndInline();
			}

			OutRows.Add(Columns);
		}

		return true;
	}

	bool CreateParameterizedTestDataset(
		const TSharedPtr<FJsonObject>& JsonObject,
		const FString& DatasetName,
		TArray<TSharedPtr<FJsonObject>>& OutTestCases)
	{
		if (!JsonObject.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("CreateParameterizedTestDataset: JsonObject is null"));
			return false;
		}

		// Get the dataset array
		const TArray<TSharedPtr<FJsonValue>>* DatasetArray;
		if (!JsonObject->TryGetArrayField(DatasetName, DatasetArray))
		{
			UE_LOG(LogTemp, Error, TEXT("CreateParameterizedTestDataset: Dataset not found: %s"), *DatasetName);
			return false;
		}

		// Extract test cases
		for (const TSharedPtr<FJsonValue>& Value : *DatasetArray)
		{
			TSharedPtr<FJsonObject> TestCase = Value->AsObject();
			if (TestCase.IsValid())
			{
				OutTestCases.Add(TestCase);
			}
		}

		return true;
	}

	bool GenerateRealisticTestData(
		const FString& SchemaType,
		int32 Count,
		TArray<UObject*>& OutData)
	{
		OutData.Empty();

		if (SchemaType == TEXT("Character"))
		{
			for (int32 i = 0; i < Count; ++i)
			{
				FString Name = FString::Printf(TEXT("Character_%d"), i);
				float Health = 100.0f + (i * 10.0f);
				float Damage = 10.0f + (i * 2.0f);
				UDelveDeepCharacterData* Data = CreateTestCharacterData(Name, Health, Damage);
				OutData.Add(Data);
			}
			return true;
		}
		else if (SchemaType == TEXT("Weapon"))
		{
			for (int32 i = 0; i < Count; ++i)
			{
				FString Name = FString::Printf(TEXT("Weapon_%d"), i);
				float Damage = 10.0f + (i * 5.0f);
				float AttackSpeed = 1.0f + (i * 0.1f);
				UDelveDeepWeaponData* Data = CreateTestWeaponData(Name, Damage, AttackSpeed);
				OutData.Add(Data);
			}
			return true;
		}
		else if (SchemaType == TEXT("Ability"))
		{
			for (int32 i = 0; i < Count; ++i)
			{
				FString Name = FString::Printf(TEXT("Ability_%d"), i);
				float Cooldown = 5.0f + (i * 1.0f);
				float ResourceCost = 10.0f + (i * 5.0f);
				UDelveDeepAbilityData* Data = CreateTestAbilityData(Name, Cooldown, ResourceCost);
				OutData.Add(Data);
			}
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GenerateRealisticTestData: Unknown schema type: %s"), *SchemaType);
			return false;
		}
	}

	bool CreateScalabilityTestDatasets(
		UObject* BaseData,
		const TArray<int32>& Sizes,
		TMap<int32, TArray<UObject*>>& OutDatasets)
	{
		if (!IsValid(BaseData))
		{
			UE_LOG(LogTemp, Error, TEXT("CreateScalabilityTestDatasets: BaseData is null"));
			return false;
		}

		OutDatasets.Empty();

		// Determine schema type from base data
		FString SchemaType;
		if (Cast<UDelveDeepCharacterData>(BaseData))
		{
			SchemaType = TEXT("Character");
		}
		else if (Cast<UDelveDeepWeaponData>(BaseData))
		{
			SchemaType = TEXT("Weapon");
		}
		else if (Cast<UDelveDeepAbilityData>(BaseData))
		{
			SchemaType = TEXT("Ability");
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CreateScalabilityTestDatasets: Unknown base data type"));
			return false;
		}

		// Generate datasets for each size
		for (int32 Size : Sizes)
		{
			TArray<UObject*> Dataset;
			if (GenerateRealisticTestData(SchemaType, Size, Dataset))
			{
				OutDatasets.Add(Size, Dataset);
			}
		}

		return OutDatasets.Num() > 0;
	}

	bool ValidateTestDataSchema(
		UObject* Data,
		const FString& SchemaType,
		FDelveDeepValidationContext& Context)
	{
		Context.SystemName = TEXT("TestUtilities");
		Context.OperationName = TEXT("ValidateTestDataSchema");

		if (!IsValid(Data))
		{
			Context.AddError(TEXT("Data is null"));
			return false;
		}

		// Validate based on schema type
		if (SchemaType == TEXT("Character"))
		{
			UDelveDeepCharacterData* CharacterData = Cast<UDelveDeepCharacterData>(Data);
			if (!CharacterData)
			{
				Context.AddError(TEXT("Data is not a Character"));
				return false;
			}
			return CharacterData->Validate(Context);
		}
		else if (SchemaType == TEXT("Weapon"))
		{
			UDelveDeepWeaponData* WeaponData = Cast<UDelveDeepWeaponData>(Data);
			if (!WeaponData)
			{
				Context.AddError(TEXT("Data is not a Weapon"));
				return false;
			}
			return WeaponData->Validate(Context);
		}
		else if (SchemaType == TEXT("Ability"))
		{
			UDelveDeepAbilityData* AbilityData = Cast<UDelveDeepAbilityData>(Data);
			if (!AbilityData)
			{
				Context.AddError(TEXT("Data is not an Ability"));
				return false;
			}
			return AbilityData->Validate(Context);
		}
		else
		{
			Context.AddError(FString::Printf(TEXT("Unknown schema type: %s"), *SchemaType));
			return false;
		}
	}
}
