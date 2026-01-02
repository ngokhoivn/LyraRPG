// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameMode/RPGGameFeatureAction_WorldActionBase.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "RPGGameFeatureAction_AddWidgets.generated.h"

class UCommonActivatableWidget;
struct FComponentRequestHandle;

USTRUCT()
struct FRPGHUDLayoutRequest
{
	GENERATED_BODY()

	/** The widget to add. */
	UPROPERTY(EditAnywhere, Category=UI)
	TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

	/** The layer to add the widget to. */
	UPROPERTY(EditAnywhere, Category=UI, meta=(Categories="UI.Layer"))
	FGameplayTag LayerID;
};

/**
 * GameFeatureAction that adds widgets to the player's UI.
 */
UCLASS(meta = (DisplayName = "Add Widgets"))
class RPGRUNTIME_API URPGGameFeatureAction_AddWidgets final : public URPGGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction interface
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~ End UGameFeatureAction interface

	//~ Begin URPGGameFeatureAction_WorldActionBase interface
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~ End URPGGameFeatureAction_WorldActionBase interface

	UPROPERTY(EditAnywhere, Category=UI)
	TArray<FRPGHUDLayoutRequest> Layout;

private:
	struct FPerActorData
	{
		TArray<TWeakObjectPtr<UCommonActivatableWidget>> LayoutsAdded;
	};

	struct FPerContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
		TMap<FObjectKey, FPerActorData> ActorData;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	void AddWidgets(AActor* Actor, FPerContextData& ActiveData);
	void RemoveWidgets(AActor* Actor, FPerContextData& ActiveData);

	void Reset(FPerContextData& ActiveData);
};
