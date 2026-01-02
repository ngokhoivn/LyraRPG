// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
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
 * FRPGAttributeChangedMessage
 *
 *	Message sent when an attribute has changed.
 */
USTRUCT(BlueprintType)
struct FRPGAttributeChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayTag AttributeTag;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float NewValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float MaxValue = 0.0f;
};

/**
 * URPGAttributeSet
 *
 *	Base attribute set class for the RPG plugin.
 */
UCLASS(BlueprintType)
class RPGRUNTIME_API URPGAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	URPGAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	URPGAbilitySystemComponent* GetRPGAbilitySystemComponent() const;

public:
	// Attribute: Health
	UPROPERTY(BlueprintReadOnly, Category = "RPG|Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, Health);

	// Attribute: MaxHealth
	UPROPERTY(BlueprintReadOnly, Category = "RPG|Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, MaxHealth);

	// Attribute: Mana
	UPROPERTY(BlueprintReadOnly, Category = "RPG|Attributes", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, Mana);

	// Attribute: MaxMana
	UPROPERTY(BlueprintReadOnly, Category = "RPG|Attributes", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, MaxMana);

	// Attribute: Stamina
	UPROPERTY(BlueprintReadOnly, Category = "RPG|Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, Stamina);

	// Attribute: MaxStamina
	UPROPERTY(BlueprintReadOnly, Category = "RPG|Attributes", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, MaxStamina);

	// -------------------------------------------------------------------
	//	Meta Attributes (can only be used by executions and during post-replication)
	// -------------------------------------------------------------------

	// The current healing attribute. Healing is an "intermediate" property used by a gameplay effect to increase the health attribute.
	UPROPERTY(BlueprintReadOnly, Category = "RPG|Attributes", Meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, Healing);

	// The current damage attribute. Damage is an "intermediate" property used by a gameplay effect to decrease the health attribute.
	UPROPERTY(BlueprintReadOnly, Category = "RPG|Attributes", Meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(URPGAttributeSet, Damage);

public:
	// Delegate to broadcast when the health attribute changes.
	mutable FRPGAttributeEvent OnHealthChanged;

	// Delegate to broadcast when the max health attribute changes.
	mutable FRPGAttributeEvent OnMaxHealthChanged;

	// Delegate to broadcast when the stamina attribute changes.
	mutable FRPGAttributeEvent OnStaminaChanged;

	// Delegate to broadcast when the max stamina attribute changes.
	mutable FRPGAttributeEvent OnMaxStaminaChanged;

	// Delegate to broadcast when the health attribute reaches zero.
	mutable FRPGAttributeEvent OnOutOfHealth;

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

private:
	// Used to track when the health attribute reach zero.
	bool bOutOfHealth;

	// Helper to store values before they are changed
	float HealthBeforeAttributeChange;
	float MaxHealthBeforeAttributeChange;
	float StaminaBeforeAttributeChange;
	float MaxStaminaBeforeAttributeChange;
};
