// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepCharacter.h"
#include "Character/DelveDeepCharacterComponent.h"
#include "Character/DelveDeepStatsComponent.h"
#include "Character/DelveDeepAbilitiesComponent.h"
#include "Character/DelveDeepEquipmentComponent.h"
#include "Configuration/DelveDeepConfigurationManager.h"
#include "Configuration/DelveDeepCharacterData.h"
#include "Validation/ValidationContext.h"
#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventPayload.h"
#include "GameplayTagsManager.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "PaperFlipbookComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepCharacter, Log, All);

ADelveDeepCharacter::ADelveDeepCharacter()
{
	// Disable tick by default for performance
	PrimaryActorTick.bCanEverTick = false;

	// Create stats component
	StatsComponent = CreateDefaultSubobject<UDelveDeepStatsComponent>(TEXT("StatsComponent"));

	// Create abilities component
	AbilitiesComponent = CreateDefaultSubobject<UDelveDeepAbilitiesComponent>(TEXT("AbilitiesComponent"));

	// Create equipment component
	EquipmentComponent = CreateDefaultSubobject<UDelveDeepEquipmentComponent>(TEXT("EquipmentComponent"));

	// Initialize character data to nullptr
	CharacterData = nullptr;

	// Initialize death flag
	bIsDead = false;
}

void ADelveDeepCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize character from configuration data
	InitializeFromData();
}

void ADelveDeepCharacter::InitializeFromData()
{
	// Validate character class name is set
	if (CharacterClassName.IsNone())
	{
		UE_LOG(LogDelveDeepCharacter, Error, TEXT("Character class name not set for %s"), *GetName());
		return;
	}

	// Get configuration manager
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogDelveDeepCharacter, Error, TEXT("Failed to get game instance for %s"), *GetName());
		return;
	}

	UDelveDeepConfigurationManager* ConfigManager = GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
	if (!ConfigManager)
	{
		UE_LOG(LogDelveDeepCharacter, Error, TEXT("Failed to get configuration manager for %s"), *GetName());
		return;
	}

	// Query character data
	CharacterData = ConfigManager->GetCharacterData(CharacterClassName);
	if (!CharacterData)
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("Failed to load character data for class '%s' on %s"), 
			*CharacterClassName.ToString(), *GetName());
		return;
	}

	// Validate character data
	FValidationContext Context;
	Context.SystemName = TEXT("Character");
	Context.OperationName = TEXT("InitializeFromData");

	if (!ValidateCharacterData(Context))
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("Character data validation failed for %s: %s"), 
			*GetName(), *Context.GetReport());
		
		// Continue with fallback values rather than failing completely
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Using fallback values for %s"), *GetName());
	}

	// Initialize components with character data
	InitializeComponents();

	UE_LOG(LogDelveDeepCharacter, Display, 
		TEXT("Character initialized: %s (Class: %s)"), 
		*GetName(), *CharacterClassName.ToString());
}

bool ADelveDeepCharacter::ValidateCharacterData(FValidationContext& Context) const
{
	bool bIsValid = true;

	// Validate character data exists
	if (!CharacterData)
	{
		Context.AddError(TEXT("Character data is null"));
		return false;
	}

	// Validate base stats are in reasonable ranges
	if (CharacterData->BaseHealth <= 0.0f || CharacterData->BaseHealth > 10000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseHealth out of range: %.2f (expected 1-10000)"), 
			CharacterData->BaseHealth));
		bIsValid = false;
	}

	if (CharacterData->BaseDamage < 0.0f || CharacterData->BaseDamage > 1000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseDamage out of range: %.2f (expected 0-1000)"), 
			CharacterData->BaseDamage));
		bIsValid = false;
	}

	if (CharacterData->BaseMoveSpeed <= 0.0f || CharacterData->BaseMoveSpeed > 2000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseMoveSpeed out of range: %.2f (expected 1-2000)"), 
			CharacterData->BaseMoveSpeed));
		bIsValid = false;
	}

	// Validate starting weapon reference (warning only, not critical)
	if (CharacterData->StartingWeapon.IsNull())
	{
		Context.AddWarning(TEXT("No starting weapon assigned"));
	}

	// Validate starting abilities (warning only)
	if (CharacterData->StartingAbilities.Num() == 0)
	{
		Context.AddWarning(TEXT("No starting abilities assigned"));
	}

	return bIsValid;
}

