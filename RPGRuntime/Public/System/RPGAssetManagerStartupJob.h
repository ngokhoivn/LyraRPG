// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Templates/SharedPointer.h"

struct FStreamableHandle;
class URPGAssetManager;

/**
 * FRPGAssetManagerStartupJob
 * 
 * Represents a task to be executed during the RPGAssetManager's initial loading phase.
 * Standalone version of LyraAssetManagerStartupJob.
 */
struct FRPGAssetManagerStartupJob
{
	DECLARE_DELEGATE_TwoParams(FRPGAssetManagerStartupJobDelegate, const FRPGAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&);

	FRPGAssetManagerStartupJobDelegate JobDelegate;
	FString JobName;
	float JobWeight;
	TSharedPtr<FStreamableHandle> Handle;

	FRPGAssetManagerStartupJob()
		: JobName(TEXT("Unknown"))
		, JobWeight(1.0f)
	{
	}

	FRPGAssetManagerStartupJob(const FString& InJobName, const FRPGAssetManagerStartupJobDelegate& InJobDelegate, float InJobWeight = 1.0f)
		: JobDelegate(InJobDelegate)
		, JobName(InJobName)
		, JobWeight(InJobWeight)
	{
	}

	void DoJob() const
	{
		TSharedPtr<FStreamableHandle> MutableHandle;
		JobDelegate.ExecuteIfBound(*this, MutableHandle);
	}
};
