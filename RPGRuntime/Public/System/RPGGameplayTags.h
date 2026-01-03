// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * FRPGGameplayTags
 *
 * Singleton containing native Gameplay Tags
 */
struct FRPGGameplayTags
{
public:
	static const FRPGGameplayTags& Get() { return GameplayTags; }

	static void InitializeNativeTags();

	// Weapon Tags
	FGameplayTag Weapon_Type_Unarmed;
	FGameplayTag Weapon_Type_Greatsword;
	FGameplayTag Weapon_Type_Axe;

	// Inventory Tags
	FGameplayTag Inventory_Slot_Weapon;
	FGameplayTag Inventory_Slot_Armor;
	FGameplayTag Inventory_Item_Consumable;

	// Stat Tags
	FGameplayTag Stat_Health;
	FGameplayTag Stat_Mana;
	FGameplayTag Stat_Stamina;
	FGameplayTag Stat_Attack;
	FGameplayTag Stat_Defense;

	// Ability Tags
	FGameplayTag Ability_Action_Attack;
	FGameplayTag Ability_Action_Block;
	FGameplayTag Ability_Action_Dodge;

	// Message Tags
	FGameplayTag Message_Inventory_StackChanged;
	FGameplayTag Message_QuickBar_SlotsChanged;
	FGameplayTag Message_QuickBar_ActiveIndexChanged;
	FGameplayTag Message_Attribute_HealthChanged;
	FGameplayTag Message_Attribute_ManaChanged;
	FGameplayTag Message_Attribute_StaminaChanged;
	FGameplayTag Message_Respawn_Duration;
	FGameplayTag Message_Respawn_Completed;

	// Cosmetic Status Tags
	FGameplayTag Status_Weapon_Equipped;
	FGameplayTag Status_Death;
	FGameplayTag Status_Death_Dying;
	FGameplayTag Status_Death_Dead;

	// Movement Status Tags
	FGameplayTag Status_Movement_Moving;
	FGameplayTag Status_Movement_Idle;

	// Action Status Tags
	FGameplayTag Status_Action_Combo;

	// Gameplay Event Tags
	FGameplayTag GameplayEvent_Death;
	FGameplayTag GameplayEvent_Combo_Transition;
	FGameplayTag GameplayEvent_Combo_Damage;

	// Init State Tags
	FGameplayTag InitState_Spawned;
	FGameplayTag InitState_DataAvailable;
	FGameplayTag InitState_DataInitialized;
	FGameplayTag InitState_GameplayReady;

	// UI Layer Tags
	FGameplayTag UI_Layer_Game;
	FGameplayTag UI_Layer_GameMenu;
	FGameplayTag UI_Layer_Menu;
	FGameplayTag UI_Layer_Modal;

	// Input Tags
	FGameplayTag InputTag_Move;
	FGameplayTag InputTag_Look_Mouse;
	FGameplayTag InputTag_Look_Stick;
	FGameplayTag InputTag_Crouch;
	FGameplayTag InputTag_AutoRun;

private:
	static FRPGGameplayTags GameplayTags;
};

namespace RPGGameplayTags
{
	RPGRUNTIME_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	// Declare all of the custom native tags that RPG will use
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cost);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Networking);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Behavior_SurvivesDeath);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_AutoRun);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Reset);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_RequestReset);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Heal);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_GodMode);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_UnlimitedHealth);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Attribute_HealthChanged);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Attribute_ManaChanged);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Attribute_StaminaChanged);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Game);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_GameMenu);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Menu);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Modal);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Crouching);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_AutoRunning);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dying);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Movement_Moving);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Movement_Idle);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Action_Combo);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Combo_Transition);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Combo_Damage);

	// These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
	RPGRUNTIME_API extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
	RPGRUNTIME_API extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);

	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Status_Death);
	/** Ability.Respawn.Duration - Message with respawn duration. */
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Respawn_Duration);

	/** Ability.Respawn.Completed - Message when respawn is completed. */
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Message_Respawn_Completed);

	/** HUD.Slot.Reticle - HUD slot for the reticle/respawn timer. */
	RPGRUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HUD_Slot_Reticle);
}
