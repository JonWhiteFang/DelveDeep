// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepCharacter.h"
#include "Character/DelveDeepCharacterComponent.h"
#include "Character/DelveDeepStatsComponent.h"
#include "Character/DelveDeepAbilitiesComponent.h"
#include "Character/DelveDeepEquipmentComponent.h"
#include "DelveDeepConfigurationManager.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepValidation.h"
#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventPayload.h"
#include "DelveDeepTelemetrySubsystem.h"
#include "GameplayTagsManager.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepCharacter, Log, All);

// Performance profiling stats
DECLARE_STATS_GROUP(TEXT("DelveDeep"), STATGROUP_DelveDeep, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Character TakeDamage"), STAT_CharacterTakeDamage, STATGROUP_DelveDeep);
DECLARE_CYCLE_STAT(TEXT("Character Heal"), STAT_CharacterHeal, STATGROUP_DelveDeep);
DECLARE_CYCLE_STAT(TEXT("Character Die"), STAT_CharacterDie, STATGROUP_DelveDeep);
DECLARE_CYCLE_STAT(TEXT("Character InitializeFromData"), STAT_CharacterInitialize, STATGROUP_DelveDeep);

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

	// Register with telemetry subsystem
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDelveDeepTelemetrySubsystem* Telemetry = GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>())
		{
			// Track character entity count
			int32 CurrentCount = Telemetry->GetEntityCount(FName("Characters"));
			Telemetry->TrackEntityCount(FName("Characters"), CurrentCount + 1);
		}
	}

	// Initialize character from configuration data
	InitializeFromData();
}

void ADelveDeepCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from telemetry subsystem
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDelveDeepTelemetrySubsystem* Telemetry = GameInstance->GetSubsystem<UDelveDeepTelemetrySubsystem>())
		{
			// Decrement character entity count
			int32 CurrentCount = Telemetry->GetEntityCount(FName("Characters"));
			Telemetry->TrackEntityCount(FName("Characters"), FMath::Max(0, CurrentCount - 1));
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ADelveDeepCharacter::InitializeFromData()
{
	SCOPE_CYCLE_COUNTER(STAT_CharacterInitialize);
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
	FDelveDeepValidationContext Context;
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

bool ADelveDeepCharacter::ValidateCharacterData(FDelveDeepValidationContext& Context) const
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

float ADelveDeepCharacter::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	SCOPE_CYCLE_COUNTER(STAT_CharacterTakeDamage);
	
	// Call parent implementation
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	
	// Validate damage amount
	if (ActualDamage <= 0.0f)
	{
		return 0.0f;
	}

	// Cannot damage dead characters
	if (bIsDead)
	{
		return 0.0f;
	}

	// Validate stats component
	if (!StatsComponent)
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("Cannot apply damage without stats component on %s"), *GetName());
		return 0.0f;
	}

	// Apply damage to health
	StatsComponent->ModifyHealth(-ActualDamage);

	// Broadcast damage event
	BroadcastDamageEvent(ActualDamage, DamageCauser);

	// Call Blueprint event
	OnDamaged(ActualDamage, DamageCauser);

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
		*GetName(), ActualDamage, DamageCauser ? *DamageCauser->GetName() : TEXT("Unknown"));
	
	return ActualDamage;
}

void ADelveDeepCharacter::ApplySimpleDamage(float DamageAmount, AActor* DamageSource)
{
	// Create a simple damage event and call the full TakeDamage function
	FDamageEvent DamageEvent;
	TakeDamage(DamageAmount, DamageEvent, nullptr, DamageSource);
}