void ADelveDeepCharacter::InitializeComponents()
{
	// Validate components exist
	if (!StatsComponent || !AbilitiesComponent || !EquipmentComponent)
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("One or more components missing on %s"), *GetName());
		return;
	}

	// Validate character data exists
	if (!CharacterData)
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("Cannot initialize components without character data on %s"), *GetName());
		return;
	}

	// Initialize stats component
	StatsComponent->InitializeFromCharacterData(CharacterData);

	// Initialize abilities component
	AbilitiesComponent->InitializeFromCharacterData(CharacterData);

	// Initialize equipment component
	EquipmentComponent->InitializeFromCharacterData(CharacterData);

	UE_LOG(LogDelveDeepCharacter, Verbose, 
		TEXT("Components initialized for %s"), *GetName());
}

void ADelveDeepCharacter::TakeDamage(float DamageAmount, AActor* DamageSource)
{
	// Validate damage amount
	if (DamageAmount < 0.0f)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, 
			TEXT("Attempted to apply negative damage: %.2f"), DamageAmount);
		return;
	}

	// Cannot damage dead characters
	if (bIsDead)
	{
		return;
	}

	// Validate stats component
	if (!StatsComponent)
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("Cannot apply damage without stats component on %s"), *GetName());
		return;
	}

	// Apply damage to health
	StatsComponent->ModifyHealth(-DamageAmount);

	// Broadcast damage event
	BroadcastDamageEvent(DamageAmount, DamageSource);

	// Call Blueprint event
	OnDamaged(DamageAmount, DamageSource);

	// Apply visual feedback (sprite flash)
	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		// Flash red for damage feedback
		SpriteComponent->SetSpriteColor(FLinearColor(1.0f, 0.5f, 0.5f, 1.0f));
		
		// Reset color after 0.1 seconds
		FTimerHandle FlashTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			FlashTimerHandle,
			[this, SpriteComponent]()
			{
				if (SpriteComponent)
				{
					SpriteComponent->SetSpriteColor(FLinearColor::White);
				}
			},
			0.1f,
			false
		);
	}

	// Check for death
	if (StatsComponent->GetCurrentHealth() <= 0.0f)
	{
		Die();
	}

	UE_LOG(LogDelveDeepCharacter, Verbose, 
		TEXT("%s took %.2f damage from %s"), 
		*GetName(), DamageAmount, DamageSource ? *DamageSource->GetName() : TEXT("Unknown"));
}

void ADelveDeepCharacter::Heal(float HealAmount)
{
	// Validate heal amount
	if (HealAmount < 0.0f)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, 
			TEXT("Attempted to apply negative healing: %.2f"), HealAmount);
		return;
	}

	// Cannot heal dead characters
	if (bIsDead)
	{
		return;
	}

	// Validate stats component
	if (!StatsComponent)
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("Cannot apply healing without stats component on %s"), *GetName());
		return;
	}

	// Apply healing to health
	StatsComponent->ModifyHealth(HealAmount);

	// Broadcast heal event
	BroadcastHealEvent(HealAmount);

	// Call Blueprint event
	OnHealed(HealAmount);

	// Apply visual feedback (sprite glow)
	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		// Glow green for healing feedback
		SpriteComponent->SetSpriteColor(FLinearColor(0.5f, 1.0f, 0.5f, 1.0f));
		
		// Reset color after 0.2 seconds
		FTimerHandle GlowTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			GlowTimerHandle,
			[this, SpriteComponent]()
			{
				if (SpriteComponent)
				{
					SpriteComponent->SetSpriteColor(FLinearColor::White);
				}
			},
			0.2f,
			false
		);
	}

	UE_LOG(LogDelveDeepCharacter, Verbose, 
		TEXT("%s healed for %.2f"), *GetName(), HealAmount);
}

void ADelveDeepCharacter::Die()
{
	// Already dead
	if (bIsDead)
	{
		return;
	}

	// Set death flag
	bIsDead = true;

	// Disable input
	DisableInput(nullptr);

	// Disable collision
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Broadcast death event
	BroadcastDeathEvent(nullptr); // TODO: Track killer

	// Call Blueprint event
	OnDeath();

	// Set timer to destroy actor after 2 seconds
	GetWorld()->GetTimerManager().SetTimer(
		DeathTimerHandle,
		[this]()
		{
			Destroy();
		},
		2.0f,
		false
	);

	UE_LOG(LogDelveDeepCharacter, Display, TEXT("%s died"), *GetName());
}

