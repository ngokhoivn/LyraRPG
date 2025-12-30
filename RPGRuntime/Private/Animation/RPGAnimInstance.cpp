#include "Animation/RPGAnimInstance.h"
#include "Equipment/RPGEquipmentManagerComponent.h"
#include "Equipment/RPGWeaponInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Animation/AnimSequence.h"
#include "Math/UnrealMathUtility.h"

// Sửa constructor - tên phải khớp với class
URPGAnimInstance::URPGAnimInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , WorldVelocity(FVector::ZeroVector)
    , WorldVelocity2D(FVector::ZeroVector)
    , LocalVelocity2D(FVector::ZeroVector)
    , bHasVelocity(false)
    , DisplacementSpeed(0.0f)
    , PreviousLocation(FVector::ZeroVector)
    , LocalVelocityDirectionAngle(0.0f)
    , AnimationSpeed(0.0f)
    , CharacterSpeed(0.0f)
    , SpeedRatio(1.0f)
    , LocalVelocityDirection(ERPGCardinalDirection::Forward)
    , WorldRotation(FRotator::ZeroRotator)
    , bUseWalkAnimations(false)
    , bIsRunningIntoWall(false)
    , StrideWarpingCycleAlpha(1.0f)
    , OwningPawn(nullptr)
    , OwningCharacter(nullptr)
{
}

bool URPGAnimInstance::HasIdleAnimation() const
{
    return IdleAnimation != nullptr;
}

void URPGAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwningPawn = TryGetPawnOwner();
    if (OwningPawn)
    {
        OwningCharacter = Cast<ACharacter>(OwningPawn);
    }

    if (OwningPawn)
    {
        PreviousLocation = OwningPawn->GetActorLocation();
    }
}

void URPGAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    bIsWeaponEquipped = false;
    CurrentWeaponTags.Reset();

    if (OwningCharacter)
    {
        if (URPGEquipmentManagerComponent* EquipManager = OwningCharacter->FindComponentByClass<URPGEquipmentManagerComponent>())
        {
            // Tìm vũ khí đầu tiên đang trang bị (Lyra style thường có 1 weapon instance chính)
            if (URPGWeaponInstance* CurrentWeapon = EquipManager->GetFirstInstanceOfType(URPGWeaponInstance::StaticClass()))
            {
                if (CurrentWeapon->IsEquipped())
                {
                    bIsWeaponEquipped = true;
                    CurrentWeaponTags = CurrentWeapon->CosmeticTags;
                }
            }
        }
    }
}

void URPGAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

    if (!OwningPawn)
    {
        OwningPawn = TryGetPawnOwner();
        OwningCharacter = Cast<ACharacter>(OwningPawn);
    }

    if (OwningPawn && DeltaSeconds > 0.0f)
    {
        UpdateVelocityData(DeltaSeconds);
    }
}

