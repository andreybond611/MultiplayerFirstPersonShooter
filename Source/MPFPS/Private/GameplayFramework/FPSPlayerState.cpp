// Copyright Andrei Bondarenko 2023

#include "GameplayFramework/FPSPlayerState.h"
#include "GameplayAbilitySystem/FPSAbilitySystemComponent.h"
#include "GameplayAbilitySystem/AttributeSets/PlayerAttributeSet.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPSPlayerState, All, All);

AFPSPlayerState::AFPSPlayerState()
{
	NetUpdateFrequency = 100.f;

	AbilitySystemComponent = CreateDefaultSubobject<UFPSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("AttributeSet"));

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetCurrentHealthAttribute())
		.AddUObject(this, &AFPSPlayerState::CurrentHealthChanged);

	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* AFPSPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AFPSPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AFPSPlayerState::CurrentHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (OnAttributeChangeData.NewValue == 0.f)
	{
		// todo: ded
	}
}
