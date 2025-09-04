#include "GameplayAbilitySystem/GA_Fireball.h"

#include "AbilitySystemComponent.h"
#include "Characters/SimpleCharacter.h"
#include "GameplayAbilitySystem/SimpleAttributeSet.h"
#include "GameplayTagsManager.h"

UGA_Fireball::UGA_Fireball()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Fireball")));
}

void UGA_Fireball::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

    Char->LaunchFireball(DamageEffect, SourceASC);

    EndAbility(Handle, Info, ActivationInfo, false, false);
}

bool UGA_Fireball::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Info || !Info->AbilitySystemComponent.IsValid()) return false;

    // Check, need >= 20 Mana
    const float Mana = Info->AbilitySystemComponent->GetNumericAttribute(USimpleAttributeSet::GetManaAttribute());
    const bool bEnough = Mana >= 20.f;
    if (!bEnough && OptionalRelevantTags)
    {
        OptionalRelevantTags->AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Failed.NotEnoughMana")));
    }
    return bEnough && Super::CheckCost(Handle, Info, OptionalRelevantTags);
}

void UGA_Fireball::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info, const FGameplayAbilityActivationInfo ActivationInfo) const
{
    Super::ApplyCost(Handle, Info, ActivationInfo);
    
    if (Info && Info->AbilitySystemComponent.IsValid() && CostEffect)
    {
        Info->AbilitySystemComponent->ApplyGameplayEffectToSelf(
            CostEffect->GetDefaultObject<UGameplayEffect>(),
            1.f,
            Info->AbilitySystemComponent->MakeEffectContext());
    }
}