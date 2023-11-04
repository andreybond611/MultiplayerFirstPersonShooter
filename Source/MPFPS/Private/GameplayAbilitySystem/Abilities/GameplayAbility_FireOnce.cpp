// Copyright Andrei Bondarenko 2023

#include "GameplayAbilitySystem/Abilities/GameplayAbility_FireOnce.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Camera/CameraComponent.h"
#include "Characters/PlayerCharacter.h"
#include "GameplayAbilitySystem/FPSAbilitySystemComponent.h"
#include "GameplayAbilitySystem/AbilityTasks/AbilityTask_ServerWaitForClientData.h"
#include "GameplayAbilitySystem/AbilityTasks/AbilityTask_WaitTargetDataUsingActor.h"
#include "GameplayAbilitySystem/TargetActors/TargetActor_LineTrace.h"
#include "Types/CollisionTypes.h"
#include "Weapons/EquipmentComponent.h"

void UGameplayAbility_FireOnce::FireShot()
{
	if (GetActorInfo().IsLocallyControlledPlayer())
	{
		const float ShotTime = GetWorld()->GetTimeSeconds();
		const float ShotTimeDifference = ShotTime - LastShotTime;

		if (ShotTimeDifference > TimeBetweenShots)
		{
			if (!TargetActor)
			{
				SpawnTargetActor();
			}

			auto LineTraceActor = Cast<ATargetActor_LineTrace>(TargetActor);
			if (LineTraceActor)
			{
				auto Character = Cast<APlayerCharacter>(GetSourceObject(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo()));
				check(Character);

				const FVector StartTrace = Character->GetFirstPersonCamera()->GetComponentLocation();
				const float Spread = Character->GetEquipmentComponent()->GetSpread();
				const float HalfConeAngleDeg = FMath::Lerp(0.01, 20.f, Spread);
				const FVector DeviationVector = FMath::VRandCone(Character->GetBaseAimRotation().Vector(), FMath::DegreesToRadians(HalfConeAngleDeg));
				const FVector EndTrace = StartTrace + DeviationVector * TraceLength;

				LineTraceActor->Configure(StartTrace, EndTrace, BULLET_TRACE_COLLISION);
			}

			auto PlayerCharacter = Cast<APlayerCharacter>(GetActorInfo().AvatarActor);
			if (PlayerCharacter)
			{
				auto EquipmentComponent = PlayerCharacter->GetEquipmentComponent();
				if (EquipmentComponent)
				{
					EquipmentComponent->AddSpread(0.1f); // todo: magic number. maybe we can even not to pass a parameter
				}
			}

			WaitTargetDataTask = UAbilityTask_WaitTargetDataUsingActor::WaitTargetDataWithReusableActor(
				this, NAME_None, EGameplayTargetingConfirmation::Instant, TargetActor, true);

			WaitTargetDataTask->ValidData.AddDynamic(this, &UGameplayAbility_FireOnce::OnValidDataAcquired);
			WaitTargetDataTask->Activate();
		}
	}
}

void UGameplayAbility_FireOnce::SpawnTargetActor()
{
	if (!TargetActor)
	{
		AGameplayAbilityTargetActor* SpawnedActor = nullptr;

		UAbilityTask_WaitTargetData* SpawnTargetActorTask =
			UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::Instant, ATargetActor_LineTrace::StaticClass());
		
		SpawnTargetActorTask->BeginSpawningActor(this, ATargetActor_LineTrace::StaticClass(), SpawnedActor);
		SpawnTargetActorTask->FinishSpawningActor(this, SpawnedActor);

		TargetActor = SpawnedActor;
	}
}

void UGameplayAbility_FireOnce::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
												const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("FireOnce ActivateAbility"))

	SpawnTargetActor();

	ServerWaitForClientDataTask = UAbilityTask_ServerWaitForClientData::ServerWaitForClientTargetData(this, NAME_None, false);
	ServerWaitForClientDataTask->ValidData.AddDynamic(this, &UGameplayAbility_FireOnce::OnValidDataAcquired);
	ServerWaitForClientDataTask->Activate();

	FireShot();
}

bool UGameplayAbility_FireOnce::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
										  FGameplayTagContainer* OptionalRelevantTags) const
{
	if (auto EquipmentComponent = GetCurrentActorInfo()->AvatarActor->FindComponentByClass<UEquipmentComponent>())
	{
		UE_LOG(LogTemp, Warning, TEXT("FireOnce CheckCost: %i"), EquipmentComponent->GetCurrentClipAmmo() > 0.f)
		return EquipmentComponent->GetCurrentClipAmmo() > 0.f;
	}

	return false;
}

bool UGameplayAbility_FireOnce::CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
												  const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
{
	if (auto EquipmentComponent = GetCurrentActorInfo()->AvatarActor->FindComponentByClass<UEquipmentComponent>())
	{
		EquipmentComponent->SpendAmmo(1.f);
	}

	return true;
}

void UGameplayAbility_FireOnce::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ServerWaitForClientDataTask)
	{
		ServerWaitForClientDataTask->EndTask();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_FireOnce::OnValidDataAcquired(const FGameplayAbilityTargetDataHandle& Data)
{
	CommitAbilityCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), nullptr);

	const FGameplayAbilityTargetData* DataPtr = Data.Get(0);
	if (DataPtr)
	{
		FGameplayCueParameters Parameters;
		Parameters.EffectCauser = GetAvatarActorFromActorInfo();
		Parameters.Location = DataPtr->GetEndPoint();

		
		// apply damage
		auto AbilitySystemInterface = Cast<IAbilitySystemInterface>(DataPtr->GetHitResult()->GetActor());
		if (AbilitySystemInterface)
		{
			UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface->GetAbilitySystemComponent();

			const FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(DamageEffect);

			Parameters.EffectContext = EffectSpec.Data.Get()->GetEffectContext();
			Parameters.EffectContext.AddHitResult(*DataPtr->GetHitResult());

			EffectSpec.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Value.Damage"), 15.f);
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
		}

		auto AbilitySystemComponent = Cast<UFPSAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->ExecuteGameplayCue(ShotGameplayCue, Parameters);
		}

		LastShotTime = GetWorld()->GetTimeSeconds();
	}
}
