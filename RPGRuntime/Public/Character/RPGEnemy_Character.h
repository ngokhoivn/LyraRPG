// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Character/RPGCharacter.h"
#include "RPGEnemy_Character.generated.h"

/**
 * ARPGEnemy_Character
 * 
 * Base class for enemy characters with AI-specific logic.
 */
UCLASS()
class RPGRUNTIME_API ARPGEnemy_Character : public ARPGCharacter
{
	GENERATED_BODY()

public:
	ARPGEnemy_Character(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ARPGCharacter interface
	virtual void OnDeathStarted(AActor* OwningActor) override;
	//~End of ARPGCharacter interface
};
