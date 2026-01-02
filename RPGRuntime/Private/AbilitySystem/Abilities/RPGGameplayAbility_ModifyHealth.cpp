// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/RPGGameplayAbility_ModifyHealth.h"
#include "AbilitySystem/Attributes/RPGAttributeSet.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGameplayAbility_ModifyHealth)

URPGGameplayAbility_ModifyHealth::URPGGameplayAbility_ModifyHealth(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Value = 10.0f;
	bIsHealing = false;
}

void URPGGameplayAbility_ModifyHealth::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		FGameplayAttribute AttributeToModify = bIsHealing ? URPGAttributeSet::GetHealingAttribute() : URPGAttributeSet::GetDamageAttribute();
		
		// Apply a simple modification to the attribute. 
		// Since Damage and Healing are meta-attributes, they will be processed in PostGameplayEffectExecute.
		ASC->ApplyModToAttribute(AttributeToModify, EGameplayModOp::Additive, Value);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
