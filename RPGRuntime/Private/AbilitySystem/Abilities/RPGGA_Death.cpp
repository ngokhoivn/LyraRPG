// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/RPGGA_Death.h"
#include "Character/RPGHealthComponent.h"
#include "Character/RPGHero_Character.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "System/RPGGameplayTags.h"
#include "System/RPGLogChannels.h"
#include "Animation/AnimMontage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGA_Death)

URPGGA_Death::URPGGA_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	bAutoStartDeath = true;
	bAutoFinishDeath = true;

	ActivationPolicy = ERPGAbilityActivationPolicy::OnInputTriggered; // Kích hoạt bằng event cũng dùng policy này trong RPG template
	ActivationGroup = ERPGAbilityActivationGroup::Exclusive_Blocking;

	AbilityTags.AddTag(RPGGameplayTags::Ability_ActivateFail_IsDead);

	// Tự động nối với sự kiện Death từ HealthComponent
	FAbilityTriggerData DeathTrigger;
	DeathTrigger.TriggerTag = RPGGameplayTags::GameplayEvent_Death;
	DeathTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(DeathTrigger);
}

void URPGGA_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);

	URPGAbilitySystemComponent* RPGASC = CastChecked<URPGAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	// If we are already dead, don't do anything.
	if (RPGASC->HasMatchingGameplayTag(FRPGGameplayTags::Get().Status_Death_Dead))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (bAutoStartDeath)
	{
		StartDeath();
	}

	// Determine which montages to use (Ability level or Character level)
	TArray<TObjectPtr<UAnimMontage>> SelectedMontages = DeathMontages;
	
	if (SelectedMontages.Num() == 0)
	{
		if (ARPGHero_Character* HeroChar = Cast<ARPGHero_Character>(GetAvatarActorFromActorInfo()))
		{
			SelectedMontages = HeroChar->GetDeathMontages();
		}
	}

	// Play a death montage if available and wait for it
	if (SelectedMontages.Num() > 0)
	{
		const int32 RandomIndex = FMath::RandRange(0, SelectedMontages.Num() - 1);
		if (UAnimMontage* MontageToPlay = SelectedMontages[RandomIndex])
		{
			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay, 1.0f, NAME_None, bAutoFinishDeath, 1.0f);
			if (MontageTask)
			{
				MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnDeathMontageCompleted);
				MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnDeathMontageCompleted);
				MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnDeathMontageCompleted);
				MontageTask->ReadyForActivation();
			}
			else
			{
				UE_LOG(LogRPG, Warning, TEXT("URPGGA_Death::ActivateAbility failed to create montage task for [%s]"), *GetNameSafe(MontageToPlay));
				EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			}
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
	}
	else
	{
		// No montage? End immediately to trigger FinishDeath
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void URPGGA_Death::OnDeathMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void URPGGA_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bAutoFinishDeath)
	{
		FinishDeath();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URPGGA_Death::StartDeath()
{
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		if (URPGHealthComponent* HealthComponent = URPGHealthComponent::FindHealthComponent(AvatarActor))
		{
			HealthComponent->StartDeath();
		}
	}
}

void URPGGA_Death::FinishDeath()
{
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		if (URPGHealthComponent* HealthComponent = URPGHealthComponent::FindHealthComponent(AvatarActor))
		{
			HealthComponent->FinishDeath();
		}
	}
}
