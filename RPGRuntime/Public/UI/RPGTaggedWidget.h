// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "UI/RPGUserWidget.h"
#include "RPGTaggedWidget.generated.h"

enum class ESlateVisibility : uint8;

/**
 * URPGTaggedWidget
 *
 * A widget that shows or hides itself based on whether the owning pawn has specific gameplay tags.
 */
UCLASS()
class RPGRUNTIME_API URPGTaggedWidget : public URPGUserWidget
{
	GENERATED_BODY()

public:
	URPGTaggedWidget(const FObjectInitializer& ObjectInitializer);

	//~UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~End of UUserWidget interface

protected:
	/** If the owning pawn has any of these tags, this widget will be hidden. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget Model")
	FGameplayTagContainer HiddenByTags;

	/** If the owning pawn HAS ALL of these tags, this widget will be shown. (If empty, this requirement is ignored) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget Model")
	FGameplayTagContainer ShownByTags;

	/** The visibility to use when the widget is shown. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget Model")
	ESlateVisibility ShownVisibility;

	/** The visibility to use when the widget is hidden. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget Model")
	ESlateVisibility HiddenVisibility;

private:
	void OnWatchedTagsChanged(const FGameplayTag Tag, int32 NewCount);

	FDelegateHandle TagChangeDelegateHandle;
};
