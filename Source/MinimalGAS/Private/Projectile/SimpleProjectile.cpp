#include "Projectile/SimpleProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/Engine.h"

ASimpleProjectile::ASimpleProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    Collision->InitSphereRadius(6.f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Block);
    Collision->SetNotifyRigidBodyCollision(true);
    Collision->SetGenerateOverlapEvents(false);
    Collision->bReturnMaterialOnMove = true;
    SetRootComponent(Collision);

    Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
    Movement->InitialSpeed = 3000.f;
    Movement->MaxSpeed = 3000.f;
    Movement->ProjectileGravityScale = 0.f;
    Movement->bRotationFollowsVelocity = true;

    InitialLifeSpan = 5.f;
    SetReplicates(true);
}

void ASimpleProjectile::BeginPlay()
{
    Super::BeginPlay();

    Collision->OnComponentHit.AddDynamic(this, &ASimpleProjectile::OnHit);

    // Ignore owner to prevent immediate self-hit
    if (AActor* OwnerActor = GetOwner())
    {
        Collision->IgnoreActorWhenMoving(OwnerActor, true);
    }
}

void ASimpleProjectile::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ASimpleProjectile::FireInDirection(const FVector& WorldDir, float Speed)
{
    Movement->Velocity = WorldDir.GetSafeNormal() * Speed;
}

void ASimpleProjectile::InitFromAbility(UAbilitySystemComponent* InSourceASC, TSubclassOf<UGameplayEffect> InDamageGE)
{
    // Safe to store now; Step 2 will use these on hit to apply effects/cues
    SourceASC = InSourceASC;
    DamageGE = InDamageGE;
}

void ASimpleProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Step 1: keep it simple—just destroy on server when we hit something.
    if (!HasAuthority())
    {
        Destroy();
        return;
    }

    // (Step 2 will: optionally execute explosion cue if bTriggerExplosionCue,
    //  and apply DamageGE to target if it has an ASC)

    Destroy();
}