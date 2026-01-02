// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RPGHealthComponent.h"
#include "AbilitySystem/Attributes/RPGAttributeSet.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "System/RPGGameplayTags.h"
#include "System/RPGLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGHealthComponent)

URPGHealthComponent::URPGHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	AttributeSet = nullptr;
	DeathState = ERPGDeathState::NotDead;
}

void URPGHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URPGHealthComponent, DeathState);
}

void URPGHealthComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

void URPGHealthComponent::InitializeWithAbilitySystem(URPGAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogRPG, Error, TEXT("RPGHealthComponent: Health component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogRPG, Error, TEXT("RPGHealthComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	AttributeSet = AbilitySystemComponent->GetSet<URPGAttributeSet>();
	if (!AttributeSet)
	{
		UE_LOG(LogRPG, Error, TEXT("RPGHealthComponent: Cannot initialize health component for owner [%s] with NULL attribute set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// Register to listen for attribute changes.
	AttributeSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	AttributeSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	AttributeSet->OnStaminaChanged.AddUObject(this, &ThisClass::HandleStaminaChanged);
	AttributeSet->OnMaxStaminaChanged.AddUObject(this, &ThisClass::HandleMaxStaminaChanged);
	AttributeSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

	// Clear tags
	ClearGameplayTags();

	OnHealthChangedDelegate.Broadcast(this, AttributeSet->GetHealth(), AttributeSet->GetHealth(), nullptr);
	OnMaxHealthChangedDelegate.Broadcast(this, AttributeSet->GetMaxHealth(), AttributeSet->GetMaxHealth(), nullptr);
	OnStaminaChangedDelegate.Broadcast(this, AttributeSet->GetStamina(), AttributeSet->GetStamina(), nullptr);
	OnMaxStaminaChangedDelegate.Broadcast(this, AttributeSet->GetMaxStamina(), AttributeSet->GetMaxStamina(), nullptr);
}

void URPGHealthComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (AttributeSet)
	{
		AttributeSet->OnHealthChanged.RemoveAll(this);
		AttributeSet->OnMaxHealthChanged.RemoveAll(this);
		AttributeSet->OnStaminaChanged.RemoveAll(this);
		AttributeSet->OnMaxStaminaChanged.RemoveAll(this);
		AttributeSet->OnOutOfHealth.RemoveAll(this);
	}

	AttributeSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void URPGHealthComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
		AbilitySystemComponent->SetLooseGameplayTagCount(GameplayTags.Status_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(GameplayTags.Status_Death_Dead, 0);
	}
}

float URPGHealthComponent::GetHealth() const
{
	return (AttributeSet ? AttributeSet->GetHealth() : 0.0f);
}

float URPGHealthComponent::GetMaxHealth() const
{
	return (AttributeSet ? AttributeSet->GetMaxHealth() : 0.0f);
}

float URPGHealthComponent::GetHealthNormalized() const
{
	if (AttributeSet)
	{
		const float Health = AttributeSet->GetHealth();
		const float MaxHealth = AttributeSet->GetMaxHealth();

		return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
	}

	return 0.0f;
}

float URPGHealthComponent::GetStamina() const
{
	return (AttributeSet ? AttributeSet->GetStamina() : 0.0f);
}

float URPGHealthComponent::GetMaxStamina() const
{
	return (AttributeSet ? AttributeSet->GetMaxStamina() : 0.0f);
}

float URPGHealthComponent::GetStaminaNormalized() const
{
	if (AttributeSet)
	{
		const float Stamina = AttributeSet->GetStamina();
		const float MaxStamina = AttributeSet->GetMaxStamina();

		return ((MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f);
	}

	return 0.0f;
}

void URPGHealthComponent::HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnHealthChangedDelegate.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void URPGHealthComponent::HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxHealthChangedDelegate.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void URPGHealthComponent::HandleStaminaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnStaminaChangedDelegate.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void URPGHealthComponent::HandleMaxStaminaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxStaminaChangedDelegate.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void URPGHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();

		// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.
		{
			FGameplayEventData Payload;
			Payload.EventTag = GameplayTags.GameplayEvent_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec->Def;
			Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;

			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}
	}
#endif // #if WITH_SERVER_CODE
}

void URPGHealthComponent::OnRep_DeathState(ERPGDeathState OldDeathState)
{
	const ERPGDeathState NewDeathState = DeathState;

	DeathState = OldDeathState;

	if (OldDeathState > NewDeathState)
	{
		return;
	}

	if (OldDeathState == ERPGDeathState::NotDead)
	{
		if (NewDeathState == ERPGDeathState::DeathStarted)
		{
			StartDeath();
		}
		else if (NewDeathState == ERPGDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
	}
	else if (OldDeathState == ERPGDeathState::DeathStarted)
	{
		if (NewDeathState == ERPGDeathState::DeathFinished)
		{
			FinishDeath();
		}
	}
}

void URPGHealthComponent::StartDeath()
{
	if (DeathState != ERPGDeathState::NotDead)
	{
		return;
	}

	DeathState = ERPGDeathState::DeathStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(FRPGGameplayTags::Get().Status_Death_Dying, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void URPGHealthComponent::FinishDeath()
{
	if (DeathState != ERPGDeathState::DeathStarted)
	{
		return;
	}

	DeathState = ERPGDeathState::DeathFinished;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(FRPGGameplayTags::Get().Status_Death_Dead, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void URPGHealthComponent::DamageSelfDestruct(bool bFellOutOfWorld)
{
	if ((DeathState == ERPGDeathState::NotDead) && AbilitySystemComponent)
	{
		StartDeath();
		FinishDeath();
	}
}
