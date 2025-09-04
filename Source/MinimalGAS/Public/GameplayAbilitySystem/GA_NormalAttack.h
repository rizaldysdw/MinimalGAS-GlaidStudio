// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_NormalAttack.generated.h"

UCLASS()
class MINIMALGAS_API UGA_NormalAttack : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UGA_NormalAttack();

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    TSubclassOf<class UGameplayEffect> DamageEffect;

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};