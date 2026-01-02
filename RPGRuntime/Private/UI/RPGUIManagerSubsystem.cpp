// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RPGUIManagerSubsystem.h"
#include "UI/RPGUIPolicy.h"
#include "Engine/GameInstance.h"
#include "CommonLocalPlayer.h"
#include "Engine/LocalPlayer.h"
#include "System/RPGLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGUIManagerSubsystem)

URPGUIManagerSubsystem::URPGUIManagerSubsystem()
{
}

void URPGUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameInstance* GameInstance = GetGameInstance();
	GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &URPGUIManagerSubsystem::OnLocalPlayerAdded);
	GameInstance->OnLocalPlayerRemovedEvent.AddUObject(this, &URPGUIManagerSubsystem::OnLocalPlayerRemoved);

	if (!DefaultPolicyClass.IsNull())
	{
		UClass* PolicyClass = DefaultPolicyClass.LoadSynchronous();
		if (PolicyClass)
		{
			UE_LOG(LogRPG, Display, TEXT("RPGUIManagerSubsystem: Loading Default Policy [%s]"), *GetNameSafe(PolicyClass));
			URPGUIPolicy* NewPolicy = NewObject<URPGUIPolicy>(this, PolicyClass);
			SwitchToPolicy(NewPolicy);
		}
	}
	else
	{
		UE_LOG(LogRPG, Warning, TEXT("RPGUIManagerSubsystem: DefaultPolicyClass is NULL! Check DefaultGame.ini"));
	}
}

void URPGUIManagerSubsystem::Deinitialize()
{
	SwitchToPolicy(nullptr);
	Super::Deinitialize();
}

void URPGUIManagerSubsystem::OnLocalPlayerAdded(ULocalPlayer* LocalPlayer)
{
	if (CurrentPolicy)
	{
		if (UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(LocalPlayer))
		{
			CurrentPolicy->NotifyPlayerAdded(CommonLocalPlayer);
		}
	}
}

void URPGUIManagerSubsystem::OnLocalPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (CurrentPolicy)
	{
		if (UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(LocalPlayer))
		{
			CurrentPolicy->NotifyPlayerRemoved(CommonLocalPlayer);
		}
	}
}

void URPGUIManagerSubsystem::SwitchToPolicy(URPGUIPolicy* NewPolicy)
{
	if (CurrentPolicy != NewPolicy)
	{
		if (CurrentPolicy)
		{
			for (auto It = GetGameInstance()->GetLocalPlayerIterator(); It; ++It)
			{
				UCommonLocalPlayer* LocalPlayer = Cast<UCommonLocalPlayer>(*It);
				if (LocalPlayer)
				{
					CurrentPolicy->NotifyPlayerRemoved(LocalPlayer);
				}
			}
		}

		CurrentPolicy = NewPolicy;

		if (CurrentPolicy)
		{
			for (auto It = GetGameInstance()->GetLocalPlayerIterator(); It; ++It)
			{
				UCommonLocalPlayer* LocalPlayer = Cast<UCommonLocalPlayer>(*It);
				if (LocalPlayer)
				{
					CurrentPolicy->NotifyPlayerAdded(LocalPlayer);
				}
			}
		}
	}
}
