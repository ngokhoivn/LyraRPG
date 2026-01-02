// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "RPGAbilityTypes.h"
#include "RPGGameplayAbility.generated.h"

class URPGAbilitySystemComponent;
class URPGCameraMode;
class UAnimMontage;
class UObject;

/**
 * URPGGameplayAbility
 *
 *	The base gameplay ability class used by the RPG plugin.
 *  Standalone version of LyraGameplayAbility.
 */
UCLASS(Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class used by the RPG project."))
class RPGRUNTIME_API URPGGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	friend class URPGAbilitySystemComponent;

public:

	URPGGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "RPG|Ability")
	URPGAbilitySystemComponent* GetRPGAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "RPG|Ability")
	APlayerController* GetPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "RPG|Ability")
	APlayerState* GetPlayerStateFromActorInfo() const;

	ERPGAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	ERPGAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

	// Returns true if the requested activation group is a valid transition.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "RPG|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool CanChangeActivationGroup(ERPGAbilityActivationGroup NewGroup) const;

	// Tries to change the activation group.  Returns true if it successfully changed.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "RPG|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ChangeActivationGroup(ERPGAbilityActivationGroup NewGroup);

	// Sets the ability's camera mode.
	UFUNCTION(BlueprintCallable, Category = "RPG|Ability")
	void SetCameraMode(TSubclassOf<URPGCameraMode> CameraMode);

	// Clears the ability's camera mode.  Automatically called if needed when the ability ends.
	UFUNCTION(BlueprintCallable, Category = "RPG|Ability")
	void ClearCameraMode();

	void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
	{
		NativeOnAbilityFailedToActivate(FailedReason);
		ScriptOnAbilityFailedToActivate(FailedReason);
	}

protected:

	// Called when the ability fails to activate
	virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	// Called when the ability fails to activate
	UFUNCTION(BlueprintImplementableEvent)
	void ScriptOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	//~End of UGameplayAbility interface

	virtual void OnPawnAvatarSet();

	/** Called when this ability is granted to the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	void K2_OnAbilityAdded();

	/** Called when this ability is removed from the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
	void K2_OnAbilityRemoved();

	/** Called when the ability system is initialized with a pawn avatar. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
	void K2_OnPawnAvatarSet();

protected:

	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Ability Activation")
	ERPGAbilityActivationPolicy ActivationPolicy;

	// Defines the relationship between this ability activating and other abilities activating.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Ability Activation")
	ERPGAbilityActivationGroup ActivationGroup;

	// Map of failure tags to simple error messages
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

	// Map of failure tags to anim montages that should be played with them
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

	// If true, extra information should be logged when this ability is canceled.
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	bool bLogCancelation;

	// Current camera mode set by the ability.
	TSubclassOf<URPGCameraMode> ActiveCameraMode;
};
