// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/RPGGA_ModifyStamina.h"
#include "AbilitySystem/Attributes/RPGAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "System/RPGLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGA_ModifyStamina)

URPGGA_ModifyStamina::URPGGA_ModifyStamina(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Value = 10.0f;
}

void URPGGA_ModifyStamina::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && EffectToApply.Get())
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectToApply);
		if (SpecHandle.IsValid())
		{
			// Truyền giá trị từ GA vào GE thông qua SetByCaller
			SpecHandle.Data->SetSetByCallerMagnitude(SetByCallerTag, Value);

			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	else
	{
		UE_LOG(LogRPG, Warning, TEXT("RPGGA_ModifyStamina: Failed to apply GE. ASC or EffectToApply is missing."));
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
