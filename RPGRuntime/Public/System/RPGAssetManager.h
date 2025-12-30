// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "System/LyraAssetManager.h"
#include "System/RPGLogChannels.h"
#include "System/RPGAssetManagerStartupJob.h"
#include "Templates/SubclassOf.h"
#include "RPGAssetManager.generated.h"

class UPrimaryDataAsset;
class URPGGameData;
class URPGPawnData;

struct FRPGBundles
{
	static const FName Equipped;
};

/**
 * URPGAssetManager
 *
 * Specialized version of LyraAssetManager for the RPG plugin.
 * Inherits from ULyraAssetManager to maintain compatibility with Lyra's Game Feature Actions.
 */
UCLASS(Config = Game)
class RPGRUNTIME_API URPGAssetManager : public ULyraAssetManager
{
	GENERATED_BODY()

public:

	URPGAssetManager();

	// Returns the AssetManager singleton object.
	static URPGAssetManager& Get();

	// Returns the asset referenced by a TSoftObjectPtr. This will synchronously load the asset if it's not already loaded.
	template<typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// Returns the subclass referenced by a TSoftClassPtr. This will synchronously load the asset if it's not already loaded.
	template<typename AssetType>
	static TSubclassOf<AssetType> GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// Logs all assets currently loaded and tracked by the asset manager.
	static void DumpLoadedAssets();

	const URPGGameData& GetGameData();
	const URPGPawnData* GetDefaultPawnData() const;

protected:
	template <typename GameDataClass>
	const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
	{
		if (TObjectPtr<UPrimaryDataAsset> const * pResult = GameDataMap.Find(GameDataClass::StaticClass()))
		{
			return *CastChecked<GameDataClass>(*pResult);
		}

		// Does a blocking load if needed
		UPrimaryDataAsset* LoadedAsset = LoadGameDataOfClass(GameDataClass::StaticClass(), DataPath, GameDataClass::StaticClass()->GetFName());
		if (LoadedAsset == nullptr)
		{
			UE_LOG(LogRPG, Error, TEXT("RPGGameData is missing! The Editor will continue but features may not work. Please create a Data Asset of type RPGGameData and configure its path in DefaultGame.ini: [/Script/RPGRuntime.RPGAssetManager] RPGGameDataPath=\"/Path/To/Asset\""));
			
			// Return a dummy object to prevent the reference crash during startup
			return *GetMutableDefault<GameDataClass>();
		}

		return *CastChecked<const GameDataClass>(LoadedAsset);
	}

	//~UAssetManager interface
	virtual void StartInitialLoading() override;
#if WITH_EDITOR
	virtual void PreBeginPIE(bool bStartSimulate) override;
#endif
	//~End of UAssetManager interface

	UPrimaryDataAsset* LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType);

protected:
	// Global game data asset to use for the RPG plugin.
	UPROPERTY(Config)
	TSoftObjectPtr<URPGGameData> RPGGameDataPath;

	// Pawn data used when spawning player pawns if there isn't one set on the player state.
	UPROPERTY(Config)
	TSoftObjectPtr<URPGPawnData> RPGDefaultPawnData;

private:
	// Flushes the StartupJobs array. Processes all startup work.
	void DoAllStartupJobs();

	// Called periodically during loads, could be used to feed the status to a loading screen
	void UpdateInitialGameContentLoadPercent(float GameContentPercent);

	// The list of tasks to execute on startup. Used to track startup progress.
	TArray<FRPGAssetManagerStartupJob> StartupJobs;
};


template<typename AssetType>
AssetType* URPGAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedAsset = AssetPointer.Get();
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), *AssetPointer.ToString());
		}

		if (LoadedAsset && bKeepInMemory)
		{
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
	}

	return LoadedAsset;
}

template<typename AssetType>
TSubclassOf<AssetType> URPGAssetManager::GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubclass;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedSubclass = AssetPointer.Get();
		if (!LoadedSubclass)
		{
			LoadedSubclass = Cast<UClass>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedSubclass, TEXT("Failed to load asset class [%s]"), *AssetPointer.ToString());
		}

		if (LoadedSubclass && bKeepInMemory)
		{
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));
		}
	}

	return LoadedSubclass;
}
