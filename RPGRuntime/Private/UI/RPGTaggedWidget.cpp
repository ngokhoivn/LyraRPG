// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RPGTaggedWidget.h"
#include "Components/SlateWrapperTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGTaggedWidget)

URPGTaggedWidget::URPGTaggedWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ShownVisibility = ESlateVisibility::Visible;
	HiddenVisibility = ESlateVisibility::Collapsed;
}

void URPGTaggedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsDesignTime())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn()))
		{
			// Combine tags to listen to
			FGameplayTagContainer AllTags = HiddenByTags;
			AllTags.AppendTags(ShownByTags);

			for (auto It = AllTags.CreateConstIterator(); It; ++It)
			{
				ASC->RegisterGameplayTagEvent(*It, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &URPGTaggedWidget::OnWatchedTagsChanged);
			}
		}

		OnWatchedTagsChanged(FGameplayTag(), 0);
	}
}

void URPGTaggedWidget::NativeDestruct()
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn()))
	{
		ASC->RegisterGameplayTagEvent(FGameplayTag(), EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}

	Super::NativeDestruct();
}

void URPGTaggedWidget::OnWatchedTagsChanged(const FGameplayTag Tag, int32 NewCount)
{
	bool bShouldBeVisible = true;

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn()))
	{
		if (HiddenByTags.Num() > 0 && ASC->HasAnyMatchingGameplayTags(HiddenByTags))
		{
			bShouldBeVisible = false;
		}

		if (bShouldBeVisible && ShownByTags.Num() > 0 && !ASC->HasAllMatchingGameplayTags(ShownByTags))
		{
			bShouldBeVisible = false;
		}
	}
	else
	{
		// If no ASC, hide it if it requires tags
		if (ShownByTags.Num() > 0)
		{
			bShouldBeVisible = false;
		}
	}

	SetVisibility(bShouldBeVisible ? ShownVisibility : HiddenVisibility);
}
