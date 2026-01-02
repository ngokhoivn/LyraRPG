// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "RPGUserWidget.generated.h"

/**
 * URPGUserWidget
 *
 * Base class for all widgets in the RPG plugin.
 * Inherits from CommonUserWidget.
 */
UCLASS(Abstract, Blueprintable)
class RPGRUNTIME_API URPGUserWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	URPGUserWidget(const FObjectInitializer& ObjectInitializer);
};
