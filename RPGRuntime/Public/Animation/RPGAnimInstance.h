#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "RPGAnimInstance.generated.h"

// Enum Cardinal Direction (giống Lyra trong video)
UENUM(BlueprintType)
enum class ERPGCardinalDirection : uint8
{
    Forward UMETA(DisplayName = "Forward"),
    Backward UMETA(DisplayName = "Backward"),
    Left UMETA(DisplayName = "Left"),
    Right UMETA(DisplayName = "Right")
};

// Anim Struct Cardinal Directions (giống Lyra trong video)
USTRUCT(BlueprintType)
struct FRPGCardinalDirectionAnimations
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UAnimSequence> Forward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UAnimSequence> Backward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UAnimSequence> Left;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UAnimSequence> Right;

    FRPGCardinalDirectionAnimations()
        : Forward(nullptr)
        , Backward(nullptr)
        , Left(nullptr)
        , Right(nullptr)
    {
    }
};

UCLASS()
class RPGRUNTIME_API URPGAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	URPGAnimInstance(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    /** Cập nhật thông tin vận tốc từ Character (Thread Safe) */
    UFUNCTION(BlueprintCallable, Category = "Animation", meta = (BlueprintThreadSafe))
    void UpdateVelocityData(float DeltaSeconds);

    /** Chuyển đổi góc xoay sang hướng Cardinal (Thread Safe) */
    UFUNCTION(BlueprintCallable, Category = "Animation", meta = (BlueprintThreadSafe, BlueprintPure))
    ERPGCardinalDirection SelectCardinalDirectionFromAngle(float Angle) const;

    /** Lấy Animation Sequence tương ứng với hướng di chuyển (Thread Safe) */
    UFUNCTION(BlueprintCallable, Category = "Animation", meta = (BlueprintThreadSafe, BlueprintPure))
    class UAnimSequence* GetLocomotionAnimation(ERPGCardinalDirection Direction, bool bIsWalking) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation|Idle", meta = (BlueprintThreadSafe))
    UAnimSequence* GetIdleAnimation() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation|Idle", meta = (BlueprintThreadSafe))
    bool HasIdleAnimation() const;

protected:
	/** Khoảng cách tới mặt đất */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG|Animation")
	float GroundDistance;

    // Velocity Data
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    FVector WorldVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    FVector WorldVelocity2D;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    FVector LocalVelocity2D;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    bool bHasVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    float DisplacementSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    FVector PreviousLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    float LocalVelocityDirectionAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    float AnimationSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    float CharacterSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    float SpeedRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Velocity", Transient)
    ERPGCardinalDirection LocalVelocityDirection;

    // Configurable Animation Speeds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float JogAnimationSpeed = 375.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float WalkAnimationSpeed = 200.0f;

    // Rotation Data
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Rotation", Transient)
    FRotator WorldRotation;

    // Wall Detection & Stride Control (Lyra Style)
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Status", Transient)
    bool bIsRunningIntoWall;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Warping", Transient)
    float StrideWarpingCycleAlpha;

    // ===== ANIMATION DATA =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Idle")
    TObjectPtr<class UAnimSequence> IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Locomotion")
    FRPGCardinalDirectionAnimations JogCardinals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Locomotion")
    FRPGCardinalDirectionAnimations WalkCardinals;

    // Walk/Jog toggle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bUseWalkAnimations;

    // Weapon State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|Weapon")
    bool bIsWeaponEquipped;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|Weapon")
    FGameplayTagContainer CurrentWeaponTags;

private:
    float GetCurrentAnimationSpeed() const;

private:
    // References
    UPROPERTY(Transient)
    TObjectPtr<class APawn> OwningPawn;

    UPROPERTY(Transient)
    TObjectPtr<class ACharacter> OwningCharacter;
};

