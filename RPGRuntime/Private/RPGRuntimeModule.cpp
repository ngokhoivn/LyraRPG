// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGRuntimeModule.h"
#include "System/RPGGameplayTags.h"

#define LOCTEXT_NAMESPACE "FRPGRuntimeModule"

void FRPGRuntimeModule::StartupModule()
{
}

void FRPGRuntimeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.
	// For modules that support dynamic reloading, we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRPGRuntimeModule, RPGRuntime)
