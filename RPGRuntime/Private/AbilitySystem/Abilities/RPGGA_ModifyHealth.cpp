// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/RPGGA_ModifyHealth.h"
#include "AbilitySystem/Attributes/RPGAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "System/RPGLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGA_ModifyHealth)

URPGGA_ModifyHealth::URPGGA_ModifyHealth(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Value = 10.0f;
	bIsHealing = false;
}

void URPGGA_ModifyHealth::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
			
			UE_LOG(LogRPG, Log, TEXT("RPGGA_ModifyHealth: Applied GE [%s] with SetByCaller [%s] = %f"), 
				*GetNameSafe(EffectToApply), *SetByCallerTag.ToString(), Value);
			UE_LOG(LogRPG, Log, TEXT("RPGGA_ModifyHealth: Internal Health is now %f"), ASC->GetNumericAttribute(URPGAttributeSet::GetHealthAttribute()));
		}
	}
	else
	{
		UE_LOG(LogRPG, Warning, TEXT("RPGGA_ModifyHealth: Failed to apply GE. ASC or EffectToApply is missing."));
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
