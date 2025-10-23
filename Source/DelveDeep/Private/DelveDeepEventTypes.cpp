// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventTypes.h"

bool FDelveDeepEventFilter::PassesFilter(const FDelveDeepEventPayload& Payload) const
{
	// If no filters are enabled, pass all events
	if (!bUseActorFilter && !bUseSpatialFilter)
	{
		return true;
	}

	// Check actor filter
	if (bUseActorFilter)
	{
		if (!SpecificActor.IsValid())
		{
			// Filter is enabled but actor is invalid - fail the filter
			return false;
		}

		// Check if the event involves the specific actor
		// This checks the Instigator field from the base payload
		if (Payload.Instigator.IsValid() && Payload.Instigator.Get() == SpecificActor.Get())
		{
			// Actor filter passed
		}
		else
		{
			// Actor filter failed
			return false;
		}
	}

	// Spatial filter requires location information from derived payload types
	// This base implementation only checks actor filter
	// Spatial filtering is handled in the overload that accepts EventLocation

	return true;
}

bool FDelveDeepEventFilter::PassesFilter(const FDelveDeepEventPayload& Payload, const FVector& EventLocation) const
{
	// First check actor filter
	if (!PassesFilter(Payload))
	{
		return false;
	}

	// Check spatial filter
	if (bUseSpatialFilter)
	{
		if (SpatialRadius <= 0.0f)
		{
			// Invalid radius - fail the filter
			return false;
		}

		// Calculate distance from filter location to event location
		float DistanceSquared = FVector::DistSquared(SpatialLocation, EventLocation);
		float RadiusSquared = SpatialRadius * SpatialRadius;

		if (DistanceSquared > RadiusSquared)
		{
			// Event is outside the spatial radius
			return false;
		}
	}

	return true;
}
