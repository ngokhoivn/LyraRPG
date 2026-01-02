// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/RPGGameplayAbility.h"
#include "RPGGA_ModifyHealth.generated.h"

/**
 * URPGGA_ModifyHealth
 *
 * A test ability used to modify the health of the owner by applying changes to Damage or Healing attributes.
 */
UCLASS()
class RPGRUNTIME_API URPGGA_ModifyHealth : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGGA_ModifyHealth(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// Amount to change the health by
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	float Value;

	// If true, will modify the Healing attribute (increase health). If false, will modify the Damage attribute (decrease health).
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bIsHealing;

	// The Gameplay Effect class to apply
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	TSubclassOf<UGameplayEffect> EffectToApply;

	// The tag used to pass the value into the GE via SetByCaller
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	FGameplayTag SetByCallerTag;
};
