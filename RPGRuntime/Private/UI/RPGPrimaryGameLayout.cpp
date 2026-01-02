// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RPGPrimaryGameLayout.h"

#include "CommonLocalPlayer.h"
#include "Engine/GameInstance.h"
#include "UI/RPGUIManagerSubsystem.h"
#include "UI/RPGUIPolicy.h"
#include "Kismet/GameplayStatics.h"
#include "System/RPGGameplayTags.h"
#include "System/RPGLogChannels.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGPrimaryGameLayout)

class UObject;

/*static*/ URPGPrimaryGameLayout* URPGPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	APlayerController* PlayerController = GameInstance->GetPrimaryPlayerController(false);
	return GetPrimaryGameLayout(PlayerController);
}

/*static*/ URPGPrimaryGameLayout* URPGPrimaryGameLayout::GetPrimaryGameLayout(APlayerController* PlayerController)
{
	return PlayerController ? GetPrimaryGameLayout(Cast<UCommonLocalPlayer>(PlayerController->Player)) : nullptr;
}

/*static*/ URPGPrimaryGameLayout* URPGPrimaryGameLayout::GetPrimaryGameLayout(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
	{
		const UCommonLocalPlayer* CommonLocalPlayer = CastChecked<UCommonLocalPlayer>(LocalPlayer);
		if (const UGameInstance* GameInstance = CommonLocalPlayer->GetGameInstance())
		{
			if (URPGUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<URPGUIManagerSubsystem>())
			{
				if (const URPGUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
				{
					if (URPGPrimaryGameLayout* RootLayout = Policy->GetRootLayout(CommonLocalPlayer))
					{
						return RootLayout;
					}
					else
					{
						UE_LOG(LogRPG, Verbose, TEXT("GetPrimaryGameLayout: Policy found but RootLayout is NULL for Player [%s]"), *GetNameSafe(CommonLocalPlayer));
					}
				}
				else
				{
					UE_LOG(LogRPG, Verbose, TEXT("GetPrimaryGameLayout: UIManager found but CurrentUIPolicy is NULL!"));
				}
			}
			else
			{
				UE_LOG(LogRPG, Verbose, TEXT("GetPrimaryGameLayout: URPGUIManagerSubsystem NOT FOUND in GameInstance!"));
			}
		}
	}

	return nullptr;
}

URPGPrimaryGameLayout::URPGPrimaryGameLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URPGPrimaryGameLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Automatically register layers based on bound widgets
	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();

	RegisterLayer(GameplayTags.UI_Layer_Game, GameLayer_Stack);
	RegisterLayer(GameplayTags.UI_Layer_GameMenu, GameMenu_Stack);
	RegisterLayer(GameplayTags.UI_Layer_Menu, Menu_Stack);
	
	if (Modal_Stack)
	{
		RegisterLayer(GameplayTags.UI_Layer_Modal, Modal_Stack);
	}
}

void URPGPrimaryGameLayout::SetIsDormant(bool InDormant)
{
	if (bIsDormant != InDormant)
	{
		const ULocalPlayer* LP = GetOwningLocalPlayer();
		const int32 PlayerId = LP ? LP->GetControllerId() : -1;
		const TCHAR* OldDormancyStr = bIsDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* NewDormancyStr = InDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* PrimaryPlayerStr = LP && LP->IsPrimaryPlayer() ? TEXT("[Primary]") : TEXT("[Non-Primary]");
		UE_LOG(LogRPG, Display, TEXT("%s RPGPrimaryGameLayout Dormancy changed for [%d] from [%s] to [%s]"), PrimaryPlayerStr, PlayerId, OldDormancyStr, NewDormancyStr);

		bIsDormant = InDormant;
		OnIsDormantChanged();
	}
}

void URPGPrimaryGameLayout::OnIsDormantChanged()
{
	// Logic for dormancy can be added here if needed, similar to Lyra
}

void URPGPrimaryGameLayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!IsDesignTime())
	{
		// Register a layer that widgets can be pushed onto. 
		// The LayerWidget can be any subclass of UCommonActivatableWidgetContainerBase.
		// - Use UCommonActivatableWidgetStack for standard LIFO behavior (e.g., Menus, HUD).
		// - Use UCommonActivatableWidgetQueue for sequential display (e.g., Modal notifications).
		
		LayerWidget->OnTransitioningChanged.AddUObject(this, &URPGPrimaryGameLayout::OnWidgetStackTransitioning);
		
		// We set transition duration to 0 by default to ensure focus transitions immediately.
		LayerWidget->SetTransitionDuration(0.0);
		
		Layers.Add(LayerTag, LayerWidget);
		
		UE_LOG(LogRPG, Verbose, TEXT("Registered UI Layer [%s] with widget [%s]"), *LayerTag.ToString(), *GetNameSafe(LayerWidget));
	}
}

void URPGPrimaryGameLayout::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	if (bIsTransitioning)
	{
		const FName SuspendToken = UCommonUIExtensions::SuspendInputForPlayer(GetOwningLocalPlayer(), TEXT("GlobalStackTransition"));
		SuspendInputTokens.Add(SuspendToken);
	}
	else
	{
		if (ensure(SuspendInputTokens.Num() > 0))
		{
			const FName SuspendToken = SuspendInputTokens.Pop();
			UCommonUIExtensions::ResumeInputForPlayer(GetOwningLocalPlayer(), SuspendToken);
		}
	}
}

void URPGPrimaryGameLayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	for (const auto& LayerKVP : Layers)
	{
		LayerKVP.Value->RemoveWidget(*ActivatableWidget);
	}
}

UCommonActivatableWidgetContainerBase* URPGPrimaryGameLayout::GetLayerWidget(FGameplayTag LayerName)
{
	return Layers.FindRef(LayerName);
}
