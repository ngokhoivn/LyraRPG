// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/RPGGameplayAbility.h"
#include "RPGGA_Death.generated.h"

/**
 * URPGGA_Death
 *
 *	Gameplay ability used for handling character death.
 *  Standalone version of LyraGameplayAbility_Death.
 */
UCLASS(Abstract)
class RPGRUNTIME_API URPGGA_Death : public URPGGameplayAbility
{
	GENERATED_BODY()

public:

	URPGGA_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Starts the death sequence on the health component.
	UFUNCTION(BlueprintCallable, Category = "RPG|Ability")
	void StartDeath();

	// Finishes the death sequence on the health component.
	UFUNCTION(BlueprintCallable, Category = "RPG|Ability")
	void FinishDeath();

private:
	UFUNCTION()
	void OnDeathMontageCompleted();

protected:

	// If true, the ability will automatically call FinishDeath when it ends.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	bool bAutoStartDeath;

	// If true, the ability will automatically call FinishDeath when it ends.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	bool bAutoFinishDeath;

	/** Random death montages to pick from if the character doesn't have any or if we want to override */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	TArray<TObjectPtr<UAnimMontage>> DeathMontages;
};
