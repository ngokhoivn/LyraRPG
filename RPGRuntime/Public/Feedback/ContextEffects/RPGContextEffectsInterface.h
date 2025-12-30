// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/HitResult.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"

#include "RPGContextEffectsInterface.generated.h"

class UAnimSequenceBase;
class UObject;
class USceneComponent;
struct FFrame;

/**
 * ERPGContextMatchType
 */
UENUM(BlueprintType)
enum class ERPGContextMatchType : uint8
{
	ExactMatch,
	BestMatch
};

/**
 * URPGContextEffectsInterface
 */
 UINTERFACE(MinimalAPI, Blueprintable)
 class URPGContextEffectsInterface : public UInterface
 {
	 GENERATED_BODY()
 };
 
 class IRPGContextEffectsInterface : public IInterface
 {
	 GENERATED_BODY()

 public:

	/** AnimMotionEffect interface call */
 	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RPG|ContextEffects")
	void AnimMotionEffect(const FName Bone
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
		, float AudioPitch = 1);
 };
