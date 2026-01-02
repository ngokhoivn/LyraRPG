// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Attributes/RPGAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "System/RPGGameplayTags.h"
#include "System/RPGAssetManager.h"
#include "System/RPGGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGAttributeSet)

URPGAttributeSet::URPGAttributeSet()
{
	bOutOfHealth = false;
	HealthBeforeAttributeChange = 0.0f;
	MaxHealthBeforeAttributeChange = 0.0f;
	StaminaBeforeAttributeChange = 0.0f;
	MaxStaminaBeforeAttributeChange = 0.0f;

	// Skip data loading for CDO or if engine is not ready (prevents crash during startup)
	if (HasAnyFlags(RF_ClassDefaultObject) || !GEngine || !GEngine->AssetManager)
	{
		Health.SetBaseValue(100.0f);
		Health.SetCurrentValue(100.0f);
		MaxHealth.SetBaseValue(100.0f);
		MaxHealth.SetCurrentValue(100.0f);
		Mana.SetBaseValue(50.0f);
		Mana.SetCurrentValue(50.0f);
		MaxMana.SetBaseValue(50.0f);
		MaxMana.SetCurrentValue(50.0f);
		Stamina.SetBaseValue(100.0f);
		Stamina.SetCurrentValue(100.0f);
		MaxStamina.SetBaseValue(100.0f);
		MaxStamina.SetCurrentValue(100.0f);
		return;
	}

	const URPGGameData& GameData = URPGAssetManager::Get().GetGameData();

	Health.SetBaseValue(GameData.DefaultHealth);
	Health.SetCurrentValue(GameData.DefaultHealth);

	MaxHealth.SetBaseValue(GameData.DefaultMaxHealth);
	MaxHealth.SetCurrentValue(GameData.DefaultMaxHealth);

	Mana.SetBaseValue(GameData.DefaultMana);
	Mana.SetCurrentValue(GameData.DefaultMana);

	MaxMana.SetBaseValue(GameData.DefaultMaxMana);
	MaxMana.SetCurrentValue(GameData.DefaultMaxMana);

	Stamina.SetBaseValue(GameData.DefaultStamina);
	Stamina.SetCurrentValue(GameData.DefaultStamina);

	MaxStamina.SetBaseValue(GameData.DefaultMaxStamina);
	MaxStamina.SetCurrentValue(GameData.DefaultMaxStamina);
}

void URPGAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
}

void URPGAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		AdjustAttributeForMaxChange(Mana, MaxMana, NewValue, GetManaAttribute());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		AdjustAttributeForMaxChange(Stamina, MaxStamina, NewValue, GetStaminaAttribute());
	}

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
}

void URPGAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
}

bool URPGAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	HealthBeforeAttributeChange = GetHealth();
	MaxHealthBeforeAttributeChange = GetMaxHealth();
	StaminaBeforeAttributeChange = GetStamina();
	MaxStaminaBeforeAttributeChange = GetMaxStamina();

	return true;
}

void URPGAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamageDone = GetDamage();
		SetDamage(0.0f);

		if (LocalDamageDone > 0.0f)
		{
			const float NewHealth = GetHealth() - LocalDamageDone;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		const float LocalHealingDone = GetHealing();
		SetHealing(0.0f);

		if (LocalHealingDone > 0.0f)
		{
			const float NewHealth = GetHealth() + LocalHealingDone;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
		}
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		FRPGAttributeChangedMessage Message;
		Message.Owner = Data.Target.GetAvatarActor();
		Message.AttributeTag = FRPGGameplayTags::Get().Stat_Health;
		Message.NewValue = GetHealth();
		Message.MaxValue = GetMaxHealth();

		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.BroadcastMessage(FRPGGameplayTags::Get().Message_Attribute_HealthChanged, Message);

		if (GetHealth() != HealthBeforeAttributeChange)
		{
			OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
		}

		if ((GetHealth() <= 0.0f) && !bOutOfHealth)
		{
			OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());

			// Send the death event
			FGameplayEventData Payload;
			Payload.EventTag = RPGGameplayTags::GameplayEvent_Death;
			Payload.Instigator = Instigator;
			Payload.Target = Data.Target.GetAvatarActor();
			Payload.OptionalObject = Data.EffectSpec.Def;
			Payload.ContextHandle = Data.EffectSpec.GetEffectContext();
			Payload.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = Data.EvaluatedData.Magnitude;

			Data.Target.HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		bOutOfHealth = (GetHealth() <= 0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxHealthBeforeAttributeChange, GetMaxHealth());
	}
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));

		FRPGAttributeChangedMessage Message;
		Message.Owner = Data.Target.GetAvatarActor();
		Message.AttributeTag = FRPGGameplayTags::Get().Stat_Mana;
		Message.NewValue = GetMana();
		Message.MaxValue = GetMaxMana();

		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.BroadcastMessage(FRPGGameplayTags::Get().Message_Attribute_ManaChanged, Message);
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));

		FRPGAttributeChangedMessage Message;
		Message.Owner = Data.Target.GetAvatarActor();
		Message.AttributeTag = FRPGGameplayTags::Get().Stat_Stamina;
		Message.NewValue = GetStamina();
		Message.MaxValue = GetMaxStamina();

		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.BroadcastMessage(FRPGGameplayTags::Get().Message_Attribute_StaminaChanged, Message);

		if (GetStamina() != StaminaBeforeAttributeChange)
		{
			OnStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, StaminaBeforeAttributeChange, GetStamina());
		}
	}
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		OnMaxStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxStaminaBeforeAttributeChange, GetMaxStamina());
	}
}

void URPGAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void URPGAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Health, OldValue);
}

void URPGAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxHealth, OldValue);
}

void URPGAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Mana, OldValue);
}

void URPGAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxMana, OldValue);
}

void URPGAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Stamina, OldValue);
}

void URPGAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxStamina, OldValue);
}

URPGAbilitySystemComponent* URPGAttributeSet::GetRPGAbilitySystemComponent() const
{
	return Cast<URPGAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
