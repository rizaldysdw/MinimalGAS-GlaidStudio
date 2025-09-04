#include "Tests/TargetActorTest.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameplayAbilitySystem/SimpleAttributeSet.h"

ATargetActorTest::ATargetActorTest()
{
	PrimaryActorTick.bCanEverTick = true;

    Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    Capsule->InitCapsuleSize(34.f, 88.f);
    Capsule->SetCollisionProfileName(TEXT("Pawn"));
    RootComponent = Capsule;

    Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Capsule);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
    ASC->SetIsReplicated(true);
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    Attributes = CreateDefaultSubobject<USimpleAttributeSet>(TEXT("Attributes"));

    SetReplicates(true);
}

void ATargetActorTest::BeginPlay()
{
	Super::BeginPlay();
	
    if (ASC)
    {
        ASC->InitAbilityActorInfo(this, this);
        ASC->GetGameplayAttributeValueChangeDelegate(USimpleAttributeSet::GetHealthAttribute()).AddUObject(this, &ATargetActorTest::OnHealthChanged);
    }
}

void ATargetActorTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATargetActorTest::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (!HasAuthority() || !Attributes) return;

    const float NewHealth = Data.NewValue;
    if (NewHealth <= 0.f && !bPendingRegen)
    {
        bPendingRegen = true;
        StartRegenCountdown();
    }
}

void ATargetActorTest::StartRegenCountdown()
{
    GetWorldTimerManager().SetTimer(RegenTimerHandle, this, &ATargetActorTest::DoFullRegen, RegenDelaySeconds, false);
}

void ATargetActorTest::DoFullRegen()
{
    if (!ASC || !Attributes) { bPendingRegen = false; return; }

    ApplyOverrideInstant(USimpleAttributeSet::GetHealthAttribute(), Attributes->GetMaxHealth());
    ApplyOverrideInstant(USimpleAttributeSet::GetManaAttribute(), Attributes->GetMaxMana());

    bPendingRegen = false;
}

void ATargetActorTest::ApplyOverrideInstant(const FGameplayAttribute& Attr, float NewValue)
{
    if (!ASC) return;

    UGameplayEffect* GE = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_Target_Regen_Instant")));
    GE->DurationPolicy = EGameplayEffectDurationType::Instant;

    FGameplayModifierInfo Mod;
    Mod.Attribute = Attr;
    Mod.ModifierOp = EGameplayModOp::Override;
    Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(NewValue));
    GE->Modifiers.Add(Mod);

    FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
    Ctx.AddInstigator(this, this);

    ASC->ApplyGameplayEffectToSelf(GE, 1.f, Ctx);
}