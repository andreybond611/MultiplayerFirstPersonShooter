// Copyright Andrei Bondarenko 2023


#include "GameplayAbilitySystem/Abilities/FPSGameplayAbility.h"
#include "GameplayAbilitySystem/FPSAbilitySystemComponent.h"

void UFPSGameplayAbility::ReplicatedEndAbility()
{
	constexpr bool bReplicateEndAbility = true;
	constexpr bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UFPSGameplayAbility::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately)
{
	UFPSAbilitySystemComponent* FPSASC = Cast<UFPSAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (FPSASC)
	{
		return FPSASC->BatchRPCTryActivateAbility(InAbilityHandle, EndAbilityImmediately);
	}

	return false;
}