void URPGAnimInstance::UpdateVelocityData(float DeltaSeconds)
{
    if (!OwningPawn)
    {
        OwningPawn = TryGetPawnOwner();
        if (!OwningPawn) return;

        OwningCharacter = Cast<ACharacter>(OwningPawn);
    }

    if (!OwningPawn) return;

    // 1. Tính Character Speed (cm/s - UE units)
    if (OwningPawn.Get() && DeltaSeconds > SMALL_NUMBER)
    {
        FVector CurrentLocation = OwningPawn->GetActorLocation();

        // Tính displacement trong cm (UE standard units)
        FVector HorizontalDisplacement = CurrentLocation - PreviousLocation;
        HorizontalDisplacement.Z = 0.0f;

        float DisplacementSinceLastUpdate = HorizontalDisplacement.Size(); // cm

        CharacterSpeed = DisplacementSinceLastUpdate / DeltaSeconds; // cm/s
        PreviousLocation = CurrentLocation;
    }
    else
    {
        CharacterSpeed = 0.0f;
    }

    // 2. Lấy Animation Speed (cm/s)
    AnimationSpeed = GetCurrentAnimationSpeed();

    // Đảm bảo AnimationSpeed không quá nhỏ để tránh chia 0
    AnimationSpeed = FMath::Max(AnimationSpeed, 50.0f);

    // 3. Tính Speed Ratio (quan trọng cho Stride Warping)
    if (AnimationSpeed > SMALL_NUMBER)
    {
        SpeedRatio = CharacterSpeed / AnimationSpeed;

        // Clamp nghiêm ngặt để tránh chân dính
        SpeedRatio = FMath::Clamp(SpeedRatio, 0.7f, 1.3f);
    }
    else
    {
        SpeedRatio = 1.0f;
    }

    // 4. DisplacementSpeed (dùng cho Play Rate trong Blueprint)
    DisplacementSpeed = CharacterSpeed;

    // Thêm logic kiểm tra chạy vào tường
    if (OwningCharacter)
    {
        // 1. Lấy độ lớn gia tốc (tín hiệu di chuyển từ người chơi)
        float AccelerationSize = OwningCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size();

        // 2. Kiểm tra: Có nhấn phím di chuyển NHƯNG tốc độ thực tế lại gần bằng 0
        // Và phải là đang ở trạng thái di chuyển trên đất (Walking)
        const float WallThreshold = 0.1f;
        bIsRunningIntoWall = (AccelerationSize > 0.0f) && (CharacterSpeed < WallThreshold);
    }

    // 3. Tính toán StrideWarpingCycleAlpha (thay vì làm trong Blueprint)
    float TargetAlpha = bIsRunningIntoWall ? 0.5f : 1.0f;

    // Sử dụng FMath::FInterpTo để làm mượt chuyển đổi
    StrideWarpingCycleAlpha = FMath::FInterpTo(StrideWarpingCycleAlpha, TargetAlpha, DeltaSeconds, 10.0f);


    // 2. Get World Rotation (giống video 00:02:59)
    WorldRotation = OwningPawn->GetActorRotation();

    // 2. Get World Velocity (giống video 00:03:42)
    WorldVelocity = OwningPawn->GetVelocity();

    // 3. Calculate World Velocity 2D (giống video 00:04:02)
    // Multiply with vector where Z value is zero (như video nói)
    WorldVelocity2D = FVector(WorldVelocity.X, WorldVelocity.Y, 0.0f);

    // 4. Calculate Local Velocity 2D (giống video 00:04:54)
    // Unrotate the world velocity to get local space velocity
    LocalVelocity2D = WorldRotation.UnrotateVector(WorldVelocity2D);

    // 5. Check Has Velocity (giống video 00:05:29)
    // Use VectorLengthXYSquared và nearly equal như video
    float VelocitySquaredXY = FVector2D(LocalVelocity2D.X, LocalVelocity2D.Y).SizeSquared();

    // Sử dụng tolerance nhỏ như video (00:05:48)
    const float VelocityThreshold = 0.1f;
    bHasVelocity = !FMath::IsNearlyZero(VelocitySquaredXY, VelocityThreshold);

    // 6. Calculate Local Velocity Direction Angle (giống video 00:06:14)
    if (bHasVelocity)
    {
        LocalVelocityDirectionAngle = UKismetAnimationLibrary::CalculateDirection(
            WorldVelocity,
            WorldRotation
        );
    }
    else
    {
        LocalVelocityDirectionAngle = 0.0f;
    }

    // 7. Calculate Local Velocity Direction (giống video 00:11:00)
    LocalVelocityDirection = SelectCardinalDirectionFromAngle(LocalVelocityDirectionAngle);
}

ERPGCardinalDirection URPGAnimInstance::SelectCardinalDirectionFromAngle(float Angle) const
{
    // Chuyển angle về range 0-360
    float NormalizedAngle = FMath::Fmod(Angle + 360.0f, 360.0f);

    // Logic chia 4 hướng như trong video
    if (NormalizedAngle >= 315.0f || NormalizedAngle < 45.0f)
        return ERPGCardinalDirection::Forward;
    else if (NormalizedAngle >= 45.0f && NormalizedAngle < 135.0f)
        return ERPGCardinalDirection::Right;
    else if (NormalizedAngle >= 135.0f && NormalizedAngle < 225.0f)
        return ERPGCardinalDirection::Backward;
    else // 225-315
        return ERPGCardinalDirection::Left;
}

UAnimSequence* URPGAnimInstance::GetLocomotionAnimation(ERPGCardinalDirection Direction, bool bIsWalking) const
{
    // Lấy animation sequence dựa trên direction và walk/jog state
    // Giống logic trong video (00:16:32 - 00:17:45)

    const FRPGCardinalDirectionAnimations& Animations = bIsWalking ? WalkCardinals : JogCardinals;

    UAnimSequence* SelectedAnim = nullptr;

    switch (Direction)
    {
    case ERPGCardinalDirection::Forward:
        SelectedAnim = Animations.Forward.Get();
        break;
    case ERPGCardinalDirection::Backward:
        SelectedAnim = Animations.Backward.Get();
        break;
    case ERPGCardinalDirection::Left:
        SelectedAnim = Animations.Left.Get();
        break;
    case ERPGCardinalDirection::Right:
        SelectedAnim = Animations.Right.Get();
        break;
    default:
        SelectedAnim = Animations.Forward.Get();
        break;
    }

    // Fallback if null
    if (!SelectedAnim)
    {
        SelectedAnim = Animations.Forward.Get();
    }

    return SelectedAnim;
}

float URPGAnimInstance::GetCurrentAnimationSpeed() const
{
    // Trả về tốc độ animation hiện tại dựa trên state
    // Trong tương lai có thể extract trực tiếp từ AnimSequence metadata
    return bUseWalkAnimations ? WalkAnimationSpeed : JogAnimationSpeed;
}

UAnimSequence* URPGAnimInstance::GetIdleAnimation() const
{
    return IdleAnimation ? IdleAnimation.Get() : nullptr;
}
