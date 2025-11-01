// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DelveDeepValidation.h"
#include "DelveDeepValidationInterface.generated.h"

/**
 * Interface for objects that can be validated.
 * Implement this interface to provide custom validation logic in C++ or Blueprint.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDelveDeepValidatable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for objects that can be validated.
 * Implement ValidateData() to provide custom validation logic.
 */
class IDelveDeepValidatable
{
	GENERATED_BODY()

public:
	/**
	 * Validates the data in this object.
	 * Can be implemented in C++ or Blueprint.
	 * 
	 * @param Context The validation context to populate with issues
	 * @return True if validation passed (no Critical or Error issues), false otherwise
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "DelveDeep|Validation")
	bool ValidateData(FDelveDeepValidationContext& Context);
};
