// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/RPGGameplayTags.h"
#include "GameplayTagsManager.h"

FRPGGameplayTags FRPGGameplayTags::GameplayTags;

void FRPGGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	// Weapon Types
	GameplayTags.Weapon_Type_Unarmed = Manager.AddNativeGameplayTag(TEXT("RPG.Weapon.Type.Unarmed"), TEXT("Unarmed weapon type"));
	GameplayTags.Weapon_Type_Greatsword = Manager.AddNativeGameplayTag(TEXT("RPG.Weapon.Type.Greatsword"), TEXT("Greatsword weapon type"));
	GameplayTags.Weapon_Type_Axe = Manager.AddNativeGameplayTag(TEXT("RPG.Weapon.Type.Axe"), TEXT("Axe weapon type"));

	// Inventory Slots
	GameplayTags.Inventory_Slot_Weapon = Manager.AddNativeGameplayTag(TEXT("RPG.Inventory.Slot.Weapon"), TEXT("Weapon inventory slot"));
	GameplayTags.Inventory_Slot_Armor = Manager.AddNativeGameplayTag(TEXT("RPG.Inventory.Slot.Armor"), TEXT("Armor inventory slot"));
	GameplayTags.Inventory_Item_Consumable = Manager.AddNativeGameplayTag(TEXT("RPG.Inventory.Item.Consumable"), TEXT("Consumable item type"));

	// Stats
	GameplayTags.Stat_Health = Manager.AddNativeGameplayTag(TEXT("RPG.Stat.Health"), TEXT("Health statistic"));
	GameplayTags.Stat_Mana = Manager.AddNativeGameplayTag(TEXT("RPG.Stat.Mana"), TEXT("Mana statistic"));
	GameplayTags.Stat_Stamina = Manager.AddNativeGameplayTag(TEXT("RPG.Stat.Stamina"), TEXT("Stamina statistic"));
	GameplayTags.Stat_Attack = Manager.AddNativeGameplayTag(TEXT("RPG.Stat.Attack"), TEXT("Attack power statistic"));
	GameplayTags.Stat_Defense = Manager.AddNativeGameplayTag(TEXT("RPG.Stat.Defense"), TEXT("Defense statistic"));

	// Abilities
	GameplayTags.Ability_Action_Attack = Manager.AddNativeGameplayTag(TEXT("RPG.Ability.Action.Attack"), TEXT("Attack action ability"));
	GameplayTags.Ability_Action_Block = Manager.AddNativeGameplayTag(TEXT("RPG.Ability.Action.Block"), TEXT("Block action ability"));
	GameplayTags.Ability_Action_Dodge = Manager.AddNativeGameplayTag(TEXT("RPG.Ability.Action.Dodge"), TEXT("Dodge action ability"));

	// Messages
	GameplayTags.Message_Inventory_StackChanged = Manager.AddNativeGameplayTag(TEXT("RPG.Inventory.Message.StackChanged"), TEXT("Inventory stack changed message"));
	GameplayTags.Message_QuickBar_SlotsChanged = Manager.AddNativeGameplayTag(TEXT("RPG.QuickBar.Message.SlotsChanged"), TEXT("QuickBar slots changed message"));
	GameplayTags.Message_QuickBar_ActiveIndexChanged = Manager.AddNativeGameplayTag(TEXT("RPG.QuickBar.Message.ActiveIndexChanged"), TEXT("QuickBar active index changed message"));
	GameplayTags.Message_Attribute_HealthChanged = RPGGameplayTags::Message_Attribute_HealthChanged;
	GameplayTags.Message_Attribute_ManaChanged = RPGGameplayTags::Message_Attribute_ManaChanged;
	GameplayTags.Message_Attribute_StaminaChanged = RPGGameplayTags::Message_Attribute_StaminaChanged;

	// Status
	GameplayTags.Status_Weapon_Equipped = Manager.AddNativeGameplayTag(TEXT("RPG.Status.Weapon.Equipped"), TEXT("Weapon is currently equipped"));
	GameplayTags.Status_Death = RPGGameplayTags::Status_Death;
	GameplayTags.Status_Death_Dying = RPGGameplayTags::Status_Death_Dying;
	GameplayTags.Status_Death_Dead = RPGGameplayTags::Status_Death_Dead;

	// Gameplay Events
	GameplayTags.GameplayEvent_Death = RPGGameplayTags::GameplayEvent_Death;

	// Map members to namespace tags for backward compatibility
	GameplayTags.InitState_Spawned = RPGGameplayTags::InitState_Spawned;
	GameplayTags.InitState_DataAvailable = RPGGameplayTags::InitState_DataAvailable;
	GameplayTags.InitState_DataInitialized = RPGGameplayTags::InitState_DataInitialized;
	GameplayTags.InitState_GameplayReady = RPGGameplayTags::InitState_GameplayReady;

	GameplayTags.InputTag_Move = RPGGameplayTags::InputTag_Move;
	GameplayTags.InputTag_Look_Mouse = RPGGameplayTags::InputTag_Look_Mouse;
	GameplayTags.InputTag_Look_Stick = RPGGameplayTags::InputTag_Look_Stick;
	GameplayTags.InputTag_Crouch = RPGGameplayTags::InputTag_Crouch;
	GameplayTags.InputTag_AutoRun = RPGGameplayTags::InputTag_AutoRun;

	// UI Layers
	GameplayTags.UI_Layer_Game = RPGGameplayTags::UI_Layer_Game;
	GameplayTags.UI_Layer_GameMenu = RPGGameplayTags::UI_Layer_GameMenu;
	GameplayTags.UI_Layer_Menu = RPGGameplayTags::UI_Layer_Menu;
	GameplayTags.UI_Layer_Modal = RPGGameplayTags::UI_Layer_Modal;
}

