// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/RPGExperienceManager.h"

TMap<FString, int32> URPGExperienceManager::ActivatedPluginCounts;

void URPGExperienceManager::NotifyOfPluginActivation(const FString& PluginURL)
{
	int32& Count = ActivatedPluginCounts.FindOrAdd(PluginURL);
	Count++;
}

bool URPGExperienceManager::RequestToDeactivatePlugin(const FString& PluginURL)
{
	if (int32* Count = ActivatedPluginCounts.Find(PluginURL))
	{
		(*Count)--;
		if (*Count <= 0)
		{
			ActivatedPluginCounts.Remove(PluginURL);
			return true;
		}
	}

	return false;
}
