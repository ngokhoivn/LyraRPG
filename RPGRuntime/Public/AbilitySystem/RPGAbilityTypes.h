// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RPGAbilityTypes.generated.h"

/**
 * ERPGAbilityActivationGroup
 *
 *	Defines how an ability is activated in relation to other abilities.
 */
UENUM(BlueprintType)
enum class ERPGAbilityActivationGroup : uint8
{
	// Ability runs independently of all other abilities.
	Independent,

	// Ability is cancelled and replaced by other exclusive abilities.
	Exclusive_Replaceable,

	// Ability blocks other exclusive abilities from starting.
	Exclusive_Blocking,

	MAX UMETA(Hidden)
};

/**
 * ERPGAbilityActivationPolicy
 *
 *	Defines how an ability is meant to activate.
 */
UENUM(BlueprintType)
enum class ERPGAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is first pressed.
	OnInputTriggered,

	// Continually try to activate the ability while the input is held.
	WhileInputActive,

	// Try to activate the ability when an avatar is assigned.
	OnSpawn,
};
