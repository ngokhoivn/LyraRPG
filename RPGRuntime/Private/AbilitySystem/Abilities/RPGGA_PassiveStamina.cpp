// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/RPGGA_PassiveStamina.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGA_PassiveStamina)

URPGGA_PassiveStamina::URPGGA_PassiveStamina(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set default activation policy to OnSpawn for passive abilities
	ActivationPolicy = ERPGAbilityActivationPolicy::OnSpawn;
	
	// Passive abilities usually shouldn't be canceled by other abilities
	ActivationGroup = ERPGAbilityActivationGroup::Independent;
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void URPGGA_PassiveStamina::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (IsInstantiated())
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (ASC && IsLocallyControlled())
		{
			for (const TSubclassOf<UGameplayEffect>& EffectClass : PassiveEffects)
			{
				if (EffectClass)
				{
					FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectClass);
					if (SpecHandle.IsValid())
					{
						FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
						AppliedEffectHandles.Add(ActiveHandle);
					}
				}
			}
		}
	}

	// We don't call EndAbility here because it's a passive ability that should stay active
	// Super::ActivateAbility is not called as we want custom control
}

void URPGGA_PassiveStamina::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsInstantiated())
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (ASC)
		{
			for (FActiveGameplayEffectHandle ActiveHandle : AppliedEffectHandles)
			{
				if (ActiveHandle.IsValid())
				{
					ASC->RemoveActiveGameplayEffect(ActiveHandle);
				}
			}
			AppliedEffectHandles.Empty();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
