#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "TargetActorTest.generated.h"

// Forward declare
class UCapsuleComponent;
class USkeletalMeshComponent;
class UAbilitySystemComponent;
class USimpleAttributeSet;

UCLASS()
class MINIMALGAS_API ATargetActorTest : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATargetActorTest();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCapsuleComponent* Capsule;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    UAbilitySystemComponent* ASC;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    USimpleAttributeSet* Attributes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Regen")
    float RegenDelaySeconds = 3.0f;

public:	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASC; }

private:
    // Health change delegate
    void OnHealthChanged(const struct FOnAttributeChangeData& Data);

    // Start a one-shot timer to restore
    void StartRegenCountdown();

    // Actually restore attributes to full
    void DoFullRegen();

    // Helper to apply an instant override GE (sets Attr = NewValue)
    void ApplyOverrideInstant(const struct FGameplayAttribute& Attr, float NewValue);

    // State
    FTimerHandle RegenTimerHandle;
    bool bPendingRegen = false;
};