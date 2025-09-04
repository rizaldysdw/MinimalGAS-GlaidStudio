#include "GameplayAbilitySystem/GA_Berserk.h"

#include "AbilitySystemComponent.h"

UGA_Berserk::UGA_Berserk()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Berserk")));
}

void UGA_Berserk::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid() && BuffEffect)
    {
        ActorInfo->AbilitySystemComponent->ApplyGameplayEffectToSelf(
            BuffEffect->GetDefaultObject<UGameplayEffect>(),
            1.f,
            ActorInfo->AbilitySystemComponent->MakeEffectContext());
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}