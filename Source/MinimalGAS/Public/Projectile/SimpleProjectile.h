// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleProjectile.generated.h"

// Forward-declare
class USphereComponent;
class UProjectileMovementComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class MINIMALGAS_API ASimpleProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
    ASimpleProjectile();

    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* Collision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProjectileMovementComponent* Movement;

    // Configure whether this projectile should trigger the explosion GameplayCue on hit
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bTriggerExplosionCue = true;

    void FireInDirection(const FVector& WorldDir, float Speed);

    void InitFromAbility(UAbilitySystemComponent* InSourceASC, TSubclassOf<UGameplayEffect> InDamageGE);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
    UPROPERTY() 
    UAbilitySystemComponent* SourceASC = nullptr;
    
    UPROPERTY()
    TSubclassOf<UGameplayEffect> DamageGE;
};