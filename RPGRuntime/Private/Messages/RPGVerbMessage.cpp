// Copyright Epic Games, Inc. All Rights Reserved.

#include "Messages/RPGVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGVerbMessage)

FString FRPGVerbMessage::ToString() const
{
	FString InstigatorName = (Instigator != nullptr) ? Instigator->GetName() : TEXT("None");
	FString TargetName = (Target != nullptr) ? Target->GetName() : TEXT("None");

	return FString::Printf(TEXT("Verb: %s, Instigator: %s, Target: %s, Magnitude: %.2f"),
		*Verb.ToString(), *InstigatorName, *TargetName, Magnitude);
}
