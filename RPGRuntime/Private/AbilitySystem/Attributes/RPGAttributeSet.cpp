// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Attributes/RPGAttributeSet.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGAttributeSet)

URPGAttributeSet::URPGAttributeSet()
{
}

URPGAbilitySystemComponent* URPGAttributeSet::GetRPGAbilitySystemComponent() const
{
	return Cast<URPGAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
