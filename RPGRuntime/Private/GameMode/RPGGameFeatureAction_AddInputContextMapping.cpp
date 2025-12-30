#include "GameMode/RPGGameFeatureAction_AddInputContextMapping.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Character/RPGHeroComponent.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "System/RPGAssetManager.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGameFeatureAction_AddInputContextMapping)

#define LOCTEXT_NAMESPACE "RPGGameFeatures"

//////////////////////////////////////////////////////////////////////
// URPGGameFeatureAction_AddInputContextMapping

void URPGGameFeatureAction_AddInputContextMapping::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

	RegisterInputMappingContexts();
}

void URPGGameFeatureAction_AddInputContextMapping::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	if (!ensure(ActiveData.ExtensionRequestHandles.IsEmpty()) ||
		!ensure(ActiveData.ControllersAddedTo.IsEmpty()))
	{
		Reset(ActiveData);
	}
	Super::OnGameFeatureActivating(Context);
}

void URPGGameFeatureAction_AddInputContextMapping::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

void URPGGameFeatureAction_AddInputContextMapping::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();	
	
	UnregisterInputMappingContexts();
}

void URPGGameFeatureAction_AddInputContextMapping::RegisterInputMappingContexts()
{
	RegisterInputContextMappingsForGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &URPGGameFeatureAction_AddInputContextMapping::RegisterInputContextMappingsForGameInstance);

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (TIndirectArray<FWorldContext>::TConstIterator WorldContextIterator = WorldContexts.CreateConstIterator(); WorldContextIterator; ++WorldContextIterator)
	{
		RegisterInputContextMappingsForGameInstance(WorldContextIterator->OwningGameInstance);
	}
}

void URPGGameFeatureAction_AddInputContextMapping::RegisterInputContextMappingsForGameInstance(UGameInstance* GameInstance)
{
	if (GameInstance != nullptr && !GameInstance->OnLocalPlayerAddedEvent.IsBoundToObject(this))
	{
		GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &URPGGameFeatureAction_AddInputContextMapping::RegisterInputMappingContextsForLocalPlayer);
		GameInstance->OnLocalPlayerRemovedEvent.AddUObject(this, &URPGGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContextsForLocalPlayer);
		
		for (TArray<ULocalPlayer*>::TConstIterator LocalPlayerIterator = GameInstance->GetLocalPlayerIterator(); LocalPlayerIterator; ++LocalPlayerIterator)
		{
			RegisterInputMappingContextsForLocalPlayer(*LocalPlayerIterator);
		}
	}
}

void URPGGameFeatureAction_AddInputContextMapping::RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer))
	{
		if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			if (UEnhancedInputUserSettings* Settings = EISubsystem->GetUserSettings())
			{
				for (const FRPGInputMappingContextAndPriority& Entry : InputMappings)
				{
					// Skip entries that don't want to be registered
					if (!Entry.bRegisterWithSettings)
					{
						continue;
					}

					// Register this IMC with the settings!
					if (UInputMappingContext* IMC = URPGAssetManager::GetAsset(Entry.InputMapping))
					{
						Settings->RegisterInputMappingContext(IMC);
					}
				}
			}
		}
	}
}

void URPGGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContexts()
{
	FWorldDelegates::OnStartGameInstance.Remove(RegisterInputContextMappingsForGameInstanceHandle);
	RegisterInputContextMappingsForGameInstanceHandle.Reset();

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (TIndirectArray<FWorldContext>::TConstIterator WorldContextIterator = WorldContexts.CreateConstIterator(); WorldContextIterator; ++WorldContextIterator)
	{
		UnregisterInputContextMappingsForGameInstance(WorldContextIterator->OwningGameInstance);
	}
}

