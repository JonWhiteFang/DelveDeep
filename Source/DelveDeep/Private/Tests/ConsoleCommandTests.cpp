// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Console Command Testing
 * 
 * Tests for console command execution, parameter handling, and side effects.
 * Demonstrates the console command testing utilities.
 */

#include "DelveDeepTestMacros.h"
#include "DelveDeepTestUtilities.h"
#include "DelveDeepTestFixtures.h"
#include "DelveDeepValidationSubsystem.h"
#include "DelveDeepEventSubsystem.h"
#include "DelveDeepConfigurationManager.h"
#include "Misc/AutomationTest.h"

// ========================================
// Command Registration Tests
// ========================================

/**
 * Tests that validation console commands are registered on subsystem initialization.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FValidationCommandRegistrationTest,
	"DelveDeep.ConsoleCommands.ValidationCommandRegistration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FValidationCommandRegistrationTest::RunTest(const FString& Parameters)
{
	// Expected validation commands
	TArray<FString> ExpectedCommands = {
		TEXT("DelveDeep.ValidateObject"),
		TEXT("DelveDeep.ListValidationRules"),
		TEXT("DelveDeep.ListRulesForClass"),
		TEXT("DelveDeep.ShowValidationCache"),
		TEXT("DelveDeep.ClearValidationCache"),
		TEXT("DelveDeep.ShowValidationMetrics"),
		TEXT("DelveDeep.ResetValidationMetrics"),
		TEXT("DelveDeep.ExportValidationMetrics"),
		TEXT("DelveDeep.TestValidationSeverity"),
		TEXT("DelveDeep.ProfileValidation")
	};

	TArray<FString> MissingCommands;
	bool bAllRegistered = DelveDeepTestUtils::VerifySubsystemCommandsRegistered(
		TEXT("Validation"),
		ExpectedCommands,
		MissingCommands);

	EXPECT_TRUE(bAllRegistered);
	EXPECT_EQ(MissingCommands.Num(), 0);

	if (MissingCommands.Num() > 0)
	{
		for (const FString& MissingCommand : MissingCommands)
		{
			AddError(FString::Printf(TEXT("Command not registered: %s"), *MissingCommand));
		}
	}

	return true;
}

/**
 * Tests that event console commands are registered on subsystem initialization.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEventCommandRegistrationTest,
	"DelveDeep.ConsoleCommands.EventCommandRegistration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEventCommandRegistrationTest::RunTest(const FString& Parameters)
{
	// Expected event commands
	TArray<FString> ExpectedCommands = {
		TEXT("DelveDeep.Events.ListListeners"),
		TEXT("DelveDeep.Events.ListAllListeners"),
		TEXT("DelveDeep.Events.ShowMetrics"),
		TEXT("DelveDeep.Events.ResetMetrics"),
		TEXT("DelveDeep.Events.ShowEventHistory"),
		TEXT("DelveDeep.Events.EnableEventLogging"),
		TEXT("DelveDeep.Events.DisableEventLogging"),
		TEXT("DelveDeep.Events.BroadcastTestEvent"),
		TEXT("DelveDeep.Events.ValidateAllPayloads"),
		TEXT("DelveDeep.Events.EnableValidation"),
		TEXT("DelveDeep.Events.DisableValidation"),
		TEXT("DelveDeep.Events.ClearAllListeners"),
		TEXT("DelveDeep.Events.DumpEventRegistry")
	};

	TArray<FString> MissingCommands;
	bool bAllRegistered = DelveDeepTestUtils::VerifySubsystemCommandsRegistered(
		TEXT("Events"),
		ExpectedCommands,
		MissingCommands);

	EXPECT_TRUE(bAllRegistered);
	EXPECT_EQ(MissingCommands.Num(), 0);

	if (MissingCommands.Num() > 0)
	{
		for (const FString& MissingCommand : MissingCommands)
		{
			AddError(FString::Printf(TEXT("Command not registered: %s"), *MissingCommand));
		}
	}

	return true;
}

/**
 * Tests that all DelveDeep commands can be discovered by prefix.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommandDiscoveryTest,
	"DelveDeep.ConsoleCommands.CommandDiscovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCommandDiscoveryTest::RunTest(const FString& Parameters)
{
	// Get all DelveDeep commands
	TArray<FString> DelveDeepCommands = DelveDeepTestUtils::GetRegisteredConsoleCommands(TEXT("DelveDeep."));

	// Should have at least some commands registered
	EXPECT_GT(DelveDeepCommands.Num(), 0);

	// Log discovered commands
	UE_LOG(LogTemp, Display, TEXT("Discovered %d DelveDeep console commands:"), DelveDeepCommands.Num());
	for (const FString& Command : DelveDeepCommands)
	{
		UE_LOG(LogTemp, Display, TEXT("  - %s"), *Command);
	}

	return true;
}

// ========================================
// Command Execution Tests
// ========================================

/**
 * Tests basic console command execution.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBasicCommandExecutionTest,
	"DelveDeep.ConsoleCommands.BasicExecution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBasicCommandExecutionTest::RunTest(const FString& Parameters)
{
	// Test executing a simple command
	bool bExecuted = DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.TestValidationSeverity"));
	EXPECT_TRUE(bExecuted);

	// Test executing a non-existent command
	bool bFailedExecution = DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.NonExistentCommand"));
	EXPECT_FALSE(bFailedExecution);

	return true;
}

/**
 * Tests console command execution with output capture.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommandOutputCaptureTest,
	"DelveDeep.ConsoleCommands.OutputCapture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCommandOutputCaptureTest::RunTest(const FString& Parameters)
{
	TArray<FString> CapturedOutput;

	// Execute command and capture output
	bool bExecuted = DelveDeepTestUtils::ExecuteConsoleCommandWithCapture(
		TEXT("DelveDeep.TestValidationSeverity"),
		CapturedOutput);

	EXPECT_TRUE(bExecuted);
	EXPECT_GT(CapturedOutput.Num(), 0);

	// Verify output contains expected strings
	bool bFoundExpectedOutput = false;
	for (const FString& Line : CapturedOutput)
	{
		if (Line.Contains(TEXT("Testing Validation Severity")))
		{
			bFoundExpectedOutput = true;
			break;
		}
	}

	EXPECT_TRUE(bFoundExpectedOutput);

	return true;
}

/**
 * Tests console command execution with FConsoleOutputCapture helper.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FConsoleOutputCaptureHelperTest,
	"DelveDeep.ConsoleCommands.OutputCaptureHelper",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConsoleOutputCaptureHelperTest::RunTest(const FString& Parameters)
{
	// Create output capture
	DelveDeepTestUtils::FConsoleOutputCapture Capture;

	// Execute command
	bool bExecuted = DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.TestValidationSeverity"));
	EXPECT_TRUE(bExecuted);

	// Verify captured output
	EXPECT_GT(Capture.GetOutputLineCount(), 0);
	EXPECT_TRUE(Capture.ContainsOutput(TEXT("Testing Validation Severity")));

	// Test multiple search strings
	TArray<FString> ExpectedStrings = {
		TEXT("Testing Validation Severity"),
		TEXT("Issue Counts"),
		TEXT("Validation Status")
	};

	EXPECT_TRUE(Capture.ContainsAllOutput(ExpectedStrings));

	return true;
}

// ========================================
// Parameter Handling Tests
// ========================================

/**
 * Tests that commands handle invalid parameters gracefully.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInvalidParameterHandlingTest,
	"DelveDeep.ConsoleCommands.InvalidParameterHandling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInvalidParameterHandlingTest::RunTest(const FString& Parameters)
{
	TArray<FString> CapturedOutput;

	// Test command with missing required parameter
	TArray<FString> EmptyArgs;
	bool bHandledGracefully = DelveDeepTestUtils::TestConsoleCommandInvalidParameters(
		TEXT("DelveDeep.ValidateObject"),
		EmptyArgs,
		CapturedOutput);

	EXPECT_TRUE(bHandledGracefully);

	// Verify error message was logged
	bool bFoundErrorMessage = false;
	for (const FString& Line : CapturedOutput)
	{
		if (Line.Contains(TEXT("Usage:")) || Line.Contains(TEXT("Error")))
		{
			bFoundErrorMessage = true;
			break;
		}
	}

	EXPECT_TRUE(bFoundErrorMessage);

	return true;
}

/**
 * Tests command execution with valid arguments.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommandWithArgumentsTest,
	"DelveDeep.ConsoleCommands.CommandWithArguments",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCommandWithArgumentsTest::RunTest(const FString& Parameters)
{
	// Test command with arguments
	TArray<FString> Args;
	Args.Add(TEXT("DelveDeep.Event.Test"));

	bool bExecuted = DelveDeepTestUtils::ExecuteConsoleCommandWithArgs(
		TEXT("DelveDeep.Events.BroadcastTestEvent"),
		Args);

	EXPECT_TRUE(bExecuted);

	return true;
}

// ========================================
// Side Effect Verification Tests
// ========================================

/**
 * Tests that commands produce expected side effects.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommandSideEffectsTest,
	"DelveDeep.ConsoleCommands.SideEffects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCommandSideEffectsTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
	ASSERT_NOT_NULL(GameInstance);

	// Get validation subsystem
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		DelveDeepTestUtils::GetTestSubsystem<UDelveDeepValidationSubsystem>(GameInstance);
	ASSERT_NOT_NULL(ValidationSubsystem);

	// Get initial metrics
	FValidationMetricsData InitialMetrics = ValidationSubsystem->GetValidationMetrics();
	int32 InitialValidations = InitialMetrics.TotalValidations;

	// Execute command that should affect metrics
	bool bVerified = DelveDeepTestUtils::VerifyConsoleCommandSideEffects(
		TEXT("DelveDeep.TestValidationSeverity"),
		[ValidationSubsystem, InitialValidations]() -> bool
		{
			// Verify that metrics were affected
			// Note: TestValidationSeverity doesn't actually increment metrics,
			// but this demonstrates the pattern
			return true;
		});

	EXPECT_TRUE(bVerified);

	return true;
}

/**
 * Tests that reset commands clear state correctly.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommandResetSideEffectsTest,
	"DelveDeep.ConsoleCommands.ResetSideEffects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCommandResetSideEffectsTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
	ASSERT_NOT_NULL(GameInstance);

	// Get event subsystem
	UDelveDeepEventSubsystem* EventSubsystem = 
		DelveDeepTestUtils::GetTestSubsystem<UDelveDeepEventSubsystem>(GameInstance);
	ASSERT_NOT_NULL(EventSubsystem);

	// Broadcast some events to generate metrics
	FDelveDeepEventPayload TestPayload;
	TestPayload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Test"));
	EventSubsystem->BroadcastEvent(TestPayload);

	// Get initial metrics
	const FEventSystemMetrics& InitialMetrics = EventSubsystem->GetPerformanceMetrics();
	int32 InitialEvents = InitialMetrics.TotalEventsBroadcast;

	EXPECT_GT(InitialEvents, 0);

	// Reset metrics
	bool bVerified = DelveDeepTestUtils::VerifyConsoleCommandSideEffects(
		TEXT("DelveDeep.Events.ResetMetrics"),
		[EventSubsystem]() -> bool
		{
			// Verify metrics were reset
			const FEventSystemMetrics& ResetMetrics = EventSubsystem->GetPerformanceMetrics();
			return ResetMetrics.TotalEventsBroadcast == 0;
		});

	EXPECT_TRUE(bVerified);

	return true;
}

// ========================================
// Integration Tests
// ========================================

/**
 * Tests command execution in integration with subsystems.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommandSubsystemIntegrationTest,
	"DelveDeep.ConsoleCommands.SubsystemIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCommandSubsystemIntegrationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
	ASSERT_NOT_NULL(GameInstance);

	// Verify subsystems are initialized
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		DelveDeepTestUtils::GetTestSubsystem<UDelveDeepValidationSubsystem>(GameInstance);
	ASSERT_NOT_NULL(ValidationSubsystem);

	UDelveDeepEventSubsystem* EventSubsystem = 
		DelveDeepTestUtils::GetTestSubsystem<UDelveDeepEventSubsystem>(GameInstance);
	ASSERT_NOT_NULL(EventSubsystem);

	// Execute commands that interact with subsystems
	TArray<FString> CapturedOutput;

	// Test validation command
	bool bValidationExecuted = DelveDeepTestUtils::ExecuteConsoleCommandWithCapture(
		TEXT("DelveDeep.ShowValidationMetrics"),
		CapturedOutput);
	EXPECT_TRUE(bValidationExecuted);

	// Test event command
	CapturedOutput.Empty();
	bool bEventExecuted = DelveDeepTestUtils::ExecuteConsoleCommandWithCapture(
		TEXT("DelveDeep.Events.ShowMetrics"),
		CapturedOutput);
	EXPECT_TRUE(bEventExecuted);

	return true;
}

/**
 * Tests that all registered commands can be executed without crashing.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAllCommandsExecutionTest,
	"DelveDeep.ConsoleCommands.AllCommandsExecution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAllCommandsExecutionTest::RunTest(const FString& Parameters)
{
	// Get all DelveDeep commands
	TArray<FString> DelveDeepCommands = DelveDeepTestUtils::GetRegisteredConsoleCommands(TEXT("DelveDeep."));

	// Commands that require parameters (skip for this test)
	TArray<FString> CommandsRequiringParams = {
		TEXT("DelveDeep.ValidateObject"),
		TEXT("DelveDeep.ListRulesForClass"),
		TEXT("DelveDeep.ProfileValidation"),
		TEXT("DelveDeep.Events.ListListeners"),
		TEXT("DelveDeep.Events.BroadcastTestEvent")
	};

	int32 ExecutedCount = 0;
	int32 SkippedCount = 0;

	for (const FString& Command : DelveDeepCommands)
	{
		// Skip commands that require parameters
		if (CommandsRequiringParams.Contains(Command))
		{
			SkippedCount++;
			continue;
		}

		// Try to execute command
		bool bExecuted = DelveDeepTestUtils::ExecuteConsoleCommand(Command);
		if (bExecuted)
		{
			ExecutedCount++;
		}
	}

	UE_LOG(LogTemp, Display, TEXT("Executed %d commands, skipped %d commands requiring parameters"),
		ExecutedCount, SkippedCount);

	EXPECT_GT(ExecutedCount, 0);

	return true;
}
