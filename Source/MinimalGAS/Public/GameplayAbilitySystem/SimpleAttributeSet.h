#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "SimpleAttributeSet.generated.h"

#define ATTR_ACCESSORS(ClassName, PropertyName) \
  GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
  GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
  GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
  GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class MINIMALGAS_API USimpleAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
    USimpleAttributeSet();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;
    ATTR_ACCESSORS(USimpleAttributeSet, Health)
        UFUNCTION() void OnRep_Health(const FGameplayAttributeData& OldValue)
    {
        GAMEPLAYATTRIBUTE_REPNOTIFY(USimpleAttributeSet, Health, OldValue);
    }

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
    FGameplayAttributeData MaxHealth;
    ATTR_ACCESSORS(USimpleAttributeSet, MaxHealth)
        UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
    {
        GAMEPLAYATTRIBUTE_REPNOTIFY(USimpleAttributeSet, MaxHealth, OldValue);
    }

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Mana)
    FGameplayAttributeData Mana;
    ATTR_ACCESSORS(USimpleAttributeSet, Mana)
        UFUNCTION() void OnRep_Mana(const FGameplayAttributeData& OldValue)
    {
        GAMEPLAYATTRIBUTE_REPNOTIFY(USimpleAttributeSet, Mana, OldValue);
    }

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxMana)
    FGameplayAttributeData MaxMana;
    ATTR_ACCESSORS(USimpleAttributeSet, MaxMana)
        UFUNCTION() void OnRep_MaxMana(const FGameplayAttributeData& OldValue)
    {
        GAMEPLAYATTRIBUTE_REPNOTIFY(USimpleAttributeSet, MaxMana, OldValue);
    }

    // AttackPower (used by Normal Attack)
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_AttackPower)
    FGameplayAttributeData AttackPower;
    ATTR_ACCESSORS(USimpleAttributeSet, AttackPower)
        UFUNCTION() void OnRep_AttackPower(const FGameplayAttributeData& OldValue)
    {
        GAMEPLAYATTRIBUTE_REPNOTIFY(USimpleAttributeSet, AttackPower, OldValue);
    }

protected:
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};