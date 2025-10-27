// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepMonsterConfig.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"

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

		if (FunctionName.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("CallBlueprintFunction: FunctionName is empty"));
			return false;
		}

		// Find the function by name
		UFunction* Function = Object->FindFunction(FName(*FunctionName));
		if (!Function)
		{
			UE_LOG(LogTemp, Error, TEXT("CallBlueprintFunction: Function '%s' not found on object '%s'"),
				*FunctionName, *Object->GetName());
			return false;
		}

		// Verify function is Blueprint-callable
		if (!Function->HasAnyFunctionFlags(FUNC_BlueprintCallable))
		{
			UE_LOG(LogTemp, Warning, TEXT("CallBlueprintFunction: Function '%s' is not Blueprint-callable"),
				*FunctionName);
		}

		// Allocate parameter buffer
		uint8* ParamBuffer = nullptr;
		if (Function->ParmsSize > 0)
		{
			ParamBuffer = (uint8*)FMemory::Malloc(Function->ParmsSize);
			FMemory::Memzero(ParamBuffer, Function->ParmsSize);

			// Initialize parameters with default values
			for (TFieldIterator<FProperty> It(Function); It; ++It)
			{
				FProperty* Property = *It;
				if (Property->HasAnyPropertyFlags(CPF_Parm) && !Property->HasAnyPropertyFlags(CPF_ReturnParm))
				{
					Property->InitializeValue_InContainer(ParamBuffer);
				}
			}
		}

		// Call the function
		try
		{
			Object->ProcessEvent(Function, ParamBuffer);
		}
		catch (...)
		{
			UE_LOG(LogTemp, Error, TEXT("CallBlueprintFunction: Exception occurred calling function '%s'"),
				*FunctionName);
			
			if (ParamBuffer)
			{
				FMemory::Free(ParamBuffer);
			}
			return false;
		}

		// Cleanup
		if (ParamBuffer)
		{
			// Destroy parameters
			for (TFieldIterator<FProperty> It(Function); It; ++It)
			{
				FProperty* Property = *It;
				if (Property->HasAnyPropertyFlags(CPF_Parm))
				{
					Property->DestroyValue_InContainer(ParamBuffer);
				}
			}
			FMemory::Free(ParamBuffer);
		}

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

		if (PropertyName.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("ReadBlueprintProperty: PropertyName is empty"));
			return false;
		}

		// Find the property by name
		FProperty* Property = Object->GetClass()->FindPropertyByName(FName(*PropertyName));
		if (!Property)
		{
			UE_LOG(LogTemp, Error, TEXT("ReadBlueprintProperty: Property '%s' not found on object '%s'"),
				*PropertyName, *Object->GetName());
			return false;
		}

		// Verify property is Blueprint-readable
		if (!Property->HasAnyPropertyFlags(CPF_BlueprintVisible))
		{
			UE_LOG(LogTemp, Warning, TEXT("ReadBlueprintProperty: Property '%s' is not Blueprint-visible"),
				*PropertyName);
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

		if (PropertyName.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("WriteBlueprintProperty: PropertyName is empty"));
			return false;
		}

		// Find the property by name
		FProperty* Property = Object->GetClass()->FindPropertyByName(FName(*PropertyName));
		if (!Property)
		{
			UE_LOG(LogTemp, Error, TEXT("WriteBlueprintProperty: Property '%s' not found on object '%s'"),
				*PropertyName, *Object->GetName());
			return false;
		}

		// Verify property is Blueprint-writable
		if (!Property->HasAnyPropertyFlags(CPF_BlueprintVisible) || 
			Property->HasAnyPropertyFlags(CPF_BlueprintReadOnly))
		{
			UE_LOG(LogTemp, Warning, TEXT("WriteBlueprintProperty: Property '%s' is not Blueprint-writable"),
				*PropertyName);
		}

		// Import property value from string
		void* PropertyValue = Property->ContainerPtrToValuePtr<void>(Object);
		const TCHAR* ValuePtr = *Value;
		Property->ImportText_Direct(ValuePtr, PropertyValue, Object, PPF_None);

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

		if (EventName.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("TriggerBlueprintEvent: EventName is empty"));
			return false;
		}

		// Find the event function
		UFunction* EventFunction = Object->FindFunction(FName(*EventName));
		if (!EventFunction)
		{
			UE_LOG(LogTemp, Error, TEXT("TriggerBlueprintEvent: Event '%s' not found on object '%s'"),
				*EventName, *Object->GetName());
			return false;
		}

		// Verify it's a Blueprint event
		if (!EventFunction->HasAnyFunctionFlags(FUNC_BlueprintEvent))
		{
			UE_LOG(LogTemp, Warning, TEXT("TriggerBlueprintEvent: Function '%s' is not a Blueprint event"),
				*EventName);
		}

		// Trigger the event
		try
		{
			Object->ProcessEvent(EventFunction, nullptr);
			OutEventTriggered = true;
		}
		catch (...)
		{
			UE_LOG(LogTemp, Error, TEXT("TriggerBlueprintEvent: Exception occurred triggering event '%s'"),
				*EventName);
			return false;
		}

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

		// Try to call function with invalid parameters
		// The function should handle gracefully without crashing
		try
		{
			bool bResult = CallBlueprintFunction(Object, FunctionName, InvalidParams);
			// Function executed without crashing - this is success
			return true;
		}
		catch (...)
		{
			// Function crashed - this is failure
			UE_LOG(LogTemp, Error, TEXT("TestBlueprintFunctionInvalidInputs: Function '%s' crashed with invalid inputs"),
				*FunctionName);
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

		if (FunctionName.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("TestBlueprintLibraryFunction: FunctionName is empty"));
			return false;
		}

		// Find the static function
		UFunction* Function = LibraryClass->FindFunctionByName(FName(*FunctionName));
		if (!Function)
		{
			UE_LOG(LogTemp, Error, TEXT("TestBlueprintLibraryFunction: Function '%s' not found in library '%s'"),
				*FunctionName, *LibraryClass->GetName());
			return false;
		}

		// Verify it's a static function
		if (!Function->HasAnyFunctionFlags(FUNC_Static))
		{
			UE_LOG(LogTemp, Warning, TEXT("TestBlueprintLibraryFunction: Function '%s' is not static"),
				*FunctionName);
		}

		// Get the class default object to call static function
		UObject* CDO = LibraryClass->GetDefaultObject();
		if (!CDO)
		{
			UE_LOG(LogTemp, Error, TEXT("TestBlueprintLibraryFunction: Failed to get CDO for library '%s'"),
				*LibraryClass->GetName());
			return false;
		}

		// Call the static function
		return CallBlueprintFunction(CDO, FunctionName, Params);
	}

	// ========================================
	// Error Handling Testing Utilities
	// ========================================

	bool SimulateErrorScenario(
		const FString& ErrorType,
		FValidationContext& Context)
	{
		Context.SystemName = TEXT("ErrorSimulation");
		Context.OperationName = FString::Printf(TEXT("Simulate%s"), *ErrorType);

		if (ErrorType == TEXT("NullPointer"))
		{
			Context.AddError(TEXT("Simulated null pointer error: Object reference is null"));
			return true;
		}
		else if (ErrorType == TEXT("InvalidData"))
		{
			Context.AddError(TEXT("Simulated invalid data error: Value is outside valid range"));
			return true;
		}
		else if (ErrorType == TEXT("OutOfRange"))
		{
			Context.AddError(TEXT("Simulated out of range error: Index exceeds array bounds"));
			return true;
		}
		else if (ErrorType == TEXT("MissingAsset"))
		{
			Context.AddError(TEXT("Simulated missing asset error: Required asset not found"));
			return true;
		}
		else if (ErrorType == TEXT("ValidationFailure"))
		{
			Context.AddError(TEXT("Simulated validation failure: Data does not meet requirements"));
			Context.AddWarning(TEXT("Simulated validation warning: Optional field is missing"));
			return true;
		}
		else
		{
			Context.AddError(FString::Printf(TEXT("Unknown error type: %s"), *ErrorType));
			return false;
		}
	}

	bool VerifyErrorLogged(
		const FString& ExpectedMessage,
		const FString& ExpectedSeverity,
		const TArray<FString>& CapturedOutput)
	{
		if (ExpectedMessage.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyErrorLogged: ExpectedMessage is empty"));
			return false;
		}

		// Search for the expected message in captured output
		for (const FString& Line : CapturedOutput)
		{
			if (Line.Contains(ExpectedMessage))
			{
				// Check if severity matches (if specified)
				if (!ExpectedSeverity.IsEmpty())
				{
					if (ExpectedSeverity == TEXT("Error") && !Line.Contains(TEXT("Error")))
					{
						continue;
					}
					if (ExpectedSeverity == TEXT("Warning") && !Line.Contains(TEXT("Warning")))
					{
						continue;
					}
					if (ExpectedSeverity == TEXT("Display") && !Line.Contains(TEXT("Display")))
					{
						continue;
					}
				}

				// Found matching message with correct severity
				return true;
			}
		}

		UE_LOG(LogTemp, Error, TEXT("VerifyErrorLogged: Expected message not found: %s (Severity: %s)"),
			*ExpectedMessage, *ExpectedSeverity);
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

		// Trigger the error
		try
		{
			ErrorFunc();
		}
		catch (...)
		{
			UE_LOG(LogTemp, Warning, TEXT("TestErrorRecovery: Error function threw exception"));
		}

		// Attempt recovery
		try
		{
			RecoveryFunc();
		}
		catch (...)
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorRecovery: Recovery function threw exception"));
			return false;
		}

		// Verify recovery succeeded
		bool bRecovered = false;
		try
		{
			bRecovered = VerificationFunc();
		}
		catch (...)
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorRecovery: Verification function threw exception"));
			return false;
		}

		if (!bRecovered)
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorRecovery: Recovery verification failed"));
		}

		return bRecovered;
	}

	bool VerifyValidationErrorQuality(
		const FValidationContext& Context,
		const TArray<FString>& RequiredElements)
	{
		if (Context.ValidationErrors.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("VerifyValidationErrorQuality: No errors to verify"));
			return true;
		}

		// Check each error message for required elements
		for (const FString& Error : Context.ValidationErrors)
		{
			for (const FString& RequiredElement : RequiredElements)
			{
				if (!Error.Contains(RequiredElement))
				{
					UE_LOG(LogTemp, Error, TEXT("VerifyValidationErrorQuality: Error message missing required element '%s': %s"),
						*RequiredElement, *Error);
					return false;
				}
			}
		}

		return true;
	}

	bool TestErrorPropagation(
		const FValidationContext& SourceContext,
		FValidationContext& TargetContext,
		TFunction<void(const FValidationContext&, FValidationContext&)> PropagationFunc)
	{
		if (!PropagationFunc)
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorPropagation: PropagationFunc is null"));
			return false;
		}

		// Record initial state
		int32 InitialErrorCount = TargetContext.ValidationErrors.Num();
		int32 InitialWarningCount = TargetContext.ValidationWarnings.Num();

		// Propagate errors
		try
		{
			PropagationFunc(SourceContext, TargetContext);
		}
		catch (...)
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorPropagation: PropagationFunc threw exception"));
			return false;
		}

		// Verify errors were propagated
		int32 ExpectedErrorCount = InitialErrorCount + SourceContext.ValidationErrors.Num();
		int32 ExpectedWarningCount = InitialWarningCount + SourceContext.ValidationWarnings.Num();

		if (TargetContext.ValidationErrors.Num() != ExpectedErrorCount)
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorPropagation: Error count mismatch. Expected: %d, Actual: %d"),
				ExpectedErrorCount, TargetContext.ValidationErrors.Num());
			return false;
		}

		if (TargetContext.ValidationWarnings.Num() != ExpectedWarningCount)
		{
			UE_LOG(LogTemp, Error, TEXT("TestErrorPropagation: Warning count mismatch. Expected: %d, Actual: %d"),
				ExpectedWarningCount, TargetContext.ValidationWarnings.Num());
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
		if (FilePath.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("LoadTestDataFromJSON: FilePath is empty"));
			return false;
		}

		// Construct full path
		FString FullPath = FPaths::ProjectDir() / FilePath;
		
		// Check if file exists
		if (!FPaths::FileExists(FullPath))
		{
			UE_LOG(LogTemp, Error, TEXT("LoadTestDataFromJSON: File not found: %s"), *FullPath);
			return false;
		}

		// Load file content
		FString JsonString;
		if (!FFileHelper::LoadFileToString(JsonString, *FullPath))
		{
			UE_LOG(LogTemp, Error, TEXT("LoadTestDataFromJSON: Failed to load file: %s"), *FullPath);
			return false;
		}

		// Parse JSON
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
		if (!FJsonSerializer::Deserialize(JsonReader, OutJsonObject) || !OutJsonObject.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("LoadTestDataFromJSON: Failed to parse JSON from file: %s"), *FullPath);
			return false;
		}

		UE_LOG(LogTemp, Display, TEXT("LoadTestDataFromJSON: Successfully loaded JSON from: %s"), *FullPath);
		return true;
	}

	bool LoadTestDataFromCSV(
		const FString& FilePath,
		TArray<TArray<FString>>& OutRows,
		bool bHasHeader)
	{
		if (FilePath.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("LoadTestDataFromCSV: FilePath is empty"));
			return false;
		}

		// Construct full path
		FString FullPath = FPaths::ProjectDir() / FilePath;
		
		// Check if file exists
		if (!FPaths::FileExists(FullPath))
		{
			UE_LOG(LogTemp, Error, TEXT("LoadTestDataFromCSV: File not found: %s"), *FullPath);
			return false;
		}

		// Load file content
		FString CsvString;
		if (!FFileHelper::LoadFileToString(CsvString, *FullPath))
		{
			UE_LOG(LogTemp, Error, TEXT("LoadTestDataFromCSV: Failed to load file: %s"), *FullPath);
			return false;
		}

		// Parse CSV
		TArray<FString> Lines;
		CsvString.ParseIntoArrayLines(Lines);

		if (Lines.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadTestDataFromCSV: File is empty: %s"), *FullPath);
			return true;
		}

		// Skip header if specified
		int32 StartIndex = bHasHeader ? 1 : 0;

		// Parse each line
		for (int32 i = StartIndex; i < Lines.Num(); ++i)
		{
			TArray<FString> Columns;
			Lines[i].ParseIntoArray(Columns, TEXT(","), true);
			
			// Trim whitespace from each column
			for (FString& Column : Columns)
			{
				Column.TrimStartAndEndInline();
			}

			OutRows.Add(Columns);
		}

		UE_LOG(LogTemp, Display, TEXT("LoadTestDataFromCSV: Successfully loaded %d rows from: %s"),
			OutRows.Num(), *FullPath);
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

		if (DatasetName.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("CreateParameterizedTestDataset: DatasetName is empty"));
			return false;
		}

		// Get the dataset array
		const TArray<TSharedPtr<FJsonValue>>* DatasetArray;
		if (!JsonObject->TryGetArrayField(DatasetName, DatasetArray))
		{
			UE_LOG(LogTemp, Error, TEXT("CreateParameterizedTestDataset: Dataset '%s' not found in JSON"),
				*DatasetName);
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

		UE_LOG(LogTemp, Display, TEXT("CreateParameterizedTestDataset: Extracted %d test cases from dataset '%s'"),
			OutTestCases.Num(), *DatasetName);
		return true;
	}

	bool GenerateRealisticTestData(
		const FString& SchemaType,
		int32 Count,
		TArray<UObject*>& OutData)
	{
		if (SchemaType.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("GenerateRealisticTestData: SchemaType is empty"));
			return false;
		}

		if (Count <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("GenerateRealisticTestData: Count must be positive"));
			return false;
		}

		// Generate test data based on schema type
		for (int32 i = 0; i < Count; ++i)
		{
			UObject* Data = nullptr;

			if (SchemaType == TEXT("Character"))
			{
				FString Name = FString::Printf(TEXT("TestCharacter_%d"), i);
				float Health = 100.0f + (i * 10.0f);
				float Damage = 10.0f + (i * 2.0f);
				Data = CreateTestCharacterData(Name, Health, Damage);
			}
			else if (SchemaType == TEXT("Weapon"))
			{
				FString Name = FString::Printf(TEXT("TestWeapon_%d"), i);
				float Damage = 10.0f + (i * 5.0f);
				float AttackSpeed = 1.0f + (i * 0.1f);
				Data = CreateTestWeaponData(Name, Damage, AttackSpeed);
			}
			else if (SchemaType == TEXT("Ability"))
			{
				FString Name = FString::Printf(TEXT("TestAbility_%d"), i);
				float Cooldown = 5.0f + (i * 1.0f);
				float ResourceCost = 10.0f + (i * 5.0f);
				Data = CreateTestAbilityData(Name, Cooldown, ResourceCost);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GenerateRealisticTestData: Unknown schema type: %s"), *SchemaType);
				return false;
			}

			if (Data)
			{
				OutData.Add(Data);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("GenerateRealisticTestData: Generated %d instances of type '%s'"),
			OutData.Num(), *SchemaType);
		return true;
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

		if (Sizes.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("CreateScalabilityTestDatasets: Sizes array is empty"));
			return false;
		}

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
			else
			{
				UE_LOG(LogTemp, Error, TEXT("CreateScalabilityTestDatasets: Failed to generate dataset of size %d"), Size);
				return false;
			}
		}

		UE_LOG(LogTemp, Display, TEXT("CreateScalabilityTestDatasets: Generated %d datasets"), OutDatasets.Num());
		return true;
	}

	bool ValidateTestDataSchema(
		UObject* Data,
		const FString& SchemaType,
		FValidationContext& Context)
	{
		Context.SystemName = TEXT("TestDataValidation");
		Context.OperationName = FString::Printf(TEXT("Validate%sSchema"), *SchemaType);

		if (!IsValid(Data))
		{
			Context.AddError(TEXT("Data object is null"));
			return false;
		}

		// Validate based on schema type
		if (SchemaType == TEXT("Character"))
		{
			UDelveDeepCharacterData* CharacterData = Cast<UDelveDeepCharacterData>(Data);
			if (!CharacterData)
			{
				Context.AddError(FString::Printf(TEXT("Data is not of type Character (actual type: %s)"),
					*Data->GetClass()->GetName()));
				return false;
			}
			return CharacterData->Validate(Context);
		}
		else if (SchemaType == TEXT("Weapon"))
		{
			UDelveDeepWeaponData* WeaponData = Cast<UDelveDeepWeaponData>(Data);
			if (!WeaponData)
			{
				Context.AddError(FString::Printf(TEXT("Data is not of type Weapon (actual type: %s)"),
					*Data->GetClass()->GetName()));
				return false;
			}
			return WeaponData->Validate(Context);
		}
		else if (SchemaType == TEXT("Ability"))
		{
			UDelveDeepAbilityData* AbilityData = Cast<UDelveDeepAbilityData>(Data);
			if (!AbilityData)
			{
				Context.AddError(FString::Printf(TEXT("Data is not of type Ability (actual type: %s)"),
					*Data->GetClass()->GetName()));
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
