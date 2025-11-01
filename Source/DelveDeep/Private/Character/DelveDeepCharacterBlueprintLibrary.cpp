// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepCharacterBlueprintLibrary.h"
#include "Character/DelveDeepCharacter.h"
#include "Character/DelveDeepStatsComponent.h"
#include "Engine/World.h"

ADelveDeepCharacter* UDelveDeepCharacterBlueprintLibrary::SpawnCharacter(
	const UObject* WorldContextObject,
	TSubclassOf<ADelveDeepCharacter> CharacterClass,
	FVector Location,
	FRotator Rotation)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCharacter: Invalid world context object"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCharacter: Failed to get world from context object"));
		return nullptr;
	}

	if (!CharacterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCharacter: Invalid character class"));
		return nullptr;
	}

	// Spawn character
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ADelveDeepCharacter* SpawnedCharacter = World->SpawnActor<ADelveDeepCharacter>(
		CharacterClass,
		Location,
		Rotation,
		SpawnParams);

	if (SpawnedCharacter)
	{
		UE_LOG(LogTemp, Display, TEXT("SpawnCharacter: Successfully spawned character at %s"), 
			*Location.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnCharacter: Failed to spawn character"));
	}

	return SpawnedCharacter;
}

bool UDelveDeepCharacterBlueprintLibrary::IsCharacterAlive(ADelveDeepCharacter* Character)
{
	if (!Character)
	{
		return false;
	}

	return !Character->IsDead();
}

float UDelveDeepCharacterBlueprintLibrary::GetHealthPercentage(ADelveDeepCharacter* Character)
{
	if (!Character)
	{
		return 0.0f;
	}

	float MaxHealth = Character->GetMaxHealth();
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return Character->GetCurrentHealth() / MaxHealth;
}

float UDelveDeepCharacterBlueprintLibrary::GetResourcePercentage(ADelveDeepCharacter* Character)
{
	if (!Character)
	{
		return 0.0f;
	}

	float MaxResource = Character->GetMaxResource();
	if (MaxResource <= 0.0f)
	{
		return 0.0f;
	}

	return Character->GetCurrentResource() / MaxResource;
}

void UDelveDeepCharacterBlueprintLibrary::ApplyDamageToCharacter(
	ADelveDeepCharacter* Character,
	float DamageAmount,
	AActor* DamageSource)
{
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyDamageToCharacter: Invalid character"));
		return;
	}

	if (DamageAmount < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyDamageToCharacter: Negative damage amount: %.2f"), DamageAmount);
		return;
	}

	Character->TakeDamage(DamageAmount, DamageSource);
}

void UDelveDeepCharacterBlueprintLibrary::HealCharacter(ADelveDeepCharacter* Character, float HealAmount)
{
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("HealCharacter: Invalid character"));
		return;
	}

	if (HealAmount < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("HealCharacter: Negative heal amount: %.2f"), HealAmount);
		return;
	}

	Character->Heal(HealAmount);
}

void UDelveDeepCharacterBlueprintLibrary::AddTemporaryStatBoost(
	ADelveDeepCharacter* Character,
	FName StatName,
	float Modifier,
	float Duration)
{
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddTemporaryStatBoost: Invalid character"));
		return;
	}

	if (StatName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("AddTemporaryStatBoost: Invalid stat name"));
		return;
	}

	UDelveDeepStatsComponent* StatsComponent = Character->GetStatsComponent();
	if (!StatsComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddTemporaryStatBoost: Character has no stats component"));
		return;
	}

	StatsComponent->AddStatModifier(StatName, Modifier, Duration);
	
	UE_LOG(LogTemp, Display, TEXT("AddTemporaryStatBoost: Added %s modifier %.2f for %.2f seconds"), 
		*StatName.ToString(), Modifier, Duration);
}
