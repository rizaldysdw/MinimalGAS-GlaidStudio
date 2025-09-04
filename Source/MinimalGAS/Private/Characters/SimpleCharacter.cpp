#include "Characters/SimpleCharacter.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameplayAbilitySystem/SimpleAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
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
    Muzzle->SetupAttachment(GunMesh, TEXT("Muzzle"));               // if no socket, attach to GunMesh

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

    // GAS
    ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
    ASC->SetIsReplicated(true);
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    Attributes = CreateDefaultSubobject<USimpleAttributeSet>(TEXT("Attributes"));
}

void ASimpleCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (ASC)
    {
        ASC->InitAbilityActorInfo(this, this);
    }

    if (HasAuthority() && ASC)
    {
        if (NormalAttackAbility) ASC->GiveAbility(FGameplayAbilitySpec(NormalAttackAbility, 1, 0, this));
        if (FireballAbility) ASC->GiveAbility(FGameplayAbilitySpec(FireballAbility, 1, 0, this));
        if (BerserkAbility) ASC->GiveAbility(FGameplayAbilitySpec(BerserkAbility, 1, 0, this));
    }
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
    PlayerInputComponent->BindAction("Berserk", IE_Pressed, this, &ASimpleCharacter::Input_Berserk);
    PlayerInputComponent->BindAction("Restore", IE_Pressed, this, &ASimpleCharacter::Input_Restore);
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

void ASimpleCharacter::SpawnProjectileTowards(const FVector& Target, ASimpleProjectile*& OutProj, bool bExplosionCue, TSubclassOf<UGameplayEffect> InDamageGE, UAbilitySystemComponent* InSourceASC)
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
        Proj->bTriggerExplosionCue = bExplosionCue;
        Proj->InitFromAbility(InSourceASC, InDamageGE);
        Proj->FireInDirection(Dir, ProjectileSpeed);
    }

    OutProj = Proj;
}

void ASimpleCharacter::LaunchNormalAttack(TSubclassOf<UGameplayEffect> InDamageGE, UAbilitySystemComponent* InSourceASC)
{
    FVector Target;
    if (!GetCursorHitPoint(Target)) return;

    ASimpleProjectile* Spawned = nullptr;
    SpawnProjectileTowards(Target, Spawned, false, InDamageGE, InSourceASC);
}

void ASimpleCharacter::LaunchFireball(TSubclassOf<UGameplayEffect> InDamageGE, UAbilitySystemComponent* InSourceASC)
{
    FVector Target;
    if (!GetCursorHitPoint(Target)) return;

    ASimpleProjectile* Spawned = nullptr;
    SpawnProjectileTowards(Target, Spawned, true, InDamageGE, InSourceASC);
}

void ASimpleCharacter::Input_NormalAttack()
{
    if (ASC && NormalAttackAbility)
    {
        ASC->TryActivateAbilityByClass(NormalAttackAbility);
    }
}

void ASimpleCharacter::Input_Fireball()
{
    if (ASC && FireballAbility)
    {
        ASC->TryActivateAbilityByClass(FireballAbility);
    }
}

void ASimpleCharacter::Input_Berserk()
{
    if (ASC && BerserkAbility)
    {
        ASC->TryActivateAbilityByClass(BerserkAbility);
    }
}

void ASimpleCharacter::Input_Restore()
{
    if (!ASC || !Attributes) return;

    // Create a transient GameplayEffect
    UGameplayEffect* RestoreEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName("GE_Restore"));

    RestoreEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

    // Health to MaxHealth
    {
        FGameplayModifierInfo ModInfo;
        ModInfo.Attribute = USimpleAttributeSet::GetHealthAttribute();
        ModInfo.ModifierOp = EGameplayModOp::Override;
        ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Attributes->GetMaxHealth()));
        RestoreEffect->Modifiers.Add(ModInfo);
    }

    // Mana to MaxMana
    {
        FGameplayModifierInfo ModInfo;
        ModInfo.Attribute = USimpleAttributeSet::GetManaAttribute();
        ModInfo.ModifierOp = EGameplayModOp::Override;
        ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Attributes->GetMaxMana()));
        RestoreEffect->Modifiers.Add(ModInfo);
    }

    ASC->ApplyGameplayEffectToSelf(RestoreEffect, 1.f, ASC->MakeEffectContext());
}