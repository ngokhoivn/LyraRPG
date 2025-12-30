// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RPGPawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGPawnData)

URPGPawnData::URPGPawnData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PawnClass = nullptr;
	InputConfig = nullptr;
	DefaultCameraMode = nullptr;
}
