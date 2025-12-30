// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * URPGExperienceManager
 * 
 * Static helper class for managing experience-related plugins.
 */
class URPGExperienceManager
{
public:
	// Tracks that a plugin has been activated by an experience
	static void NotifyOfPluginActivation(const FString& PluginURL);

	// Tracks that a plugin is no longer needed by an experience, returns true if it should actually be deactivated
	static bool RequestToDeactivatePlugin(const FString& PluginURL);

private:
	// Tracking map for plugin URLs and their reference counts
	static TMap<FString, int32> ActivatedPluginCounts;
};
