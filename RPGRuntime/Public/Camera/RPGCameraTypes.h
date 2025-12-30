// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RPGCameraTypes.generated.h"

/**
 * FRPGPenetrationAvoidanceFeeler
 *
 *	Struct used to define a feeler ray used for camera penetration avoidance.
 *  Standalone version of FLyraPenetrationAvoidanceFeeler.
 */
USTRUCT(BlueprintType)
struct FRPGPenetrationAvoidanceFeeler
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	FRotator AdjustmentRot;

	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	float WorldWeight;

	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	float PawnWeight;

	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	float Extent;

	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	int32 TraceInterval;

	UPROPERTY(Transient)
	int32 FramesUntilNextTrace;

	FRPGPenetrationAvoidanceFeeler()
		: AdjustmentRot(ForceInit)
		, WorldWeight(1.0f)
		, PawnWeight(1.0f)
		, Extent(0.0f)
		, TraceInterval(0)
		, FramesUntilNextTrace(0)
	{}

	FRPGPenetrationAvoidanceFeeler(FRotator InAdjustmentRot, float InWorldWeight, float InPawnWeight, float InExtent, int32 InTraceInterval = 0)
		: AdjustmentRot(InAdjustmentRot)
		, WorldWeight(InWorldWeight)
		, PawnWeight(InPawnWeight)
		, Extent(InExtent)
		, TraceInterval(InTraceInterval)
		, FramesUntilNextTrace(0)
	{}
};
