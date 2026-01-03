// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/RPGGameplayAbility.h"
#include "RPGGA_ModifyStamina.generated.h"

/**
 * URPGGA_ModifyStamina
 *
 * A test ability used to modify the stamina of the owner by applying changes to the Stamina attribute.
 */
UCLASS()
class RPGRUNTIME_API URPGGA_ModifyStamina : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGGA_ModifyStamina(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// Amount to change the stamina by
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	float Value;

	// The Gameplay Effect class to apply
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	TSubclassOf<UGameplayEffect> EffectToApply;

	// The tag used to pass the value into the GE via SetByCaller
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	FGameplayTag SetByCallerTag;
};
