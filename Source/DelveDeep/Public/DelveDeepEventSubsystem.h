// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "DelveDeepEventTypes.h"
#include "DelveDeepEventPayload.h"
#include "DelveDeepEventSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepEvents, Log, All);

/**
 * Location information for a registered listener.
 * Used for O(1) handle-to-listener lookup.
 */
struct FListenerLocation
{
	FGameplayTag EventTag;
	EDelveDeepEventPriority Priority;
	int32 IndexInArray;
};

/**
 * Centralized event system subsystem for DelveDeep.
 * Provides a gameplay event bus using GameplayTags for loose coupling between systems.
 * Enables event-driven communication without direct dependencies.
 */
UCLASS(BlueprintType)
class DELVEDEEP_API UDelveDeepEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Registers a listener for a specific event tag.
	 * @param EventTag The GameplayTag identifying the event type to listen for
	 * @param Callback The function to invoke when the event is broadcast
	 * @param Owner The object registering the listener (for automatic cleanup)
	 * @param Priority The priority level for this listener (High, Normal, or Low)
	 * @param Filter Optional filter criteria for this listener
	 * @return Handle that can be used to unregister the listener
	 */
	FDelegateHandle RegisterListener(
		FGameplayTag EventTag,
		TFunction<void(const FDelveDeepEventPayload&)> Callback,
		UObject* Owner,
		EDelveDeepEventPriority Priority = EDelveDeepEventPriority::Normal,
		const FDelveDeepEventFilter& Filter = FDelveDeepEventFilter()
	);

	/**
	 * Unregisters a listener using its handle.
	 * @param Handle The handle returned from RegisterListener
	 */
	void UnregisterListener(FDelegateHandle Handle);

	/**
	 * Unregisters all listeners owned by a specific object.
	 * @param Owner The object whose listeners should be unregistered
	 */
	void UnregisterAllListeners(UObject* Owner);

	/**
	 * Broadcasts an event to all registered listeners.
	 * Listeners are invoked in priority order (High -> Normal -> Low).
	 * @param Payload The event payload containing event data
	 */
	void BroadcastEvent(const FDelveDeepEventPayload& Payload);

	/**
	 * Gets the number of listeners registered for a specific event tag.
	 * @param EventTag The event tag to query
	 * @return Number of listeners registered for this tag
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
	int32 GetListenerCount(FGameplayTag EventTag) const;

private:
	/** Event registry: Maps GameplayTag to listener lists */
	UPROPERTY()
	TMap<FGameplayTag, FEventListenerList> EventRegistry;

	/** Maps delegate handles to listener locations for O(1) lookup */
	TMap<FDelegateHandle, FListenerLocation> HandleToListenerMap;

	/** Counter for generating unique delegate handles */
	int32 NextHandleId = 1;

	/**
	 * Generates a unique delegate handle.
	 * @return A new unique FDelegateHandle
	 */
	FDelegateHandle GenerateUniqueHandle();

	/**
	 * Removes stale listeners with invalid owners from the registry.
	 * Called during low-activity frames for lazy cleanup.
	 */
	void CleanupStaleListeners();

	/**
	 * Removes a listener from a specific priority array.
	 * @param ListenerArray The array to remove from
	 * @param Index The index of the listener to remove
	 * @param EventTag The event tag (for logging)
	 * @param Priority The priority level (for logging)
	 */
	void RemoveListenerFromArray(
		TArray<FDelveDeepEventListener>& ListenerArray,
		int32 Index,
		const FGameplayTag& EventTag,
		EDelveDeepEventPriority Priority
	);
};
