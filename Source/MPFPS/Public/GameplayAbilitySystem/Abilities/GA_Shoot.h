// Copyright Andrei Bondarenko 2023

#pragma once

#include "CoreMinimal.h"
#include "FPSGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GA_Shoot.generated.h"

/**
 *
 */
UCLASS()
class MPFPS_API UGA_Shoot : public UFPSGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
						   FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							  const FGameplayAbilityActivationInfo ActivationInfo) override;

private:
	UPROPERTY(EditAnywhere, meta = (GameplayTagFilter = "Weapon.FireMode"))
	FGameplayTag SemiAutoTag = FGameplayTag::RequestGameplayTag("Weapon.FireMode.SemiAuto");
	UPROPERTY(EditAnywhere, meta = (GameplayTagFilter = "Weapon.FireMode"))
	FGameplayTag FullAutoTag = FGameplayTag::RequestGameplayTag("Weapon.FireMode.FullAuto");

	UPROPERTY(EditAnywhere)
	float ShotCooldown = 0.1f;

	UPROPERTY()
	class UGA_FireOnce* SingleShotAbilityInstance;
	UPROPERTY()
	class UAbilityTask_WaitDelay* WaitDelayTask;
	UPROPERTY()
	UAbilityTask_WaitInputRelease* WaitInputReleaseTask;

	float LastShotTime = -9999.f;

	UFUNCTION()
	void OnInputRelease(float TimeHeld);
	UFUNCTION()
	void OnShotCooldownExpired();

	void EndShootAbilities();

	FGameplayAbilitySpecHandle SingleShotAbilitySpecHandle;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGA_FireOnce> SingleShotAbilityClass;
};
