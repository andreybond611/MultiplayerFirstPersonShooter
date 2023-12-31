// Copyright Andrei Bondarenko 2023

#pragma once

#include "CoreMinimal.h"
#include "FPSGameplayAbility.h"
#include "GA_FireOnce.generated.h"

class AGameplayAbilityTargetActor;
class UAbilityTask_ServerWaitForClientData;
class UAbilityTask_WaitTargetDataUsingActor;
/**
 *
 */
UCLASS()
class MPFPS_API UGA_FireOnce : public UFPSGameplayAbility
{
	GENERATED_BODY()
public:
	void FireShot();
	void SpawnTargetActor();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
						   FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual bool CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								   const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnValidDataAcquired(const FGameplayAbilityTargetDataHandle& Data);

	UPROPERTY()
	UAbilityTask_WaitTargetDataUsingActor* WaitTargetDataTask;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> AddMoneyEffect;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ThirdPersonFireMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* FirstPersonFireMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* FirstPersonFireMontageAiming;

	UPROPERTY(EditAnywhere, meta = (Categories = "GameplayCue"))
	FGameplayTag ShotGameplayCue = FGameplayTag::RequestGameplayTag("GameplayCue.Shooting.BulletImpact");

	UPROPERTY(EditAnywhere)
	float TimeBetweenShots = 0.1f;

	float LastShotTime = -99999;

	UPROPERTY()
	AGameplayAbilityTargetActor* TargetActor;
	UPROPERTY()
	UAbilityTask_ServerWaitForClientData* ServerWaitForClientDataTask;

	UPROPERTY(EditAnywhere)
	float TraceLength = 9999.f;

	TWeakObjectPtr<AActor> LastDeadTarget;

	void GiveRewardToInstigator(UAbilitySystemComponent* TargetAbilitySystemComponent);
};
