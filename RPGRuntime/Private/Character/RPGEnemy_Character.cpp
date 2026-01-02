// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RPGEnemy_Character.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGEnemy_Character)

ARPGEnemy_Character::ARPGEnemy_Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ARPGEnemy_Character::OnDeathStarted(AActor* OwningActor)
{
	Super::OnDeathStarted(OwningActor);

	// Unregister from AI senses (specifically Sight)
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(GetWorld()))
	{
		PerceptionSystem->UnregisterSource(*this, UAISense_Sight::StaticClass());
	}
}
