// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "RPGAnimNotify_SendGameplayEvent.generated.h"

/**
 * URPGAnimNotify_SendGameplayEvent
 *
 * AnimNotify that sends a gameplay event with a specific tag to the owning actor.
 */
UCLASS(const, HideCategories = Object, Meta = (DisplayName = "Send Gameplay Event"))
class RPGRUNTIME_API URPGAnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	URPGAnimNotify_SendGameplayEvent();

	//~UAnimNotify interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotify interface

protected:
	// The tag to send as a gameplay event
	UPROPERTY(EditAnywhere, Category = "AnimNotify", Meta = (Categories = "Event"))
	FGameplayTag EventTag;
};
