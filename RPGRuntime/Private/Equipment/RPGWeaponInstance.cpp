#include "Equipment/RPGWeaponInstance.h"
#include "Equipment/RPGEquipmentDefinition.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Character/RPGPawnExtensionComponent.h"
#include "System/RPGLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RPGWeaponInstance)

URPGWeaponInstance::URPGWeaponInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , Instigator(nullptr)
    , EquipmentDefinition(nullptr)
    , bIsEquipped(false)
{
}

UWorld* URPGWeaponInstance::GetWorld() const
{
    if (APawn* Pawn = GetPawn())
    {
        return Pawn->GetWorld();
    }
    return nullptr;
}

void URPGWeaponInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(URPGWeaponInstance, Instigator);
    DOREPLIFETIME(URPGWeaponInstance, SpawnedActors);
}

void URPGWeaponInstance::Initialize(UObject* InInstigator, URPGEquipmentDefinition* InDefinition)
{
    Instigator = InInstigator;
    EquipmentDefinition = InDefinition;
}

APawn* URPGWeaponInstance::GetPawn() const
{
    return Cast<APawn>(Instigator);
}

void URPGWeaponInstance::Equip()
{
    if (bIsEquipped) return;

    SpawnWeaponActors();
    ActivateAnimLayer(true);

    // Grant abilities
    if (APawn* Pawn = GetPawn())
    {
        // BUG FIX: ASC nằm trên PlayerState, không phải trên Pawn. 
        // Phải dùng PawnExtensionComponent hoặc casting Pawn sang IAbilitySystemInterface.
        URPGAbilitySystemComponent* RPGASC = nullptr;
        if (URPGPawnExtensionComponent* PawnExtComp = Pawn->FindComponentByClass<URPGPawnExtensionComponent>())
        {
            RPGASC = PawnExtComp->GetRPGAbilitySystemComponent();
        }

        if (RPGASC)
        {
            UE_LOG(LogRPG, Log, TEXT("RPGWeaponInstance::Equip: Found ASC on %s, granting %d ability sets."), *GetNameSafe(Pawn), EquipmentDefinition ? EquipmentDefinition->AbilitySetsToGrant.Num() : 0);
            if (EquipmentDefinition)
            {
                for (const URPGAbilitySet* AbilitySet : EquipmentDefinition->AbilitySetsToGrant)
                {
                    if (AbilitySet)
                    {
                        AbilitySet->GiveToAbilitySystem(RPGASC, &GrantedHandles, this);
                    }
                }
            }
        }
    }
    
    bIsEquipped = true;
    K2_OnEquipped();
}

void URPGWeaponInstance::Unequip()
{
    if (!bIsEquipped) return;

    // Remove granted abilities
    if (APawn* Pawn = GetPawn())
    {
        if (URPGAbilitySystemComponent* RPGASC = Pawn->FindComponentByClass<URPGAbilitySystemComponent>())
        {
            GrantedHandles.TakeFromAbilitySystem(RPGASC);
        }
    }

    ActivateAnimLayer(false);
    DestroyWeaponActors();
    
    bIsEquipped = false;
    K2_OnUnequipped();
}

void URPGWeaponInstance::ActivateAnimLayer(bool bEquip)
{
    TSubclassOf<UAnimInstance> AnimLayer = PickBestAnimLayer(bEquip, CosmeticTags);

    if (AnimLayer)
    {
        if (APawn* Pawn = GetPawn())
        {
            if (USkeletalMeshComponent* Mesh = Pawn->FindComponentByClass<USkeletalMeshComponent>())
            {
                if (bEquip)
                {
                    Mesh->LinkAnimClassLayers(AnimLayer);
                }
                else
                {
                    Mesh->UnlinkAnimClassLayers(AnimLayer);
                }
            }
        }
    }
}

void URPGWeaponInstance::SpawnWeaponActors()
{
    if (!GetWorld()) return;
    
    APawn* OwningPawn = GetPawn();
    if (!OwningPawn) return;

    USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
    if (ACharacter* Character = Cast<ACharacter>(OwningPawn))
    {
        AttachTarget = Character->GetMesh();
    }

    for (const FRPGWeaponActorToSpawn& SpawnInfo : ActorsToSpawn)
    {
        if (!SpawnInfo.ActorToSpawn) continue;

        AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(
            SpawnInfo.ActorToSpawn,
            FTransform::Identity,
            OwningPawn
        );
        
        NewActor->FinishSpawning(FTransform::Identity, true);
        if (SpawnInfo.AttachSocket != NAME_None)
            NewActor->AttachToComponent(
                AttachTarget,
                FAttachmentTransformRules::KeepRelativeTransform,
                SpawnInfo.AttachSocket
            );
        
        NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);

        SpawnedActors.Add(NewActor);
    }
}

void URPGWeaponInstance::DestroyWeaponActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();
}

void URPGWeaponInstance::SetActorsHidden(bool bHidden)
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->SetActorHiddenInGame(bHidden);
		}
	}
}

TSubclassOf<UAnimInstance> URPGWeaponInstance::PickBestAnimLayer_Implementation(bool bEquip, const FGameplayTagContainer& InCosmeticTags)
{
    return bEquip ? EquippedAnimLayer : UnequippedAnimLayer;
}