// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepCharacterBlueprintLibrary.h"
#include "Character/DelveDeepCharacter.h"
#include "Character/DelveDeepWarrior.h"
#include "Character/DelveDeepRanger.h"
#include "Character/DelveDeepMage.h"
#include "Character/DelveDeepNecromancer.h"
#include "Character/DelveDeepStatsComponent.h"
#include "Character/DelveDeepAbilitiesComponent.h"
#include "Character/DelveDeepEquipmentComponent.h"
#include "DelveDeepValidation.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepCharacter, Log, All);

ADelveDeepCharacter* UDelveDeepCharacterBlueprintLibrary::SpawnCharacter(
	const UObject* WorldContextObject,
	TSubclassOf<ADelveDeepCharacter> CharacterClass,
	FVector Location,
	FRotator Rotation)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("SpawnCharacter: Invalid world context object"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("SpawnCharacter: Failed to get world from context object"));
		return nullptr;
	}

	if (!CharacterClass)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("SpawnCharacter: Invalid character class"));
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
		UE_LOG(LogDelveDeepCharacter, Display, TEXT("SpawnCharacter: Successfully spawned character at %s"), 
			*Location.ToString());
	}
	else
	{
		UE_LOG(LogDelveDeepCharacter, Error, TEXT("SpawnCharacter: Failed to spawn character"));
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
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("ApplyDamageToCharacter: Invalid character"));
		return;
	}

	if (DamageAmount < 0.0f)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("ApplyDamageToCharacter: Negative damage amount: %.2f"), DamageAmount);
		return;
	}

	Character->TakeDamage(DamageAmount, DamageSource);
}

void UDelveDeepCharacterBlueprintLibrary::HealCharacter(ADelveDeepCharacter* Character, float HealAmount)
{
	if (!Character)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("HealCharacter: Invalid character"));
		return;
	}

	if (HealAmount < 0.0f)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("HealCharacter: Negative heal amount: %.2f"), HealAmount);
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
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("AddTemporaryStatBoost: Invalid character"));
		return;
	}

	if (StatName.IsNone())
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("AddTemporaryStatBoost: Invalid stat name"));
		return;
	}

	UDelveDeepStatsComponent* StatsComponent = Character->GetStatsComponent();
	if (!StatsComponent)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("AddTemporaryStatBoost: Character has no stats component"));
		return;
	}

	StatsComponent->AddStatModifier(StatName, Modifier, Duration);
	
	UE_LOG(LogDelveDeepCharacter, Display, TEXT("AddTemporaryStatBoost: Added %s modifier %.2f for %.2f seconds"), 
		*StatName.ToString(), Modifier, Duration);
}

// ============================================================================
// Console Command Implementations
// ============================================================================

namespace DelveDeepCharacterConsoleCommands
{
	/**
	 * Helper function to get the player character from the world.
	 */
	static ADelveDeepCharacter* GetPlayerCharacter(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (!PlayerController)
		{
			return nullptr;
		}

		return Cast<ADelveDeepCharacter>(PlayerController->GetPawn());
	}

	/**
	 * Helper function to get character class from string name.
	 */
	static TSubclassOf<ADelveDeepCharacter> GetCharacterClassFromName(const FString& ClassName)
	{
		if (ClassName.Equals(TEXT("Warrior"), ESearchCase::IgnoreCase))
		{
			return ADelveDeepWarrior::StaticClass();
		}
		else if (ClassName.Equals(TEXT("Ranger"), ESearchCase::IgnoreCase))
		{
			return ADelveDeepRanger::StaticClass();
		}
		else if (ClassName.Equals(TEXT("Mage"), ESearchCase::IgnoreCase))
		{
			return ADelveDeepMage::StaticClass();
		}
		else if (ClassName.Equals(TEXT("Necromancer"), ESearchCase::IgnoreCase))
		{
			return ADelveDeepNecromancer::StaticClass();
		}

		return nullptr;
	}

