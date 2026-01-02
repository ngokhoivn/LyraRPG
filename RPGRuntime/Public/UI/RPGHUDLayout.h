// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UI/RPGActivatableWidget.h"
#include "RPGHUDLayout.generated.h"

/**
 * URPGHUDLayout
 *
 * Base class for the HUD layout in the RPG plugin.
 * Inherits from RPGActivatableWidget.
 */
UCLASS(Abstract, Blueprintable)
class RPGRUNTIME_API URPGHUDLayout : public URPGActivatableWidget
{
	GENERATED_BODY()

public:
	URPGHUDLayout(const FObjectInitializer& ObjectInitializer);
};