namespace RPGGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Networking, "Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "An ability with this type tag should not be canceled due to death.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Stick, "InputTag.Look.Stick", "Look (stick) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "InputTag.Crouch", "Crouch input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_AutoRun, "InputTag.AutoRun", "Auto-run input.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death, "GameplayEvent.Death", "Event that fires on death. This event only fires on the server.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Reset, "GameplayEvent.Reset", "Event that fires once a player reset is executed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_RequestReset, "GameplayEvent.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one at a valid spawn location.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Damage, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Heal, "SetByCaller.Heal", "SetByCaller tag used by healing gameplay effects.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_GodMode, "Cheat.GodMode", "GodMode cheat is active on the owner.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_UnlimitedHealth, "Cheat.UnlimitedHealth", "UnlimitedHealth cheat is active on the owner.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Attribute_HealthChanged, "Message.Attribute.HealthChanged", "Message UI for health changed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Attribute_ManaChanged, "Message.Attribute.ManaChanged", "Message UI for mana changed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Message_Attribute_StaminaChanged, "Message.Attribute.StaminaChanged", "Message UI for stamina changed");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Game, "UI.Layer.Game", "Game layer (HUD)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_GameMenu, "UI.Layer.GameMenu", "Game menu layer (Inventory)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Menu, "UI.Layer.Menu", "Main menu layer (Settings)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Layer_Modal, "UI.Layer.Modal", "Modal layer (Pop-ups)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Crouching, "Status.Crouching", "Target is crouching.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_AutoRunning, "Status.AutoRunning", "Target is auto-running.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead, "Status.Death.Dead", "Target has finished the death process.");
						  
	// These are mapped to the movement modes inside GetMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking, "Movement.Mode.Walking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_NavWalking, "Movement.Mode.NavWalking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling, "Movement.Mode.Falling", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Swimming, "Movement.Mode.Swimming", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Flying, "Movement.Mode.Flying", "Default Character movement tag");

	// When extending Lyra, you can create your own movement modes but you need to update GetCustomMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Custom, "Movement.Mode.Custom", "This is invalid and should be replaced with custom tags.  See LyraGameplayTags::CustomMovementModeTagMap.");

	// Unreal Movement Modes
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_NavWalking, Movement_Mode_NavWalking },
		{ MOVE_Falling, Movement_Mode_Falling },
		{ MOVE_Swimming, Movement_Mode_Swimming },
		{ MOVE_Flying, Movement_Mode_Flying },
		{ MOVE_Custom, Movement_Mode_Custom }
	};

	// Custom Movement Modes
	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
		// Fill these in with your custom modes
	};

	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}
