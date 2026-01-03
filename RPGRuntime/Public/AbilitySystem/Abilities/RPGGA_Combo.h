// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/RPGGameplayAbility.h"
#include "RPGGA_Combo.generated.h"

/**
 * URPGGA_Combo
 *
 * Base class for combo abilities that are driven by animation montages and gameplay events.
 */
UCLASS(Abstract)
class RPGRUNTIME_API URPGGA_Combo : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGGA_Combo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	// Called when input is pressed during the ability
	virtual void OnAbilityInputPressed();

	// Handles gameplay events sent from the montage
	UFUNCTION()
	void HandleComboEvent(FGameplayEventData Payload);

	// Jumps to the specified montage section
	void AdvanceToNextSection(FName SectionName);

protected:
	// Montage containing the combo sections (Attack1, Attack2, etc.)
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TObjectPtr<UAnimMontage> ComboMontage;

	// Maps combo state/section to a Gameplay Effect for damage
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TMap<FGameplayTag, TSubclassOf<UGameplayEffect>> DamageEffectMap;

private:
	// Tracks if the player has pressed the button during the action window
	bool bInputBuffered = false;

	// Current section of the combo
	FName CurrentSectionName;

	// Handle for the input event listener
	FDelegateHandle InputPressedHandle;
};
