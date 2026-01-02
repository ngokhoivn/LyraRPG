// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/RPGGameplayAbility.h"
#include "TimerManager.h"
#include "UIExtensionSystem.h"
#include "RPGGA_AutoRespawn.generated.h"

class URPGHealthComponent;

/**
 * URPGGA_AutoRespawn
 *
 * Ability that automatically requests a respawn after the character dies.
 */
UCLASS()
class RPGRUNTIME_API URPGGA_AutoRespawn : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGGA_AutoRespawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnPawnAvatarSet() override;

	/** Checks if the avatar is currently dead or dying */
	bool IsAvatarDeadOrDying() const;

	/** Binds to health component events */
	void BindDeathListener();

	/** Clears health component event bindings */
	void ClearDeathListener();

	/** Called when the health component signals death has started */
	UFUNCTION()
	void OnDeathStarted(AActor* OwningActor);

	/** Actually requests the respawn from the game mode */
	void ExecuteRespawn();

	/** Cancels any active death abilities */
	void EndDeathAbilities();

	/** Fallback for when the avatar is destroyed unexpectedly */
	UFUNCTION()
	void OnAvatarEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);

protected:
	/** Delay before requesting a respawn */
	UPROPERTY(EditDefaultsOnly, Category = "RPG|Respawn")
	float RespawnDelayDuration = 5.0f;

	/** Widget class to display during the respawn countdown */
	UPROPERTY(EditDefaultsOnly, Category = "RPG|Respawn")
	TSubclassOf<UUserWidget> RespawnWidgetClass;

private:
	void RegisterRespawnUI();
	void UnregisterRespawnUI();

	UPROPERTY()
	TObjectPtr<AActor> LastBoundAvatarActor;

	UPROPERTY()
	TObjectPtr<URPGHealthComponent> LastBoundHealthComponent;

	FTimerHandle RespawnTimerHandle;

	FUIExtensionHandle RespawnExtensionHandle;
};
