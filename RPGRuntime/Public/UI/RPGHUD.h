// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/HUD.h"
#include "RPGHUD.generated.h"

/**
 * ARPGHUD
 *
 *	The base HUD class used by this project.
 *  Stub for standaloning.
 */
UCLASS(Config = Game)
class RPGRUNTIME_API ARPGHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARPGHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	//~UObject interface
	virtual void PreInitializeComponents() override;
	//~End of UObject interface

	//~AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~AHUD interface
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
	//~End of AHUD interface
};
