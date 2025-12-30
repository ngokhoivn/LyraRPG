// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GenericTeamAgentInterface.h"
#include "UObject/Interface.h"
#include "RPGTeamAgentInterface.generated.h"

template <typename InterfaceType> class TScriptInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRPGTeamIndexChangedDelegate, UObject*, ObjectChangingTeam, int32, OldTeamID, int32, NewTeamID);

RPGRUNTIME_API int32 GenericTeamIdToInteger(FGenericTeamId ID);
RPGRUNTIME_API FGenericTeamId IntegerToGenericTeamId(int32 ID);

/**
 * Interface for actors that have a team assignment.
 * Standalone version of LyraTeamAgentInterface.
 */
UINTERFACE(BlueprintType, meta=(CannotImplementInterfaceInBlueprint))
class URPGTeamAgentInterface : public UGenericTeamAgentInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IRPGTeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_IINTERFACE_BODY()

	virtual FOnRPGTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }

	static void ConditionalBroadcastTeamChanged(TScriptInterface<IRPGTeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);
	
	FOnRPGTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
	{
		FOnRPGTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
		check(Result);
		return *Result;
	}
};