	/**
	 * Console command: Spawn a character at player location.
	 * Usage: DelveDeep.SpawnCharacter <ClassName>
	 * Example: DelveDeep.SpawnCharacter Warrior
	 */
	static void SpawnCharacterCommand(const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("SpawnCharacter: Missing class name argument. Usage: DelveDeep.SpawnCharacter <ClassName>"));
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("Available classes: Warrior, Ranger, Mage, Necromancer"));
			return;
		}

		UWorld* World = GEngine->GetWorldContexts()[0].World();
		if (!World)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("SpawnCharacter: Failed to get world"));
			return;
		}

		TSubclassOf<ADelveDeepCharacter> CharacterClass = GetCharacterClassFromName(Args[0]);
		if (!CharacterClass)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("SpawnCharacter: Invalid class name '%s'"), *Args[0]);
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("Available classes: Warrior, Ranger, Mage, Necromancer"));
			return;
		}

		// Get player location
		FVector SpawnLocation = FVector::ZeroVector;
		FRotator SpawnRotation = FRotator::ZeroRotator;

		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController && PlayerController->GetPawn())
		{
			SpawnLocation = PlayerController->GetPawn()->GetActorLocation() + FVector(100.0f, 0.0f, 0.0f);
			SpawnRotation = PlayerController->GetPawn()->GetActorRotation();
		}

		// Spawn character
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ADelveDeepCharacter* SpawnedCharacter = World->SpawnActor<ADelveDeepCharacter>(
			CharacterClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams);

		if (SpawnedCharacter)
		{
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("SpawnCharacter: Successfully spawned %s at %s"),
				*Args[0], *SpawnLocation.ToString());
		}
		else
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("SpawnCharacter: Failed to spawn character"));
		}
	}

	/**
	 * Console command: Modify player character health.
	 * Usage: DelveDeep.ModifyHealth <Amount>
	 * Example: DelveDeep.ModifyHealth 50
	 */
	static void ModifyHealthCommand(const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ModifyHealth: Missing amount argument. Usage: DelveDeep.ModifyHealth <Amount>"));
			return;
		}

		UWorld* World = GEngine->GetWorldContexts()[0].World();
		if (!World)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ModifyHealth: Failed to get world"));
			return;
		}

		ADelveDeepCharacter* Character = GetPlayerCharacter(World);
		if (!Character)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ModifyHealth: No player character found"));
			return;
		}

		float Amount = FCString::Atof(*Args[0]);
		if (Amount == 0.0f && !Args[0].Equals(TEXT("0")))
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ModifyHealth: Invalid amount '%s'"), *Args[0]);
			return;
		}

		UDelveDeepStatsComponent* StatsComponent = Character->GetStatsComponent();
		if (!StatsComponent)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ModifyHealth: Character has no stats component"));
			return;
		}

		float OldHealth = StatsComponent->GetCurrentHealth();
		StatsComponent->ModifyHealth(Amount);
		float NewHealth = StatsComponent->GetCurrentHealth();

		UE_LOG(LogDelveDeepCharacter, Display, TEXT("ModifyHealth: Changed health from %.2f to %.2f (delta: %.2f)"),
			OldHealth, NewHealth, Amount);
	}

	/**
	 * Console command: Modify player character resource (Mana/Energy/Rage/Souls).
	 * Usage: DelveDeep.ModifyResource <Amount>
	 * Example: DelveDeep.ModifyResource 25
	 */
	static void ModifyResourceCommand(const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ModifyResource: Missing amount argument. Usage: DelveDeep.ModifyResource <Amount>"));
			return;
		}

		UWorld* World = GEngine->GetWorldContexts()[0].World();
		if (!World)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ModifyResource: Failed to get world"));
			return;
		}

		ADelveDeepCharacter* Character = GetPlayerCharacter(World);
		if (!Character)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ModifyResource: No player character found"));
			return;
		}

		float Amount = FCString::Atof(*Args[0]);
		if (Amount == 0.0f && !Args[0].Equals(TEXT("0")))
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ModifyResource: Invalid amount '%s'"), *Args[0]);
			return;
		}

		UDelveDeepStatsComponent* StatsComponent = Character->GetStatsComponent();
		if (!StatsComponent)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ModifyResource: Character has no stats component"));
			return;
		}

		float OldResource = StatsComponent->GetCurrentResource();
		StatsComponent->ModifyResource(Amount);
		float NewResource = StatsComponent->GetCurrentResource();

		UE_LOG(LogDelveDeepCharacter, Display, TEXT("ModifyResource: Changed resource from %.2f to %.2f (delta: %.2f)"),
			OldResource, NewResource, Amount);
	}

	/**
	 * Console command: Deal damage to player character.
	 * Usage: DelveDeep.DealDamage <Amount>
	 * Example: DelveDeep.DealDamage 25
	 */
	static void DealDamageCommand(const TArray<FString>& Args)
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("DealDamage: Missing amount argument. Usage: DelveDeep.DealDamage <Amount>"));
			return;
		}

		UWorld* World = GEngine->GetWorldContexts()[0].World();
		if (!World)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("DealDamage: Failed to get world"));
			return;
		}

		ADelveDeepCharacter* Character = GetPlayerCharacter(World);
		if (!Character)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("DealDamage: No player character found"));
			return;
		}

		float Amount = FCString::Atof(*Args[0]);
		if (Amount <= 0.0f)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("DealDamage: Invalid damage amount '%s' (must be positive)"), *Args[0]);
			return;
		}

		float OldHealth = Character->GetCurrentHealth();
		Character->TakeDamage(Amount, nullptr);
		float NewHealth = Character->GetCurrentHealth();

		UE_LOG(LogDelveDeepCharacter, Display, TEXT("DealDamage: Dealt %.2f damage. Health: %.2f -> %.2f"),
			Amount, OldHealth, NewHealth);

		if (Character->IsDead())
		{
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("DealDamage: Character died from damage"));
		}
	}

	/**
	 * Console command: Display player character state.
	 * Usage: DelveDeep.ShowCharacterState
	 */
	static void ShowCharacterStateCommand()
	{
		UWorld* World = GEngine->GetWorldContexts()[0].World();
		if (!World)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ShowCharacterState: Failed to get world"));
			return;
		}

		ADelveDeepCharacter* Character = GetPlayerCharacter(World);
		if (!Character)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("ShowCharacterState: No player character found"));
			return;
		}

		UE_LOG(LogDelveDeepCharacter, Display, TEXT("========================================"));
		UE_LOG(LogDelveDeepCharacter, Display, TEXT("Character State: %s"), *Character->GetName());
		UE_LOG(LogDelveDeepCharacter, Display, TEXT("========================================"));

		// Basic info
		UE_LOG(LogDelveDeepCharacter, Display, TEXT("Class: %s"), *Character->GetClass()->GetName());
		UE_LOG(LogDelveDeepCharacter, Display, TEXT("Location: %s"), *Character->GetActorLocation().ToString());
		UE_LOG(LogDelveDeepCharacter, Display, TEXT("Is Dead: %s"), Character->IsDead() ? TEXT("Yes") : TEXT("No"));

		// Stats
		if (UDelveDeepStatsComponent* StatsComponent = Character->GetStatsComponent())
		{
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("\nStats:"));
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("  Health: %.2f / %.2f (%.1f%%)"),
				StatsComponent->GetCurrentHealth(),
				StatsComponent->GetMaxHealth(),
				(StatsComponent->GetMaxHealth() > 0.0f ? (StatsComponent->GetCurrentHealth() / StatsComponent->GetMaxHealth() * 100.0f) : 0.0f));
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("  Resource: %.2f / %.2f (%.1f%%)"),
				StatsComponent->GetCurrentResource(),
				StatsComponent->GetMaxResource(),
				(StatsComponent->GetMaxResource() > 0.0f ? (StatsComponent->GetCurrentResource() / StatsComponent->GetMaxResource() * 100.0f) : 0.0f));
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("  Base Damage: %.2f"), StatsComponent->GetBaseDamage());
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("  Move Speed: %.2f"), StatsComponent->GetMoveSpeed());
		}
		else
		{
			UE_LOG(LogDelveDeepCharacter, Warning, TEXT("\nStats: No stats component found"));
		}

		// Abilities
		if (UDelveDeepAbilitiesComponent* AbilitiesComponent = Character->GetAbilitiesComponent())
		{
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("\nAbilities:"));
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("  Ability Count: %d"), AbilitiesComponent->GetAbilityCount());
		}
		else
		{
			UE_LOG(LogDelveDeepCharacter, Warning, TEXT("\nAbilities: No abilities component found"));
		}

		// Equipment
		if (UDelveDeepEquipmentComponent* EquipmentComponent = Character->GetEquipmentComponent())
		{
			UE_LOG(LogDelveDeepCharacter, Display, TEXT("\nEquipment:"));
			if (const UDelveDeepWeaponData* Weapon = EquipmentComponent->GetCurrentWeapon())
			{
				UE_LOG(LogDelveDeepCharacter, Display, TEXT("  Weapon: %s"), *Weapon->GetName());
			}
			else
			{
				UE_LOG(LogDelveDeepCharacter, Display, TEXT("  Weapon: None"));
			}
		}
		else
		{
			UE_LOG(LogDelveDeepCharacter, Warning, TEXT("\nEquipment: No equipment component found"));
		}

		UE_LOG(LogDelveDeepCharacter, Display, TEXT("========================================"));
	}

	/**
	 * Console command: Kill player character.
	 * Usage: DelveDeep.KillCharacter
	 */
	static void KillCharacterCommand()
	{
		UWorld* World = GEngine->GetWorldContexts()[0].World();
		if (!World)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("KillCharacter: Failed to get world"));
			return;
		}

		ADelveDeepCharacter* Character = GetPlayerCharacter(World);
		if (!Character)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("KillCharacter: No player character found"));
			return;
		}

		if (Character->IsDead())
		{
			UE_LOG(LogDelveDeepCharacter, Warning, TEXT("KillCharacter: Character is already dead"));
			return;
		}

		Character->Die();
		UE_LOG(LogDelveDeepCharacter, Display, TEXT("KillCharacter: Character killed"));
	}

	/**
	 * Console command: Respawn player character.
	 * Usage: DelveDeep.RespawnCharacter
	 */
	static void RespawnCharacterCommand()
	{
		UWorld* World = GEngine->GetWorldContexts()[0].World();
		if (!World)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("RespawnCharacter: Failed to get world"));
			return;
		}

		ADelveDeepCharacter* Character = GetPlayerCharacter(World);
		if (!Character)
		{
			UE_LOG(LogDelveDeepCharacter, Error, TEXT("RespawnCharacter: No player character found"));
			return;
		}

		if (!Character->IsDead())
		{
			UE_LOG(LogDelveDeepCharacter, Warning, TEXT("RespawnCharacter: Character is not dead"));
			return;
		}

		Character->Respawn();
		UE_LOG(LogDelveDeepCharacter, Display, TEXT("RespawnCharacter: Character respawned"));
	}
}