void ADelveDeepCharacter::Respawn()
{
	// Already alive
	if (!bIsDead)
	{
		return;
	}

	// Reset death flag
	bIsDead = false;

	// Clear death timer if active
	if (DeathTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DeathTimerHandle);
	}

	// Reset stats to maximum values
	if (StatsComponent)
	{
		StatsComponent->ResetToMaxValues();
		StatsComponent->ClearAllModifiers();
	}

	// Re-enable input
	EnableInput(nullptr);

	// Re-enable collision
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// Reset sprite color
	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		SpriteComponent->SetSpriteColor(FLinearColor::White);
	}

	UE_LOG(LogDelveDeepCharacter, Display, TEXT("%s respawned"), *GetName());
}

float ADelveDeepCharacter::GetCurrentHealth() const
{
	return StatsComponent ? StatsComponent->GetCurrentHealth() : 0.0f;
}

float ADelveDeepCharacter::GetMaxHealth() const
{
	return StatsComponent ? StatsComponent->GetMaxHealth() : 0.0f;
}

float ADelveDeepCharacter::GetCurrentResource() const
{
	return StatsComponent ? StatsComponent->GetCurrentResource() : 0.0f;
}

float ADelveDeepCharacter::GetMaxResource() const
{
	return StatsComponent ? StatsComponent->GetMaxResource() : 0.0f;
}

void ADelveDeepCharacter::BroadcastDamageEvent(float DamageAmount, AActor* DamageSource)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDelveDeepEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<UDelveDeepEventSubsystem>())
		{
			FDelveDeepDamageEventPayload Payload;
			Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Character.Damaged"));
			Payload.Character = this;
			Payload.DamageAmount = DamageAmount;
			Payload.DamageSource = DamageSource;
			Payload.Instigator = DamageSource;

			// Validate payload before broadcasting
			FValidationContext Context;
			Context.SystemName = TEXT("Character");
			Context.OperationName = TEXT("BroadcastDamageEvent");

			if (!Payload.Validate(Context))
			{
				UE_LOG(LogDelveDeepCharacter, Warning,
					TEXT("Damage event payload validation failed: %s"),
					*Context.GetReport());
				// Continue broadcasting despite validation warnings
			}

			EventSubsystem->BroadcastEvent(Payload);
		}
	}
}

void ADelveDeepCharacter::BroadcastHealEvent(float HealAmount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDelveDeepEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<UDelveDeepEventSubsystem>())
		{
			FDelveDeepHealthChangeEventPayload Payload;
			Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Character.Healed"));
			Payload.Character = this;
			Payload.PreviousHealth = StatsComponent ? StatsComponent->GetCurrentHealth() - HealAmount : 0.0f;
			Payload.NewHealth = StatsComponent ? StatsComponent->GetCurrentHealth() : 0.0f;
			Payload.MaxHealth = StatsComponent ? StatsComponent->GetMaxHealth() : 0.0f;
			Payload.Instigator = this;

			// Validate payload before broadcasting
			FValidationContext Context;
			Context.SystemName = TEXT("Character");
			Context.OperationName = TEXT("BroadcastHealEvent");

			if (!Payload.Validate(Context))
			{
				UE_LOG(LogDelveDeepCharacter, Warning,
					TEXT("Heal event payload validation failed: %s"),
					*Context.GetReport());
				// Continue broadcasting despite validation warnings
			}

			EventSubsystem->BroadcastEvent(Payload);
		}
	}
}

void ADelveDeepCharacter::BroadcastDeathEvent(AActor* Killer)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDelveDeepEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<UDelveDeepEventSubsystem>())
		{
			FDelveDeepCharacterDeathEventPayload Payload;
			Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Character.Death"));
			Payload.Character = this;
			Payload.Killer = Killer;
			Payload.DeathLocation = GetActorLocation();
			Payload.Instigator = Killer;

			// Validate payload before broadcasting
			FValidationContext Context;
			Context.SystemName = TEXT("Character");
			Context.OperationName = TEXT("BroadcastDeathEvent");

			if (!Payload.Validate(Context))
			{
				UE_LOG(LogDelveDeepCharacter, Warning,
					TEXT("Death event payload validation failed: %s"),
					*Context.GetReport());
				// Continue broadcasting despite validation warnings
			}

			EventSubsystem->BroadcastEvent(Payload);
		}
	}
}
