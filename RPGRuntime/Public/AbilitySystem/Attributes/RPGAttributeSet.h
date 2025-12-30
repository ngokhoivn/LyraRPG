// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AttributeSet.h"
#include "RPGAttributeSet.generated.h"

class URPGAbilitySystemComponent;
class AActor;
class UObject;
struct FFrame;
struct FGameplayEffectSpec;

/**
 * Macro used to generate setter and getter functions for attributes.
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** 
 * Delegate used to broadcast attribute changes.
 */
DECLARE_MULTICAST_DELEGATE_SixParams(FRPGAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/);

/**
 * URPGAttributeSet
 *
 *	Base attribute set class for the RPG plugin.
 */
UCLASS()
class RPGRUNTIME_API URPGAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	URPGAttributeSet();

	URPGAbilitySystemComponent* GetRPGAbilitySystemComponent() const;
};
