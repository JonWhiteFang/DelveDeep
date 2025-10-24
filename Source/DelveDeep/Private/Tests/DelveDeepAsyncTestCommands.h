// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "TimerManager.h"

/**
 * DelveDeep Async Test Commands
 * 
 * Latent automation commands for testing asynchronous operations.
 * These commands integrate with Unreal's latent command system to enable
 * testing of timers, callbacks, and other async operations.
 * 
 * Usage:
 *   ADD_ADVANCE_TIME(DeltaTime) - Advance game time
 *   ADD_WAIT_FOR_CONDITION(Condition, Timeout) - Wait for condition
 *   ADD_VERIFY_CALLBACK(CallbackFlag) - Verify callback invocation
 */

// ========================================
// Advance Time Command
// ========================================

/**
 * Latent command that advances game time without real-time delay.
 * Useful for testing timer-based operations.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FAdvanceTimeCommand,
	float, DeltaTime);

bool FAdvanceTimeCommand::Update()
{
	// Advance world time by ticking the timer manager
	if (UWorld* World = GEngine->GetWorldContexts()[0].World())
	{
		World->GetTimerManager().Tick(DeltaTime);
	}
	return true;
}

// ========================================
// Wait For Condition Command
// ========================================

/**
 * Latent command that waits for a condition to become true.
 * Includes timeout protection to prevent hanging tests.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FWaitForConditionCommand,
	TFunction<bool()>, Condition,
	float, TimeoutSeconds,
	double, StartTime);

bool FWaitForConditionCommand::Update()
{
	// Check if condition is met
	if (Condition())
	{
		return true;
	}

	// Track elapsed time
	double CurrentTime = FPlatformTime::Seconds();
	double ElapsedTime = CurrentTime - StartTime;

	// Check for timeout
	if (ElapsedTime >= TimeoutSeconds)
	{
		UE_LOG(LogTemp, Error, TEXT("Wait condition timed out after %.2f seconds"), TimeoutSeconds);
		return true;  // Return true to stop waiting
	}

	return false;  // Continue waiting
}

// ========================================
// Verify Callback Command
// ========================================

/**
 * Latent command that verifies a callback was invoked.
 * Uses a shared boolean flag to track callback invocation.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FVerifyCallbackCommand,
	TSharedPtr<bool>, bCallbackInvoked);

bool FVerifyCallbackCommand::Update()
{
	// Check if callback was invoked
	return bCallbackInvoked.IsValid() && *bCallbackInvoked;
}

// ========================================
// Helper Macros
// ========================================

/**
 * Macro for advancing game time in tests.
 * 
 * @param DeltaTime Time to advance in seconds
 * 
 * Example:
 *   ADD_ADVANCE_TIME(1.0f);  // Advance 1 second
 */
#define ADD_ADVANCE_TIME(DeltaTime) \
	ADD_LATENT_AUTOMATION_COMMAND(FAdvanceTimeCommand(DeltaTime))

/**
 * Macro for waiting on a condition with timeout.
 * 
 * @param Condition Lambda or function returning bool
 * @param Timeout Maximum wait time in seconds
 * 
 * Example:
 *   ADD_WAIT_FOR_CONDITION([&]() { return bEventReceived; }, 5.0f);
 */
#define ADD_WAIT_FOR_CONDITION(Condition, Timeout) \
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForConditionCommand(Condition, Timeout, FPlatformTime::Seconds()))

/**
 * Macro for verifying callback invocation.
 * 
 * @param CallbackFlag Shared pointer to boolean flag
 * 
 * Example:
 *   TSharedPtr<bool> bCalled = MakeShared<bool>(false);
 *   // ... register callback that sets *bCalled = true ...
 *   ADD_VERIFY_CALLBACK(bCalled);
 */
#define ADD_VERIFY_CALLBACK(CallbackFlag) \
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyCallbackCommand(CallbackFlag))

// ========================================
// Advanced Async Commands
// ========================================

/**
 * Latent command that waits for a specific number of frames.
 * Useful for testing frame-dependent operations.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FWaitFramesCommand,
	int32, FrameCount,
	int32, CurrentFrame);

bool FWaitFramesCommand::Update()
{
	CurrentFrame++;
	
	if (CurrentFrame >= FrameCount)
	{
		return true;
	}
	
	return false;
}

/**
 * Macro for waiting a specific number of frames.
 * 
 * @param FrameCount Number of frames to wait
 * 
 * Example:
 *   ADD_WAIT_FRAMES(5);  // Wait 5 frames
 */
#define ADD_WAIT_FRAMES(FrameCount) \
	ADD_LATENT_AUTOMATION_COMMAND(FWaitFramesCommand(FrameCount, 0))

/**
 * Latent command that executes a function after a delay.
 * Useful for testing deferred operations.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FDelayedExecutionCommand,
	TFunction<void()>, Function,
	float, DelaySeconds,
	double, StartTime);

bool FDelayedExecutionCommand::Update()
{
	double CurrentTime = FPlatformTime::Seconds();
	double ElapsedTime = CurrentTime - StartTime;

	if (ElapsedTime >= DelaySeconds)
	{
		if (Function)
		{
			Function();
		}
		return true;
	}

	return false;
}

/**
 * Macro for executing a function after a delay.
 * 
 * @param Function Lambda or function to execute
 * @param Delay Delay in seconds
 * 
 * Example:
 *   ADD_DELAYED_EXECUTION([&]() { DoSomething(); }, 2.0f);
 */
#define ADD_DELAYED_EXECUTION(Function, Delay) \
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedExecutionCommand(Function, Delay, FPlatformTime::Seconds()))
