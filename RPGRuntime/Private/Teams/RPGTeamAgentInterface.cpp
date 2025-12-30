// Copyright Epic Games, Inc. All Rights Reserved.

#include "Teams/RPGTeamAgentInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGTeamAgentInterface)

URPGTeamAgentInterface::URPGTeamAgentInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

int32 GenericTeamIdToInteger(FGenericTeamId ID)
{
	return (ID == FGenericTeamId::NoTeam) ? INDEX_NONE : (int32)ID.GetId();
}

FGenericTeamId IntegerToGenericTeamId(int32 ID)
{
	return (ID == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId((uint8)ID);
}
