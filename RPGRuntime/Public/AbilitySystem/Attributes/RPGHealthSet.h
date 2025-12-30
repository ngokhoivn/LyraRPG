// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RPGAttributeSet.h"
#include "RPGHealthSet.generated.h"

class UObject;
struct FFrame;

/**
 * URPGHealthSet
 *
 *	Class used by the Ability System to define health-related attributes.
 */
UCLASS(BlueprintType)
class RPGRUNTIME_API URPGHealthSet : public URPGAttributeSet
{
	GENERATED_BODY()

public:

	URPGHealthSet();

	ATTRIBUTE_ACCESSORS(URPGHealthSet, Health);
	ATTRIBUTE_ACCESSORS(URPGHealthSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(URPGHealthSet, Healing);
	ATTRIBUTE_ACCESSORS(URPGHealthSet, Damage);

	// Delegate to broadcast when the health attribute changes.
	mutable FRPGAttributeEvent OnHealthChanged;

	// Delegate to broadcast when the max health attribute changes.
	mutable FRPGAttributeEvent OnMaxHealthChanged;

	// Delegate to broadcast when the health attribute reach zero.
	mutable FRPGAttributeEvent OnOutOfHealth;

protected:

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:

	// The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "RPG|Health", Meta = (HideFromModifiers, AllowPrivateAccess = "true"))
	FGameplayAttributeData Health;

	// The current maximum health attribute.  Max health is capped of at 1,000,000.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "RPG|Health", Meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHealth;

	// Used to track when the health attribute reach zero.
	bool bOutOfHealth;

	// -------------------------------------------------------------------
	//	Meta Attributes (can only be used by executions and during post-replication)
	// -------------------------------------------------------------------

	// The current healing attribute.  Healing is an "intermediate" property used by a gameplay effect to increase the health attribute.
	UPROPERTY(BlueprintReadOnly, Category = "RPG|Health", Meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Healing;

	// The current damage attribute.  Damage is an "intermediate" property used by a gameplay effect to decrease the health attribute.
	UPROPERTY(BlueprintReadOnly, Category = "RPG|Health", Meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Damage;

protected:

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
};
