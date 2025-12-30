// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Attributes/RPGHealthSet.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGHealthSet)

URPGHealthSet::URPGHealthSet()
	: Health(100.0f)
	, MaxHealth(100.0f)
{
	bOutOfHealth = false;
}

void URPGHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URPGHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void URPGHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGHealthSet, Health, OldValue);

	// Call the health changed delegate.
	const float CurrentHealth = GetHealth();
	const float OldHealth = OldValue.GetCurrentValue();

	OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, 0.0f, OldHealth, CurrentHealth);
}

void URPGHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGHealthSet, MaxHealth, OldValue);

	// Call the max health changed delegate.
	const float CurrentMaxHealth = GetMaxHealth();
	const float OldMaxHealth = OldValue.GetCurrentValue();

	OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, 0.0f, OldMaxHealth, CurrentMaxHealth);
}

void URPGHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void URPGHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void URPGHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		// Make sure current health is not greater than the new max health.
		if (GetHealth() > NewValue)
		{
			URPGAbilitySystemComponent* RPGASC = GetRPGAbilitySystemComponent();
			check(RPGASC);

			RPGASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfHealth && (GetHealth() > 0.0f))
	{
		bOutOfHealth = false;
	}
}

void URPGHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	float DeltaValue = 0.0f;
	if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Additive)
	{
		DeltaValue = Data.EvaluatedData.Magnitude;
	}

	AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();
	AActor* Causer = Data.EffectSpec.GetContext().GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Convert the damage into a negative modification on health.
		const float LocalDamageDone = GetDamage();
		SetDamage(0.0f);

		if (LocalDamageDone > 0.0f)
		{
			// Apply the health change and then clamp it.
			const float NewHealth = GetHealth() - LocalDamageDone;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

			OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, GetHealth() + LocalDamageDone, GetHealth());
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		// Convert the healing into a positive modification on health.
		const float LocalHealingDone = GetHealing();
		SetHealing(0.0f);

		if (LocalHealingDone > 0.0f)
		{
			// Apply the health change and then clamp it.
			const float NewHealth = GetHealth() + LocalHealingDone;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

			OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, GetHealth() - LocalHealingDone, GetHealth());
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Handle other health changes.
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, GetHealth() - DeltaValue, GetHealth());
	}

	// If health has reached zero, broadcast the out of health delegate.
	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, GetHealth() + DeltaValue, GetHealth());
	}

	// Track health state
	bOutOfHealth = (GetHealth() <= 0.0f);
}

void URPGHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}
