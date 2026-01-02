// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RPGUIPolicy.h"
#include "UI/RPGPrimaryGameLayout.h"
#include "CommonLocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "System/RPGLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGUIPolicy)

URPGUIPolicy::URPGUIPolicy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UWorld* URPGUIPolicy::GetWorld() const
{
	if (const UObject* Outer = GetOuter())
	{
		return Outer->GetWorld();
	}
	return nullptr;
}

URPGPrimaryGameLayout* URPGUIPolicy::GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const
{
	URPGPrimaryGameLayout* Layout = RootLayouts.FindRef(LocalPlayer);
	if (!Layout)
	{
		// Lazy creation if requested but not exist
		const_cast<URPGUIPolicy*>(this)->CreateLayoutWidget(const_cast<UCommonLocalPlayer*>(LocalPlayer));
		Layout = RootLayouts.FindRef(LocalPlayer);
	}
	return Layout;
}

void URPGUIPolicy::NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer)
{
	CreateLayoutWidget(LocalPlayer);
}

void URPGUIPolicy::NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer)
{
	if (URPGPrimaryGameLayout* Layout = RootLayouts.FindRef(LocalPlayer))
	{
		Layout->RemoveFromParent();
		RootLayouts.Remove(LocalPlayer);
	}
}

void URPGUIPolicy::NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);
}

void URPGUIPolicy::CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer)
{
	if (APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
	{
		if (LayoutClass.IsNull())
		{
			UE_LOG(LogRPG, Error, TEXT("RPGUIPolicy: LayoutClass is NULL in [%s]!"), *GetName());
			return;
		}

		if (UClass* LayoutWidgetClass = LayoutClass.LoadSynchronous())
		{
			UE_LOG(LogRPG, Verbose, TEXT("RPGUIPolicy: Creating Root Layout [%s] for Player [%s]"), *GetNameSafe(LayoutWidgetClass), *GetNameSafe(LocalPlayer));
			URPGPrimaryGameLayout* Layout = CreateWidget<URPGPrimaryGameLayout>(PC, LayoutWidgetClass);
			RootLayouts.Add(LocalPlayer, Layout);

			if (Layout)
			{
				Layout->SetOwningPlayer(PC);
				Layout->AddToPlayerScreen();
			}
		}
	}
	else
	{
		UE_LOG(LogRPG, Warning, TEXT("RPGUIPolicy: PlayerController not ready yet for Player [%s]. Layout creation skipped."), *GetNameSafe(LocalPlayer));
	}
}