// ============================================================================
// Console Command Registration
// ============================================================================

static FAutoConsoleCommand SpawnCharacterCmd(
	TEXT("DelveDeep.SpawnCharacter"),
	TEXT("Spawns a character at player location. Usage: DelveDeep.SpawnCharacter <ClassName> (Warrior, Ranger, Mage, Necromancer)"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&DelveDeepCharacterConsoleCommands::SpawnCharacterCommand));

static FAutoConsoleCommand ModifyHealthCmd(
	TEXT("DelveDeep.ModifyHealth"),
	TEXT("Modifies character health. Usage: DelveDeep.ModifyHealth <Amount>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&DelveDeepCharacterConsoleCommands::ModifyHealthCommand));

static FAutoConsoleCommand ModifyResourceCmd(
	TEXT("DelveDeep.ModifyResource"),
	TEXT("Modifies character resource (Mana/Energy/Rage/Souls). Usage: DelveDeep.ModifyResource <Amount>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&DelveDeepCharacterConsoleCommands::ModifyResourceCommand));

static FAutoConsoleCommand DealDamageCmd(
	TEXT("DelveDeep.DealDamage"),
	TEXT("Deals damage to character. Usage: DelveDeep.DealDamage <Amount>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&DelveDeepCharacterConsoleCommands::DealDamageCommand));

static FAutoConsoleCommand ShowCharacterStateCmd(
	TEXT("DelveDeep.ShowCharacterState"),
	TEXT("Displays character state (stats, abilities, equipment)"),
	FConsoleCommandDelegate::CreateStatic(&DelveDeepCharacterConsoleCommands::ShowCharacterStateCommand));

static FAutoConsoleCommand KillCharacterCmd(
	TEXT("DelveDeep.KillCharacter"),
	TEXT("Kills the player character"),
	FConsoleCommandDelegate::CreateStatic(&DelveDeepCharacterConsoleCommands::KillCharacterCommand));

static FAutoConsoleCommand RespawnCharacterCmd(
	TEXT("DelveDeep.RespawnCharacter"),
	TEXT("Respawns the player character"),
	FConsoleCommandDelegate::CreateStatic(&DelveDeepCharacterConsoleCommands::RespawnCharacterCommand));
