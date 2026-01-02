// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "RPGActivatableWidget.generated.h"

struct FUIInputConfig;

UENUM(BlueprintType)
enum class ERPGWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};

/**
 * URPGActivatableWidget
 *
 * Base class for activatable widgets in the RPG plugin (HUD, Menus, etc).
 * Standalone version of LyraActivatableWidget, inheriting directly from CommonActivatableWidget.
 */
UCLASS(Abstract, Blueprintable)
class RPGRUNTIME_API URPGActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	URPGActivatableWidget(const FObjectInitializer& ObjectInitializer);
	
public:
	//~UCommonActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UCommonActivatableWidget interface

#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif
	
protected:
	/** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Input")
	ERPGWidgetInputMode InputConfig = ERPGWidgetInputMode::Default;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Input")
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};