void URPGGameFeatureAction_AddInputContextMapping::UnregisterInputContextMappingsForGameInstance(UGameInstance* GameInstance)
{
	if (GameInstance != nullptr)
	{
		GameInstance->OnLocalPlayerAddedEvent.RemoveAll(this);
		GameInstance->OnLocalPlayerRemovedEvent.RemoveAll(this);

		for (TArray<ULocalPlayer*>::TConstIterator LocalPlayerIterator = GameInstance->GetLocalPlayerIterator(); LocalPlayerIterator; ++LocalPlayerIterator)
		{
			UnregisterInputMappingContextsForLocalPlayer(*LocalPlayerIterator);
		}
	}
}

void URPGGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer))
	{
		if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			if (UEnhancedInputUserSettings* Settings = EISubsystem->GetUserSettings())
			{
				for (const FRPGInputMappingContextAndPriority& Entry : InputMappings)
				{
					// Skip entries that don't want to be registered
					if (!Entry.bRegisterWithSettings)
					{
						continue;
					}

					// Unregister this IMC from the settings!
					if (UInputMappingContext* IMC = Entry.InputMapping.Get())
					{
						Settings->UnregisterInputMappingContext(IMC);
					}
				}
			}
		}
	}
}


#if WITH_EDITOR
EDataValidationResult URPGGameFeatureAction_AddInputContextMapping::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 Index = 0;

	for (const FRPGInputMappingContextAndPriority& Entry : InputMappings)
	{
		if (Entry.InputMapping.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("NullInputMapping", "Null InputMapping at index {0}."), Index));
		}
		++Index;
	}

	return Result;
}
#endif

void URPGGameFeatureAction_AddInputContextMapping::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			UGameFrameworkComponentManager::FExtensionHandlerDelegate AddMappingsDelegate =
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleControllerExtension, ChangeContext);
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle =
				ComponentManager->AddExtensionHandler(APlayerController::StaticClass(), AddMappingsDelegate);

			ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
		}
	}
}

void URPGGameFeatureAction_AddInputContextMapping::Reset(FPerContextData& ActiveData)
{
	ActiveData.ExtensionRequestHandles.Empty();

	while (!ActiveData.ControllersAddedTo.IsEmpty())
	{
		TWeakObjectPtr<APlayerController> ControllerPtr = ActiveData.ControllersAddedTo.Top();
		if (ControllerPtr.IsValid())
		{
			RemoveInputMapping(ControllerPtr.Get(), ActiveData);
		}
		else
		{
			ActiveData.ControllersAddedTo.Pop();
		}
	}
}

void URPGGameFeatureAction_AddInputContextMapping::HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	APlayerController* AsController = CastChecked<APlayerController>(Actor);
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveInputMapping(AsController, ActiveData);
	}
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == URPGHeroComponent::NAME_BindInputsNow))
	{
		AddInputMappingForPlayer(AsController->GetLocalPlayer(), ActiveData);
	}
}

void URPGGameFeatureAction_AddInputContextMapping::AddInputMappingForPlayer(UPlayer* Player, FPerContextData& ActiveData)
{
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const FRPGInputMappingContextAndPriority& Entry : InputMappings)
			{
				if (const UInputMappingContext* IMC = URPGAssetManager::GetAsset(Entry.InputMapping))
				{
					InputSystem->AddMappingContext(IMC, Entry.Priority);
				}
			}
		}
		else
		{
			UE_LOG(LogGameFeatures, Error, TEXT("Failed to find `UEnhancedInputLocalPlayerSubsystem` for local player. Input mappings will not be added. Make sure you're set to use the EnhancedInput system via config file."));
		}
	}
}

void URPGGameFeatureAction_AddInputContextMapping::RemoveInputMapping(APlayerController* PlayerController, FPerContextData& ActiveData)
{
	if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const FRPGInputMappingContextAndPriority& Entry : InputMappings)
			{
				if (const UInputMappingContext* IMC = Entry.InputMapping.Get())
				{
					InputSystem->RemoveMappingContext(IMC);
				}
			}
		}
	}

	ActiveData.ControllersAddedTo.Remove(PlayerController);
}

#undef LOCTEXT_NAMESPACE
