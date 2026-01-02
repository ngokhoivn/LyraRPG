// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "RPGPrimaryGameLayout.generated.h"

class APlayerController;
class UCommonActivatableWidgetContainerBase;
class ULocalPlayer;

/**
 * The state of an async load operation for the UI.
 */
enum class ERPGAsyncWidgetLayerState : uint8
{
	Canceled,
	Initialize,
	AfterPush
};

/**
 * URPGPrimaryGameLayout
 *
 * The primary game UI layout of your game. This widget class represents how to layout, push and display all layers
 * of the UI for a single player. Each player in a split-screen game will receive their own primary game layout.
 * 
 * Layers are registered via RegisterLayer, typically in the OnInitialized event of a Blueprint subclass.
 * Standard layers include:
 * - Game: For HUD elements (Health bars, minimaps). This is the base layer and is usually always visible.
 * - GameMenu: For gameplay-related menus (Inventory, Skill tree). Higher layers usually take input focus.
 * - Menu: For system-level menus (Settings, Main Menu).
 * - Modal: For top-level pop-ups and confirmations.
 * 
 * Each layer is a separate stack of widgets. Pushing a new widget to a layer will display it on top of 
 * existing widgets in that same layer. Widgets on different layers can be visible simultaneously.
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class RPGRUNTIME_API URPGPrimaryGameLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	static URPGPrimaryGameLayout* GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject);
	static URPGPrimaryGameLayout* GetPrimaryGameLayout(APlayerController* PlayerController);
	static URPGPrimaryGameLayout* GetPrimaryGameLayout(ULocalPlayer* LocalPlayer);

public:
	URPGPrimaryGameLayout(const FObjectInitializer& ObjectInitializer);

	/** A dormant root layout is collapsed and responds only to persistent actions registered by the owning player */
	void SetIsDormant(bool Dormant);
	bool IsDormant() const { return bIsDormant; }

public:
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass)
	{
		return PushWidgetToLayerStackAsync<ActivatableWidgetT>(LayerName, bSuspendInputUntilComplete, ActivatableWidgetClass, [](ERPGAsyncWidgetLayerState, ActivatableWidgetT*) {});
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass, TFunction<void(ERPGAsyncWidgetLayerState, ActivatableWidgetT*)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		static FName NAME_PushingWidgetToLayer("PushingWidgetToLayer");
		const FName SuspendInputToken = bSuspendInputUntilComplete ? UCommonUIExtensions::SuspendInputForPlayer(GetOwningPlayer(), NAME_PushingWidgetToLayer) : NAME_None;

		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		TSharedPtr<FStreamableHandle> StreamingHandle = StreamableManager.RequestAsyncLoad(ActivatableWidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			[this, LayerName, ActivatableWidgetClass, StateFunc, SuspendInputToken]()
			{
				UCommonUIExtensions::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);

				ActivatableWidgetT* Widget = PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass.Get(), [StateFunc](ActivatableWidgetT& WidgetToInit) {
					StateFunc(ERPGAsyncWidgetLayerState::Initialize, &WidgetToInit);
				});

				StateFunc(ERPGAsyncWidgetLayerState::AfterPush, Widget);
			})
		);

		// Setup a cancel delegate so that we can resume input if this handler is canceled.
		StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this,
			[this, StateFunc, SuspendInputToken]()
			{
				UCommonUIExtensions::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
				StateFunc(ERPGAsyncWidgetLayerState::Canceled, nullptr);
			})
		);

		return StreamingHandle;
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass, [](ActivatableWidgetT&) {});
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
		{
			return Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);
		}

		return nullptr;
	}

	// Find the widget if it exists on any of the layers and remove it from the layer.
	void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	// Get the layer widget for the given layer tag.
	UCommonActivatableWidgetContainerBase* GetLayerWidget(FGameplayTag LayerName);

protected:
	/** Register a layer that widgets can be pushed onto. */
	UFUNCTION(BlueprintCallable, Category="Layer")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);
	
	virtual void NativeOnInitialized() override;
	virtual void OnIsDormantChanged();

	void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetContainerBase> GameLayer_Stack;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetContainerBase> GameMenu_Stack;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetContainerBase> Menu_Stack;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonActivatableWidgetContainerBase> Modal_Stack;
	
private:
	bool bIsDormant = false;

	// Lets us keep track of all suspended input tokens so that multiple async UIs can be loading and we correctly suspend
	// for the duration of all of them.
	TArray<FName> SuspendInputTokens;

	// The registered layers for the primary layout.
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;
};
