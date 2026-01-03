// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/RPGGameplayAbility.h"
#include "RPGGA_PassiveStamina.generated.h"

/**
 * URPGGA_PassiveStamina
 *
 * A passive ability that automatically applies a set of Gameplay Effects 
 * (like regeneration and decay) once granted.
 */
UCLASS()
class RPGRUNTIME_API URPGGA_PassiveStamina : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGGA_PassiveStamina(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// Gameplay Effects to apply when the ability is activated
	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	TArray<TSubclassOf<UGameplayEffect>> PassiveEffects;

private:
	// Handles to the applied effects so we can remove them if needed
	TArray<FActiveGameplayEffectHandle> AppliedEffectHandles;
};
