// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RPGCameraAssistInterface.generated.h"

/**
 * URPGCameraAssistInterface
 * 
 * Interface for actors that provide assistance to the camera system.
 */
UINTERFACE(MinimalAPI, BlueprintType, Meta = (CannotImplementInterfaceInBlueprint))
class URPGCameraAssistInterface : public UInterface
{
	GENERATED_BODY()
};

class RPGRUNTIME_API IRPGCameraAssistInterface
{
	GENERATED_BODY()

public:
	/** Called when the camera penetrations the target actor */
	virtual void OnCameraPenetratingTarget() = 0;

	/** Gets the target actor to use for camera penetration avoidance */
	virtual TOptional<AActor*> GetCameraPreventPenetrationTarget() { return TOptional<AActor*>(); }
};
