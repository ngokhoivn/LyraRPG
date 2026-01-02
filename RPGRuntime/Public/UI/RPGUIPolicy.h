// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "RPGUIPolicy.generated.h"

class URPGPrimaryGameLayout;
class ULocalPlayer;
class UCommonLocalPlayer;

/**
 * URPGUIPolicy
 *
 * The policy for how to layout the game UI.
 */
UCLASS(Abstract, Blueprintable, Within = RPGUIManagerSubsystem)
class RPGRUNTIME_API URPGUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	URPGUIPolicy(const FObjectInitializer& ObjectInitializer);

	virtual UWorld* GetWorld() const override;
	
	URPGPrimaryGameLayout* GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const;

	/** Notification that a local player has been added. */
	void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);

	/** Notification that a local player has been removed. */
	void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);

	/** Notification that a local player's primary layout has been created. */
	void NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer);

protected:
	void CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer);

	UPROPERTY(EditAnywhere, Category = "Layout")
	TSoftClassPtr<URPGPrimaryGameLayout> LayoutClass;

private:
	UPROPERTY(Transient)
	TMap<TObjectPtr<const UCommonLocalPlayer>, TObjectPtr<URPGPrimaryGameLayout>> RootLayouts;
};
