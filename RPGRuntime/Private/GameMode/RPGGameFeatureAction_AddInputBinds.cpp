// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/RPGGameFeatureAction_AddInputBinds.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Character/RPGHeroComponent.h"
#include "Input/RPGInputConfig.h"
#include "System/RPGAssetManager.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGGameFeatureAction_AddInputBinds)

#define LOCTEXT_NAMESPACE "RPGGameFeatures"

void URPGGameFeatureAction_AddInputBinds::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			UGameFrameworkComponentManager::FExtensionHandlerDelegate AddBindsDelegate =
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleControllerExtension, ChangeContext);
			
			ExtensionRequestHandles.Add(ComponentManager->AddExtensionHandler(APlayerController::StaticClass(), AddBindsDelegate));
		}
	}
}

void URPGGameFeatureAction_AddInputBinds::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	ExtensionRequestHandles.Empty();
}

void URPGGameFeatureAction_AddInputBinds::HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	APlayerController* PC = CastChecked<APlayerController>(Actor);

	UE_LOG(LogRPG, Log, TEXT("HandleControllerExtension: Event %s received for %s"), *EventName.ToString(), *GetNameSafe(PC));

	if (EventName == URPGHeroComponent::NAME_BindInputsNow || EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded)
	{
		APawn* Pawn = PC->GetPawn();
		UE_LOG(LogRPG, Log, TEXT("HandleControllerExtension: Checking Pawn: %s"), *GetNameSafe(Pawn));

		if (URPGHeroComponent* HeroComp = URPGHeroComponent::FindHeroComponent(Pawn))
		{
			if (const URPGInputConfig* LoadedConfig = URPGAssetManager::GetAsset(InputConfig))
			{
				UE_LOG(LogRPG, Log, TEXT("HandleControllerExtension: Found HeroComp and LoadedConfig %s. Calling AddAdditionalInputConfig."), *GetNameSafe(LoadedConfig));
				HeroComp->AddAdditionalInputConfig(LoadedConfig);
			}
			else
			{
				UE_LOG(LogRPG, Error, TEXT("HandleControllerExtension: InputConfig is NULL or failed to load!"));
			}
		}
		else
		{
			UE_LOG(LogRPG, Warning, TEXT("HandleControllerExtension: HeroComponent NOT found on Pawn!"));
		}
	}
}

#if WITH_EDITOR
EDataValidationResult URPGGameFeatureAction_AddInputBinds::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (InputConfig.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("NullInputConfig", "InputConfig is null."));
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
