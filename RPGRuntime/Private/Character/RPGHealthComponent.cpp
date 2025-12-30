// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RPGHealthComponent.h"
#include "AbilitySystem/Attributes/RPGHealthSet.h"
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
	HealthSet = nullptr;
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

	HealthSet = AbilitySystemComponent->GetSet<URPGHealthSet>();
	if (!HealthSet)
	{
		UE_LOG(LogRPG, Error, TEXT("RPGHealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// Register to listen for attribute changes.
	HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

	// Clear tags
	ClearGameplayTags();

	OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, HealthSet->GetMaxHealth(), HealthSet->GetMaxHealth(), nullptr);
}

void URPGHealthComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (HealthSet)
	{
		HealthSet->OnHealthChanged.RemoveAll(this);
		HealthSet->OnMaxHealthChanged.RemoveAll(this);
		HealthSet->OnOutOfHealth.RemoveAll(this);
	}

	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void URPGHealthComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		// Stub: We need to define RPG specific tags for death states
		// AbilitySystemComponent->SetLooseGameplayTagCount(RPGGameplayTags::Status_Death_Dying, 0);
		// AbilitySystemComponent->SetLooseGameplayTagCount(RPGGameplayTags::Status_Death_Dead, 0);
	}
}

float URPGHealthComponent::GetHealth() const
{
	return (HealthSet ? HealthSet->GetHealth() : 0.0f);
}

float URPGHealthComponent::GetMaxHealth() const
{
	return (HealthSet ? HealthSet->GetMaxHealth() : 0.0f);
}

float URPGHealthComponent::GetHealthNormalized() const
{
	if (HealthSet)
	{
		const float Health = HealthSet->GetHealth();
		const float MaxHealth = HealthSet->GetMaxHealth();

		return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
	}

	return 0.0f;
}

void URPGHealthComponent::HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void URPGHealthComponent::HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void URPGHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.
		{
			FGameplayEventData Payload;
			// Payload.EventTag = RPGGameplayTags::GameplayEvent_Death;
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
		// AbilitySystemComponent->SetLooseGameplayTagCount(RPGGameplayTags::Status_Death_Dying, 1);
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
		// AbilitySystemComponent->SetLooseGameplayTagCount(RPGGameplayTags::Status_Death_Dead, 1);
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
		// This should be implemented by applying a GE that deals massive damage
		// For now we just call StartDeath as a shortcut if preferred, 
		// but Lyra does it via GE for proper GAS flow.
		StartDeath();
		FinishDeath();
	}
}
