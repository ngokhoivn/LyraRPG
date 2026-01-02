// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/RPGGA_AutoRespawn.h"
#include "Character/RPGHealthComponent.h"
#include "GameMode/RPGGameMode.h"
#include "System/RPGGameplayTags.h"
#include "System/RPGLogChannels.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/RPGRespawnMessage.h"
#include "Messages/RPGVerbMessage.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Character/RPGCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystem/Attributes/RPGAttributeSet.h"
#include "UIExtensionSystem.h"
#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGA_AutoRespawn)

URPGGA_AutoRespawn::URPGGA_AutoRespawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	ActivationPolicy = ERPGAbilityActivationPolicy::OnSpawn;
	ActivationGroup = ERPGAbilityActivationGroup::Independent;
}

void URPGGA_AutoRespawn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);
	
	UE_LOG(LogRPG, Log, TEXT("URPGGA_AutoRespawn::ActivateAbility: Activity started on %s (LocallyControlled: %d)"), *GetNameSafe(ActorInfo->AvatarActor.Get()), IsLocallyControlled());

	if (IsLocallyControlled())
	{
		UE_LOG(LogRPG, Log, TEXT("URPGGA_AutoRespawn::ActivateAbility: Calling RegisterRespawnUI for locally controlled player."));
		RegisterRespawnUI();
	}

	// Initial binding
	OnPawnAvatarSet();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void URPGGA_AutoRespawn::OnPawnAvatarSet()
{
	ClearDeathListener();

	if (IsAvatarDeadOrDying())
	{
		OnDeathStarted(GetAvatarActorFromActorInfo());
	}
	else
	{
		BindDeathListener();
	}
}

void URPGGA_AutoRespawn::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
}

void URPGGA_AutoRespawn::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (IsLocallyControlled())
	{
		UnregisterRespawnUI();
	}
	ClearDeathListener();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void URPGGA_AutoRespawn::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearDeathListener();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool URPGGA_AutoRespawn::IsAvatarDeadOrDying() const
{
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		if (const URPGHealthComponent* HealthComponent = URPGHealthComponent::FindHealthComponent(AvatarActor))
		{
			return HealthComponent->IsDeadOrDying();
		}
	}
	return false;
}

void URPGGA_AutoRespawn::BindDeathListener()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;

	if (URPGHealthComponent* HealthComponent = URPGHealthComponent::FindHealthComponent(AvatarActor))
	{
		HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
		LastBoundHealthComponent = HealthComponent;
	}

	AvatarActor->OnEndPlay.AddDynamic(this, &ThisClass::OnAvatarEndPlay);
	LastBoundAvatarActor = AvatarActor;
}

void URPGGA_AutoRespawn::ClearDeathListener()
{
	if (LastBoundHealthComponent)
	{
		LastBoundHealthComponent->OnDeathStarted.RemoveAll(this);
		LastBoundHealthComponent = nullptr;
	}

	if (LastBoundAvatarActor)
	{
		LastBoundAvatarActor->OnEndPlay.RemoveAll(this);
		LastBoundAvatarActor = nullptr;
	}
}

void URPGGA_AutoRespawn::OnDeathStarted(AActor* OwningActor)
{
	ClearDeathListener();
	
	// Explicitly store the pawn so we can destroy it later even if unpossessed
	LastBoundAvatarActor = OwningActor;

	// Broadcast respawn timer message to UI
	if (UGameplayMessageSubsystem* MessageSubsystem = UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(GetWorld()->GetGameInstance()))
	{
		FRPGRespawnTimerDurationMessage Message;
		Message.PlayerState = GetPlayerStateFromActorInfo();
		Message.RespawnDelay = RespawnDelayDuration;

		UE_LOG(LogRPG, Log, TEXT("URPGGA_AutoRespawn::OnDeathStarted: Broadcasting Respawn Duration (%.2f) for PlayerState %s"), Message.RespawnDelay, *GetNameSafe(Message.PlayerState));
		MessageSubsystem->BroadcastMessage(RPGGameplayTags::Message_Respawn_Duration, Message);
	}

	// Start timer for execution
	GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &ThisClass::ExecuteRespawn, RespawnDelayDuration, false);
}

