#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SimpleCharacter.generated.h"

// Forward-declare
class USkeletalMeshComponent;
class UCameraComponent;
class USpringArmComponent;
class USphereComponent;
class UProjectileMovementComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class MINIMALGAS_API ASimpleCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASimpleCharacter();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* GunMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Muzzle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* Camera;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aiming")
    float AimInterpSpeed = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aiming")
    TEnumAsByte<ECollisionChannel> AimTraceChannel = ECC_Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting")
    TSubclassOf<class ASimpleProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting")
    float ProjectileSpeed = 6000.f;

public:
    UFUNCTION() void Input_NormalAttack();
    UFUNCTION() void Input_Fireball();

    void LaunchFireball(TSubclassOf<UGameplayEffect> InDamageGE = nullptr, UAbilitySystemComponent* InSourceASC = nullptr);

    void LaunchNormalAttack(TSubclassOf<UGameplayEffect> InDamageGE = nullptr, UAbilitySystemComponent* InSourceASC = nullptr);

protected:
    bool GetCursorHitPoint(FVector& OutHit) const;
    void AimArmsYawOnlyTowards(const FVector& WorldTarget, float DeltaSeconds);
    void SpawnProjectileTowards(const FVector& Target, class ASimpleProjectile*& OutProj, bool bExplosionCue, TSubclassOf<UGameplayEffect> InDamageGE, UAbilitySystemComponent* InSourceASC);
};