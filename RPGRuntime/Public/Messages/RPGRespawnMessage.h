// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RPGRespawnMessage.generated.h"

class APlayerState;

/**
 * FRPGRespawnTimerDurationMessage
 *
 * Message sent to UI to start the respawn countdown.
 */
USTRUCT(BlueprintType)
struct FRPGRespawnTimerDurationMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = Respawn)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = Respawn)
	float RespawnDelay = 0.0f;
};