void URPGGA_AutoRespawn::ExecuteRespawn()
{
	UE_LOG(LogRPG, Log, TEXT("URPGGA_AutoRespawn: Executing respawn."));

	// 1. Cancel death abilities so we can restart
	EndDeathAbilities();

	// 2. Reset health, mana, and stamina
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponentFromActorInfo())
	{
		RPGASC->FullResetAttributes();
	}

	// 3. Request restart from GameMode
	if (ARPGGameMode* GameMode = GetWorld()->GetAuthGameMode<ARPGGameMode>())
	{
		AController* Controller = GetPlayerControllerFromActorInfo();
		if (Controller)
		{
			GameMode->RequestPlayerRestartNextFrame(Controller, true);
		}
	}

	// 3. Destroy the old avatar actor to prevent "ghost" bodies
	if (LastBoundAvatarActor)
	{
		LastBoundAvatarActor->Destroy();
		LastBoundAvatarActor = nullptr;
	}

	// 4. Broadcast completion message
	if (UGameplayMessageSubsystem* MessageSubsystem = UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(GetWorld()->GetGameInstance()))
	{
		FRPGVerbMessage Message;
		Message.Verb = RPGGameplayTags::Message_Respawn_Completed;
		Message.Instigator = GetPlayerStateFromActorInfo();

		MessageSubsystem->BroadcastMessage(RPGGameplayTags::Message_Respawn_Completed, Message);
	}

	// Since this is OnSpawn, it will naturally persist on the PlayerState if granted there.
	// If it was on the Pawn, it would be killed, so it MUST be granted on the PlayerState.
}

void URPGGA_AutoRespawn::EndDeathAbilities()
{
	if (URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponentFromActorInfo())
	{
		FGameplayTagContainer DeathTags;
		DeathTags.AddTag(RPGGameplayTags::Ability_Type_Status_Death);

		// Cancel any currently active death abilities
		for (FGameplayAbilitySpec& Spec : RPGASC->GetActivatableAbilities())
		{
			if (Spec.IsActive() && Spec.Ability->AbilityTags.HasAny(DeathTags))
			{
				RPGASC->CancelAbilityHandle(Spec.Handle);
			}
		}
	}
}

void URPGGA_AutoRespawn::OnAvatarEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason == EEndPlayReason::Destroyed && !IsAvatarDeadOrDying())
	{
		// Force respawn if the avatar disappeared without dying (e.g. falling out of world)
		OnDeathStarted(Actor);
	}
}

void URPGGA_AutoRespawn::RegisterRespawnUI()
{
	if (!RespawnWidgetClass)
	{
		UE_LOG(LogRPG, Warning, TEXT("URPGGA_AutoRespawn::RegisterRespawnUI: RespawnWidgetClass is null!"));
		return;
	}

	if (UUIExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UUIExtensionSubsystem>())
	{
		const FGameplayTag& SlotTag = RPGGameplayTags::HUD_Slot_Reticle;
		
		UObject* ContextObject = nullptr;
		if (APlayerController* PC = GetPlayerControllerFromActorInfo())
		{
			ContextObject = PC->GetLocalPlayer();
		}

		UE_LOG(LogRPG, Log, TEXT("URPGGA_AutoRespawn::RegisterRespawnUI: Registering widget %s to slot %s with context %s"), *GetNameSafe(RespawnWidgetClass), *SlotTag.ToString(), *GetNameSafe(ContextObject));
		RespawnExtensionHandle = ExtensionSubsystem->RegisterExtensionAsWidgetForContext(SlotTag, ContextObject, RespawnWidgetClass, -1);
	}
}

void URPGGA_AutoRespawn::UnregisterRespawnUI()
{
	if (RespawnExtensionHandle.IsValid())
	{
		if (UUIExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UUIExtensionSubsystem>())
		{
			ExtensionSubsystem->UnregisterExtension(RespawnExtensionHandle);
		}
	}
}