void ADelveDeepCharacter::Heal(float HealAmount)
{
	SCOPE_CYCLE_COUNTER(STAT_CharacterHeal);
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
	SCOPE_CYCLE_COUNTER(STAT_CharacterDie);
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

	// Play death animation
	PlayDeathAnimation();

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

	// Reset to idle animation
	PlayIdleAnimation();

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
			FDelveDeepValidationContext Context;
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

void ADelveDeepCharacter::UpdateSpriteFacingDirection()
{
	// Get sprite component
	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (!SpriteComponent)
	{
		return;
	}

	// Get current velocity
	FVector Velocity = GetVelocity();
	
	// Only update facing if moving horizontally
	if (FMath::Abs(Velocity.X) > 1.0f)
	{
		// Flip sprite based on horizontal velocity
		// Positive X = facing right (no flip)
		// Negative X = facing left (flip)
		bool bShouldFlip = Velocity.X < 0.0f;
		
		// Update sprite rotation to flip horizontally
		if (bShouldFlip)
		{
			SpriteComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
		}
		else
		{
			SpriteComponent->SetRelativeRotation(FRotator::ZeroRotator);
		}
	}
}

void ADelveDeepCharacter::PlayIdleAnimation()
{
	if (!CharacterData)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Cannot play idle animation without character data"));
		return;
	}

	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (!SpriteComponent)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Cannot play idle animation without sprite component"));
		return;
	}

	// Load idle animation from character data
	if (!CharacterData->IdleAnimation.IsNull())
	{
		if (UPaperFlipbook* IdleFlipbook = CharacterData->IdleAnimation.LoadSynchronous())
		{
			SpriteComponent->SetFlipbook(IdleFlipbook);
			UE_LOG(LogDelveDeepCharacter, Verbose, TEXT("Playing idle animation for %s"), *GetName());
		}
		else
		{
			UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Failed to load idle animation for %s"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogDelveDeepCharacter, Verbose, TEXT("No idle animation assigned for %s"), *GetName());
	}
}

void ADelveDeepCharacter::PlayWalkAnimation()
{
	if (!CharacterData)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Cannot play walk animation without character data"));
		return;
	}

	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (!SpriteComponent)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Cannot play walk animation without sprite component"));
		return;
	}

	// Load walk animation from character data
	if (!CharacterData->WalkAnimation.IsNull())
	{
		if (UPaperFlipbook* WalkFlipbook = CharacterData->WalkAnimation.LoadSynchronous())
		{
			SpriteComponent->SetFlipbook(WalkFlipbook);
			UE_LOG(LogDelveDeepCharacter, Verbose, TEXT("Playing walk animation for %s"), *GetName());
		}
		else
		{
			UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Failed to load walk animation for %s"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogDelveDeepCharacter, Verbose, TEXT("No walk animation assigned for %s"), *GetName());
	}
}

void ADelveDeepCharacter::PlayAttackAnimation()
{
	if (!CharacterData)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Cannot play attack animation without character data"));
		return;
	}

	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (!SpriteComponent)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Cannot play attack animation without sprite component"));
		return;
	}

	// Load attack animation from character data
	if (!CharacterData->AttackAnimation.IsNull())
	{
		if (UPaperFlipbook* AttackFlipbook = CharacterData->AttackAnimation.LoadSynchronous())
		{
			SpriteComponent->SetFlipbook(AttackFlipbook);
			UE_LOG(LogDelveDeepCharacter, Verbose, TEXT("Playing attack animation for %s"), *GetName());
		}
		else
		{
			UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Failed to load attack animation for %s"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogDelveDeepCharacter, Verbose, TEXT("No attack animation assigned for %s"), *GetName());
	}
}

void ADelveDeepCharacter::PlayDeathAnimation()
{
	if (!CharacterData)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Cannot play death animation without character data"));
		return;
	}

	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (!SpriteComponent)
	{
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Cannot play death animation without sprite component"));
		return;
	}

	// Load death animation from character data
	if (!CharacterData->DeathAnimation.IsNull())
	{
		if (UPaperFlipbook* DeathFlipbook = CharacterData->DeathAnimation.LoadSynchronous())
		{
			SpriteComponent->SetFlipbook(DeathFlipbook);
			UE_LOG(LogDelveDeepCharacter, Verbose, TEXT("Playing death animation for %s"), *GetName());
		}
		else
		{
			UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Failed to load death animation for %s"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogDelveDeepCharacter, Verbose, TEXT("No death animation assigned for %s"), *GetName());
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
			FDelveDeepValidationContext Context;
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
			FDelveDeepValidationContext Context;
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
