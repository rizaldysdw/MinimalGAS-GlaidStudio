#include "GameplayAbilitySystem/GA_NormalAttack.h"

#include "AbilitySystemComponent.h"
#include "Characters/SimpleCharacter.h"

UGA_NormalAttack::UGA_NormalAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack")));
}

void UGA_NormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, Info, ActivationInfo))
    {
        EndAbility(Handle, Info, ActivationInfo, true, false);
        return;
    }

    ASimpleCharacter* Char = Info ? Cast<ASimpleCharacter>(Info->AvatarActor.Get()) : nullptr;
    UAbilitySystemComponent* SourceASC = Info ? Info->AbilitySystemComponent.Get() : nullptr;
    if (!Char || !SourceASC || !DamageEffect)
    {
        EndAbility(Handle, Info, ActivationInfo, true, false);
        return;
    }

    Char->LaunchNormalAttack(DamageEffect, SourceASC);

    EndAbility(Handle, Info, ActivationInfo, false, false);
}