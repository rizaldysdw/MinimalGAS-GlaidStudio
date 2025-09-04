#include "Characters/SimpleCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

#include "Projectile/SimpleProjectile.h"

ASimpleCharacter::ASimpleCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Gun mesh attached to the character mesh (arms/torso)
    GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
    GunMesh->SetupAttachment(GetMesh(), TEXT("GripPoint"));
    GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GunMesh->SetGenerateOverlapEvents(false);

    // Muzzle scene at the gun
    Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
    Muzzle->SetupAttachment(GunMesh, TEXT("Muzzle"));               // if no socket, just attach to GunMesh

    // Camera boom
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetCapsuleComponent());
    SpringArm->TargetArmLength = 300.f;
    SpringArm->SetRelativeLocation(FVector(100.f, 100.f, 100.f));
    SpringArm->SetRelativeRotation(FRotator(0.f, -20.f, -60.f));
    SpringArm->bUsePawnControlRotation = false;

    // Camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;
}

void ASimpleCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ASimpleCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    FVector HitPoint;
    if (GetCursorHitPoint(HitPoint))
    {
        AimArmsYawOnlyTowards(HitPoint, DeltaSeconds);
    }
}

void ASimpleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);

    PlayerInputComponent->BindAction("Attack",   IE_Pressed, this, &ASimpleCharacter::Input_NormalAttack);
    PlayerInputComponent->BindAction("Fireball", IE_Pressed, this, &ASimpleCharacter::Input_Fireball);
}

bool ASimpleCharacter::GetCursorHitPoint(FVector& OutHit) const
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        FHitResult HR;
        if (PC->GetHitResultUnderCursor(AimTraceChannel, /*bTraceComplex*/ false, HR))
        {
            OutHit = HR.ImpactPoint;
            return true;
        }

        // Fallback deproject ray
        float X = 0.f, Y = 0.f;
        if (PC->GetMousePosition(X, Y))
        {
            FVector WL, WD;
            if (PC->DeprojectScreenPositionToWorld(X, Y, WL, WD))
            {
                const FVector Start = WL;
                const FVector End = WL + WD * 100000.f;

                FHitResult RayHit;
                FCollisionQueryParams Params(SCENE_QUERY_STAT(MouseRay), false);
                Params.AddIgnoredActor(this);

                if (GetWorld()->LineTraceSingleByChannel(RayHit, Start, End, AimTraceChannel, Params))
                {
                    OutHit = RayHit.ImpactPoint;
                    return true;
                }

                OutHit = End; // nothing hit: aim far along ray
                return true;
            }
        }
    }
    return false;
}

void ASimpleCharacter::AimArmsYawOnlyTowards(const FVector& WorldTarget, float DeltaSeconds)
{
    if (!GunMesh) return;

    const FVector Origin = Muzzle ? Muzzle->GetComponentLocation()
        : GunMesh->GetComponentLocation();

    FVector Flat = WorldTarget - Origin;
    Flat.Z = 0.f;
    if (Flat.IsNearlyZero()) return;

    const FRotator DesiredYaw = Flat.GetSafeNormal().Rotation();
    const FRotator Current = GetMesh()->GetComponentRotation();

    FRotator TargetRot = Current;
    TargetRot.Yaw = DesiredYaw.Yaw;
    TargetRot.Pitch = 0.f;
    TargetRot.Roll = 0.f;

    const FRotator Smoothed = FMath::RInterpTo(Current, TargetRot, DeltaSeconds, AimInterpSpeed);
    GetMesh()->SetWorldRotation(Smoothed);
}

// Shared spawn helper
void ASimpleCharacter::SpawnProjectileTowards(const FVector& Target,
    ASimpleProjectile*& OutProj,
    bool bExplosionCue,
    TSubclassOf<UGameplayEffect> InDamageGE,
    UAbilitySystemComponent* InSourceASC)
{
    OutProj = nullptr;
    if (!ProjectileClass || !Muzzle) return;

    const FVector MuzzleLoc = Muzzle->GetComponentLocation();
    FVector Dir = (Target - MuzzleLoc);
    if (!Dir.Normalize()) return;

    FRotator SpawnRot = Dir.Rotation();
    SpawnRot.Roll = 0.f;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    Params.Owner = this;

    ASimpleProjectile* Proj = GetWorld()->SpawnActor<ASimpleProjectile>(ProjectileClass, MuzzleLoc, SpawnRot, Params);
    if (Proj)
    {
        // Configure explosion behaviour per ability
        Proj->bTriggerExplosionCue = bExplosionCue;

        // Step 1: no GAS yet; this is safe. In Step 2 we’ll use this to pass ASC/GE.
        Proj->InitFromAbility(InSourceASC, InDamageGE);

        Proj->FireInDirection(Dir, ProjectileSpeed);
    }

    OutProj = Proj;
}

// Fireball: projectile WITH explosion cue
void ASimpleCharacter::LaunchFireball(TSubclassOf<UGameplayEffect> InDamageGE, UAbilitySystemComponent* InSourceASC)
{
    FVector Target;
    if (!GetCursorHitPoint(Target)) return;

    ASimpleProjectile* Spawned = nullptr;
    SpawnProjectileTowards(Target, Spawned, /*bExplosionCue*/ true, InDamageGE, InSourceASC);
}

// Normal Attack: projectile WITHOUT explosion cue
void ASimpleCharacter::LaunchNormalAttack(TSubclassOf<UGameplayEffect> InDamageGE, UAbilitySystemComponent* InSourceASC)
{
    FVector Target;
    if (!GetCursorHitPoint(Target)) return;

    ASimpleProjectile* Spawned = nullptr;
    SpawnProjectileTowards(Target, Spawned, /*bExplosionCue*/ false, InDamageGE, InSourceASC);
}

// Optional input wrappers (hook to Input when ready)
void ASimpleCharacter::Input_Fireball()
{
    LaunchFireball(nullptr, nullptr); // Step 2 will pass DamageGE + ASC
}
void ASimpleCharacter::Input_NormalAttack()
{
    LaunchNormalAttack(nullptr, nullptr); // Step 2 will pass DamageGE + ASC
}