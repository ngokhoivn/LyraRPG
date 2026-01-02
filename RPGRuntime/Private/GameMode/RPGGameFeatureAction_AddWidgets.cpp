// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/RPGGameFeatureAction_AddWidgets.h"
#include "Components/GameFrameworkComponentManager.h"
#include "UI/RPGPrimaryGameLayout.h"
#include "UI/RPGHUD.h"
#include "CommonLocalPlayer.h"
#include "CommonUIExtensions.h"
#include "CommonActivatableWidget.h"
#include "Engine/GameInstance.h"
#include "System/RPGLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGameFeatureAction_AddWidgets)

void URPGGameFeatureAction_AddWidgets::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ActiveData)
	{
		Reset(*ActiveData);
	}
}

void URPGGameFeatureAction_AddWidgets::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			UE_LOG(LogRPG, Verbose, TEXT("URPGGameFeatureAction_AddWidgets::AddToWorld: Registering extension handler for ARPGHUD"));
			
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(
				ARPGHUD::StaticClass(),
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, ChangeContext));
			ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
		}
	}
}

void URPGGameFeatureAction_AddWidgets::Reset(FPerContextData& ActiveData)
{
	ActiveData.ComponentRequests.Empty();
	ActiveData.ActorData.Empty();
}

void URPGGameFeatureAction_AddWidgets::HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);
	
	UE_LOG(LogRPG, Verbose, TEXT("URPGGameFeatureAction_AddWidgets::HandleActorExtension: Actor [%s], Event [%s]"), *GetNameSafe(Actor), *EventName.ToString());

	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveWidgets(Actor, ActiveData);
	}
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == UGameFrameworkComponentManager::NAME_GameActorReady))
	{
		AddWidgets(Actor, ActiveData);
	}
}

void URPGGameFeatureAction_AddWidgets::AddWidgets(AActor* Actor, FPerContextData& ActiveData)
{
	ARPGHUD* HUD = CastChecked<ARPGHUD>(Actor);

	if (HUD->GetOwningPlayerController())
	{
		if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(HUD->GetOwningPlayerController()->Player))
		{
			FPerActorData& ActorData = ActiveData.ActorData.FindOrAdd(HUD);

			for (const FRPGHUDLayoutRequest& Entry : Layout)
			{
				if (TSubclassOf<UCommonActivatableWidget> ConcreteWidgetClass = Entry.LayoutClass.LoadSynchronous())
				{
					UE_LOG(LogRPG, Verbose, TEXT("URPGGameFeatureAction_AddWidgets::AddWidgets: Pushing layout [%s] to layer [%s]"), *GetNameSafe(ConcreteWidgetClass), *Entry.LayerID.ToString());
					
					UCommonActivatableWidget* AddedWidget = nullptr;
					if (URPGPrimaryGameLayout* RootLayout = URPGPrimaryGameLayout::GetPrimaryGameLayout(LocalPlayer))
					{
						AddedWidget = RootLayout->PushWidgetToLayerStack(Entry.LayerID, ConcreteWidgetClass);
					}
					else
					{
						UE_LOG(LogRPG, Error, TEXT("URPGGameFeatureAction_AddWidgets::AddWidgets: Failed to find Root Layout for Player [%s]"), *GetNameSafe(LocalPlayer));
					}

					if (AddedWidget)
					{
						ActorData.LayoutsAdded.Add(AddedWidget);
					}
					else
					{
						UE_LOG(LogRPG, Error, TEXT("URPGGameFeatureAction_AddWidgets::AddWidgets: Failed to push widget [%s]! Either Root Layout is missing or Layer [%s] not registered."), *GetNameSafe(ConcreteWidgetClass), *Entry.LayerID.ToString());
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogRPG, Warning, TEXT("URPGGameFeatureAction_AddWidgets::AddWidgets: HUD has no owning player controller yet."));
	}
}

void URPGGameFeatureAction_AddWidgets::RemoveWidgets(AActor* Actor, FPerContextData& ActiveData)
{
	ARPGHUD* HUD = CastChecked<ARPGHUD>(Actor);
	FPerActorData* ActorData = ActiveData.ActorData.Find(HUD);

	if (ActorData)
	{
		for (TWeakObjectPtr<UCommonActivatableWidget>& AddedLayout : ActorData->LayoutsAdded)
		{
			if (AddedLayout.IsValid())
			{
				AddedLayout->DeactivateWidget();
			}
		}

		ActiveData.ActorData.Remove(HUD);
	}
}
