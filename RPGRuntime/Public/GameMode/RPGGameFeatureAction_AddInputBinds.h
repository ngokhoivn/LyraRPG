// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameMode/RPGGameFeatureAction_WorldActionBase.h"
#include "UObject/SoftObjectPtr.h"
#include "RPGGameFeatureAction_AddInputBinds.generated.h"

class URPGInputConfig;

/**
 * GameFeatureAction that adds extra input configuration to the hero component.
 * Standalone version of Lyra's GameFeatureAction_AddInputBinds.
 */
UCLASS(meta = (DisplayName = "Add Input Binds"))
class RPGRUNTIME_API URPGGameFeatureAction_AddInputBinds final : public URPGGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~ Begin UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UObject interface

	UPROPERTY(EditAnywhere, Category="Input", meta=(AssetBundles="Client,Server"))
	TSoftObjectPtr<URPGInputConfig> InputConfig;

private:
	//~ Begin URPGGameFeatureAction_WorldActionBase interface
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~ End URPGGameFeatureAction_WorldActionBase interface

	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

	void HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	TArray<TSharedPtr<struct FComponentRequestHandle>> ExtensionRequestHandles;
};
