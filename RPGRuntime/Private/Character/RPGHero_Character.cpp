// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/RPGHero_Character.h"
#include "Character/RPGHeroComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Equipment/RPGEquipmentManagerComponent.h"
#include "Equipment/RPGWeaponInstance.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Inventory/RPGInventoryItemDefinition.h"
#include "Inventory/RPGInventoryManagerComponent.h"
#include "Inventory/RPGQuickbarComponent.h"
#include "Character/RPGPawnExtensionComponent.h"
#include "System/RPGGameplayTags.h"
#include "GameFramework/Controller.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGHero_Character)

ARPGHero_Character::ARPGHero_Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HeroComponent = CreateDefaultSubobject<URPGHeroComponent>(TEXT("HeroComponent"));
}

void ARPGHero_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Grant initial items on the server
	if (HasAuthority())
	{
		TryAddInitialInventory();
	}
}

void ARPGHero_Character::OnDeathStarted(AActor* OwningActor)
{
	Super::OnDeathStarted(OwningActor);

	// 1. Play random death montage
	if (DeathMontages.Num() > 0)
	{
		const int32 RandomIndex = FMath::RandRange(0, DeathMontages.Num() - 1);
		if (UAnimMontage* SelectedMontage = DeathMontages[RandomIndex])
		{
			PlayAnimMontage(SelectedMontage);
		}
	}

	// 2. Unregister from AI senses (specifically Sight)
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(GetWorld()))
	{
		PerceptionSystem->UnregisterSource(*this, UAISense_Sight::StaticClass());
	}

	// 3. Hide equipped weapons
	HideEquippedWeapons();

	// 4. Random delay before ragdoll
	const float RandomDelay = FMath::FRandRange(0.1f, 0.6f);
	GetWorldTimerManager().SetTimerForNextTick([this, RandomDelay]()
	{
		FTimerHandle RagdollTimerHandle;
		GetWorldTimerManager().SetTimer(RagdollTimerHandle, this, &ARPGHero_Character::StartRagdoll, RandomDelay, false);
	});
}

void ARPGHero_Character::StartRagdoll()
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetAllBodiesSimulatePhysics(true);
		MeshComp->SetSimulatePhysics(true);
		MeshComp->WakeAllRigidBodies();
	}
}

void ARPGHero_Character::HideEquippedWeapons()
{
	if (URPGEquipmentManagerComponent* EquipmentManager = FindComponentByClass<URPGEquipmentManagerComponent>())
	{
		EquipmentManager->SetAllWeaponsHidden(true);
	}
}

void ARPGHero_Character::AnimMotionEffect_Implementation(const FName Bone, const FGameplayTag MotionEffect, USceneComponent* StaticMeshComponent, const FVector LocationOffset, const FRotator RotationOffset, const UAnimSequenceBase* AnimationSequence, const bool bHitSuccess, const FHitResult HitResult, FGameplayTagContainer Contexts, FVector VFXScale, float AudioVolume, float AudioPitch)
{
	// Ensure we only run this on server or as appropriate for effects
	// The user specified: "Logic này chỉ chạy trên Server để đảm bảo tính đồng bộ hoặc quản lý Actor một cách tập trung."
	if (!HasAuthority())
	{
		return;
	}

	if (!FootStepActor && FootStepActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		FootStepActor = GetWorld()->SpawnActor<AActor>(FootStepActorClass, GetActorTransform(), SpawnParams);
		if (FootStepActor)
		{
			FootStepActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

	if (FootStepActor && FootStepActor->GetClass()->ImplementsInterface(URPGContextEffectsInterface::StaticClass()))
	{
		IRPGContextEffectsInterface::Execute_AnimMotionEffect(FootStepActor, Bone, MotionEffect, StaticMeshComponent, LocationOffset, RotationOffset, AnimationSequence, bHitSuccess, HitResult, Contexts, VFXScale, AudioVolume, AudioPitch);
	}
}

void ARPGHero_Character::TryAddInitialInventory()
{
	// Guard 1: Must be server and not initialized yet
	if (!HasAuthority() || bInventoryInitialized)
	{
		return;
	}

	AController* C = GetController();
	URPGPawnExtensionComponent* PawnExtComp = URPGPawnExtensionComponent::FindPawnExtensionComponent(this);
	
	// Guard 2: Pawn Extension must be ready (Data & ASC initialized)
	if (!PawnExtComp || !PawnExtComp->HasReachedInitState(RPGGameplayTags::InitState_GameplayReady))
	{
		GetWorldTimerManager().SetTimer(TimerHandle_InventoryReady, this, &ARPGHero_Character::TryAddInitialInventory, 0.1f, false);
		return;
	}

	// Guard 3: Controller components must be added by Experience
	if (C)
	{
		URPGInventoryManagerComponent* InventoryMgr = C->FindComponentByClass<URPGInventoryManagerComponent>();
		URPGQuickbarComponent* Quickbar = C->FindComponentByClass<URPGQuickbarComponent>();

		if (InventoryMgr && Quickbar)
		{
			AddInitialInventory(InventoryMgr, Quickbar);
			GetWorldTimerManager().ClearTimer(TimerHandle_InventoryReady);
			return;
		}
	}

	// If we get here, components aren't ready yet, retry
	GetWorldTimerManager().SetTimer(TimerHandle_InventoryReady, this, &ARPGHero_Character::TryAddInitialInventory, 0.1f, false);
}

void ARPGHero_Character::AddInitialInventory(URPGInventoryManagerComponent* InventoryMgr, URPGQuickbarComponent* Quickbar)
{
	check(InventoryMgr);
	check(Quickbar);

	if (bInventoryInitialized) return;

	int32 SlotIndex = 0;
	for (const TSubclassOf<URPGInventoryItemDefinition>& ItemDef : InitialInventoryItems)
	{
		if (ItemDef && SlotIndex < 10) // Limit to quickbar slots
		{
			URPGInventoryItemInstance* Instance = InventoryMgr->AddItemDefinition(ItemDef);
			if (Instance)
			{
				Quickbar->AddItemToSlot(SlotIndex, Instance);
				
				// Auto-select the first valid item
				if (SlotIndex == 0)
				{
					Quickbar->SetActiveSlotIndex(0);
				}
				
				SlotIndex++;
			}
		}
	}

	bInventoryInitialized = true;
}
