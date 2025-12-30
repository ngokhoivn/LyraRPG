// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/RPGAssetManager.h"
#include "System/RPGLogChannels.h"
#include "System/RPGGameplayTags.h"
#include "System/RPGGameData.h"
#include "Character/RPGPawnData.h"
#include "GameMode/RPGExperienceDefinition.h"
#include "GameMode/RPGExperienceActionSet.h"
#include "AbilitySystemGlobals.h"
#include "Misc/App.h"
#include "Stats/StatsMisc.h"
#include "Engine/Engine.h"
#include "Misc/ScopedSlowTask.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGAssetManager)

const FName FRPGBundles::Equipped("Equipped");

//////////////////////////////////////////////////////////////////////

static FAutoConsoleCommand CVarDumpRPGAssets(
	TEXT("RPG.DumpLoadedAssets"),
	TEXT("Shows all assets that were loaded via the RPG asset manager and are currently in memory."),
	FConsoleCommandDelegate::CreateStatic(URPGAssetManager::DumpLoadedAssets)
);

//////////////////////////////////////////////////////////////////////

#define STARTUP_JOB_WEIGHTED(JobFunc, JobWeight) StartupJobs.Add(FRPGAssetManagerStartupJob(#JobFunc, FRPGAssetManagerStartupJob::FRPGAssetManagerStartupJobDelegate::CreateLambda([this](const FRPGAssetManagerStartupJob& StartupJob, TSharedPtr<FStreamableHandle>& LoadHandle){JobFunc;}), JobWeight))
#define STARTUP_JOB(JobFunc) STARTUP_JOB_WEIGHTED(JobFunc, 1.f)

//////////////////////////////////////////////////////////////////////

URPGAssetManager::URPGAssetManager()
{
}

URPGAssetManager& URPGAssetManager::Get()
{
	check(GEngine);

	if (URPGAssetManager* Singleton = Cast<URPGAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(LogRPG, Error, TEXT("URPGAssetManager is not set as the AssetManagerClassName in DefaultEngine.ini. Fallback to base class functionality."));

	return *NewObject<URPGAssetManager>();
}

void URPGAssetManager::DumpLoadedAssets()
{
	UE_LOG(LogRPG, Log, TEXT("========== Start Dumping Loaded RPG Assets =========="));

	// LoadedAssets is private in ULyraAssetManager and cannot be accessed here.
	// We could implement a tracked list in RPGAssetManager if needed, or call Super if it existed.
	UE_LOG(LogRPG, Log, TEXT("  (Asset tracking dump not available in specialized RPGAssetManager)"));

	UE_LOG(LogRPG, Log, TEXT("========== Finish Dumping Loaded RPG Assets =========="));
}

void URPGAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// Register RPG specific Primary Asset Types
	// Many of these could also be configured in DefaultGame.ini, 
	// but adding them here ensures they are always scanned.
	
	// RPG Experience Definitions
	PrimaryAssetTypesToScan.Add(FPrimaryAssetTypeInfo(TEXT("RPGExperienceDefinition"), URPGExperienceDefinition::StaticClass(), FPrimaryAssetId::InvalidPrimaryAssetId, true, true));

	// RPG Experience Action Sets
	PrimaryAssetTypesToScan.Add(FPrimaryAssetTypeInfo(TEXT("RPGExperienceActionSet"), URPGExperienceActionSet::StaticClass(), FPrimaryAssetId::InvalidPrimaryAssetId, true, true));

	// Initialize Native Tags
	STARTUP_JOB(FRPGGameplayTags::InitializeNativeTags());

	{
		// Load base game data asset
		STARTUP_JOB_WEIGHTED(GetGameData(), 25.f);
	}

	DoAllStartupJobs();
}

const URPGGameData& URPGAssetManager::GetGameData()
{
	// RPGGameDataPath is already defined in LyraAssetManager as LyraGameDataPath or similar?
	// Actually, ULyraAssetManager likely has LyraGameDataPath.
	// If it doesn't have RPGGameDataPath, then RPGAssetManager.h should have kept it BUT renamed it if there's a conflict.
	// But the error didn't mention RPGGameDataPath conflict, only GameDataMap, DefaultPawnData, and LoadedAssets.
	// So RPGGameDataPath is fine.
	
	return GetOrLoadTypedGameData<URPGGameData>(RPGGameDataPath);
}

const URPGPawnData* URPGAssetManager::GetDefaultPawnData() const
{
	return GetAsset(RPGDefaultPawnData);
}

UPrimaryDataAsset* URPGAssetManager::LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	if (!DataClassPath.IsNull())
	{
		UE_LOG(LogRPG, Log, TEXT("Loading RPG GameData: %s ..."), *DataClassPath.ToString());

		if (GIsEditor)
		{
			Asset = DataClassPath.LoadSynchronous();
			LoadPrimaryAssetsWithType(PrimaryAssetType);
		}
		else
		{
			TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);
			if (Handle.IsValid())
			{
				Handle->WaitUntilComplete(0.0f, false);
				Asset = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());
			}
		}
	}

	if (Asset)
	{
		GameDataMap.Add(DataClass, Asset);
	}

	return Asset;
}

void URPGAssetManager::DoAllStartupJobs()
{
	const double AllStartupJobsStartTime = FPlatformTime::Seconds();

	for (const FRPGAssetManagerStartupJob& StartupJob : StartupJobs)
	{
		StartupJob.DoJob();
	}

	StartupJobs.Empty();

	UE_LOG(LogRPG, Display, TEXT("All RPG startup jobs took %.2f seconds to complete"), FPlatformTime::Seconds() - AllStartupJobsStartTime);
}

void URPGAssetManager::UpdateInitialGameContentLoadPercent(float GameContentPercent)
{
}

#if WITH_EDITOR
void URPGAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);

	GetGameData();
}
#endif
