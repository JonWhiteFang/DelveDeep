// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

/**
 * DelveDeep Test Macros
 * 
 * Google Test-style assertion macros for Unreal Engine's Automation System.
 * These macros wrap Unreal's test functions to provide a familiar testing API.
 * 
 * Usage:
 *   EXPECT_* macros: Non-fatal assertions (test continues on failure)
 *   ASSERT_* macros: Fatal assertions (test stops on failure)
 */

// Boolean assertions
#define EXPECT_TRUE(Expression) \
	TestTrue(TEXT(#Expression), Expression)

#define EXPECT_FALSE(Expression) \
	TestFalse(TEXT(#Expression), Expression)

#define ASSERT_TRUE(Expression) \
	if (!TestTrue(TEXT(#Expression), Expression)) { return false; }

#define ASSERT_FALSE(Expression) \
	if (!TestFalse(TEXT(#Expression), Expression)) { return false; }

// Equality assertions
#define EXPECT_EQ(Actual, Expected) \
	TestEqual(TEXT(#Actual " == " #Expected), Actual, Expected)

#define EXPECT_NE(Actual, Expected) \
	TestNotEqual(TEXT(#Actual " != " #Expected), Actual, Expected)

#define ASSERT_EQ(Actual, Expected) \
	if (!TestEqual(TEXT(#Actual " == " #Expected), Actual, Expected)) { return false; }

#define ASSERT_NE(Actual, Expected) \
	if (!TestNotEqual(TEXT(#Actual " != " #Expected), Actual, Expected)) { return false; }

// Comparison assertions
#define EXPECT_LT(Actual, Expected) \
	TestTrue(TEXT(#Actual " < " #Expected), (Actual) < (Expected))

#define EXPECT_LE(Actual, Expected) \
	TestTrue(TEXT(#Actual " <= " #Expected), (Actual) <= (Expected))

#define EXPECT_GT(Actual, Expected) \
	TestTrue(TEXT(#Actual " > " #Expected), (Actual) > (Expected))

#define EXPECT_GE(Actual, Expected) \
	TestTrue(TEXT(#Actual " >= " #Expected), (Actual) >= (Expected))

// Null pointer assertions
#define EXPECT_NULL(Pointer) \
	TestNull(TEXT(#Pointer " is null"), Pointer)

#define EXPECT_NOT_NULL(Pointer) \
	TestNotNull(TEXT(#Pointer " is not null"), Pointer)

#define ASSERT_NOT_NULL(Pointer) \
	if (!TestNotNull(TEXT(#Pointer " is not null"), Pointer)) { return false; }

// Floating point assertions
#define EXPECT_NEAR(Actual, Expected, Tolerance) \
	TestTrue(TEXT(#Actual " ~= " #Expected " (tolerance: " #Tolerance ")"), \
		FMath::Abs((Actual) - (Expected)) <= (Tolerance))

// String assertions
#define EXPECT_STR_EQ(Actual, Expected) \
	TestEqual(TEXT(#Actual " == " #Expected), Actual, Expected)

#define EXPECT_STR_CONTAINS(Haystack, Needle) \
	TestTrue(TEXT(#Haystack " contains " #Needle), \
		(Haystack).Contains(Needle))

// Array/Container assertions
#define EXPECT_ARRAY_SIZE(Array, ExpectedSize) \
	TestEqual(TEXT(#Array ".Num() == " #ExpectedSize), (Array).Num(), ExpectedSize)

#define EXPECT_ARRAY_CONTAINS(Array, Element) \
	TestTrue(TEXT(#Array " contains " #Element), (Array).Contains(Element))

// Validation context assertions
#define EXPECT_VALID(Context) \
	TestTrue(TEXT(#Context " is valid"), (Context).IsValid())

#define EXPECT_HAS_ERRORS(Context) \
	TestTrue(TEXT(#Context " has errors"), (Context).ValidationErrors.Num() > 0)

#define EXPECT_NO_ERRORS(Context) \
	TestTrue(TEXT(#Context " has no errors"), (Context).ValidationErrors.Num() == 0)

// Context-aware assertions
#define EXPECT_WITH_CONTEXT(Expression, ContextMessage) \
	if (!TestTrue(TEXT(#Expression), Expression)) \
	{ \
		AddError(FString::Printf(TEXT("Context: %s"), *FString(ContextMessage))); \
	}

#define EXPECT_VALIDATION_SUCCESS(Context) \
	if (!(Context).IsValid()) \
	{ \
		AddError(FString::Printf(TEXT("Validation failed: %s"), *(Context).GetReport())); \
	}
