// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Character/RPGCharacter.h"
#include "Feedback/ContextEffects/RPGContextEffectsInterface.h"
#include "RPGHero_Character.generated.h"

class UAnimMontage;
class USceneComponent;

/**
 * ARPGHero_Character
 * 
 * Specialized hero character with death logic and footstep effects.
 */
UCLASS()
class RPGRUNTIME_API ARPGHero_Character : public ARPGCharacter, public IRPGContextEffectsInterface
{
	GENERATED_BODY()

public:
	ARPGHero_Character(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ARPGCharacter interface
	virtual void OnDeathStarted(AActor* OwningActor) override;
	//~End of ARPGCharacter interface

	//~IRPGContextEffectsInterface
	virtual void AnimMotionEffect_Implementation(const FName Bone
		, const FGameplayTag MotionEffect
		, USceneComponent* StaticMeshComponent
		, const FVector LocationOffset
		, const FRotator RotationOffset
		, const UAnimSequenceBase* AnimationSequence
		, const bool bHitSuccess
		, const FHitResult HitResult
		, FGameplayTagContainer Contexts
		, FVector VFXScale = FVector(1)
		, float AudioVolume = 1
		, float AudioPitch = 1) override;
	//~End of IRPGContextEffectsInterface

protected:
	/** Random death montages to pick from */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Death")
	TArray<TObjectPtr<UAnimMontage>> DeathMontages;

	/** Actor class to spawn for footstep effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RPG|Effects")
	TSubclassOf<AActor> FootStepActorClass;

	/** The spawned footstep effect actor */
	UPROPERTY(Transient)
	TObjectPtr<AActor> FootStepActor;

	/** Starts ragdoll physics */
	void StartRagdoll();

	/** Hides all equipped weapons */
	void HideEquippedWeapons();
};
