// Copyright Andrei Bondarenko 2023

#include "GameplayAbilitySystem/Abilities/GA_Interact.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Camera/CameraComponent.h"
#include "Characters/ShootingCharacter.h"
#include "Components/InteractionComponent.h"
#include "GameplayAbilitySystem/AbilityTasks/AbilityTask_Tick.h"
#include "GameplayAbilitySystem/AbilityTasks/AbilityTask_WaitTargetDataUsingActor.h"
#include "GameplayAbilitySystem/TargetActors/TargetActor_LineTrace.h"
#include "Types/CollisionTypes.h"
#include "Weapons/EquipmentComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayFramework/FPSPlayerController.h"
#include "UI/FPSHUD.h"
#include "UI/HUDWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogInteractAbility, All, All);

void UGA_Interact::FindInteractionWidget()
{
	auto PlayerController = PlayerCharacter->GetController<AFPSPlayerController>();
	if (!PlayerController)
	{
		return;
	}

	auto HUD = PlayerController->GetHUD<AFPSHUD>();
	if (!HUD)
	{
		return;
	}

	if (!HUD->GetHUDWidget())
	{
		return;
	}

	InteractionWidget = HUD->GetHUDWidget()->GetInteractionWidget();
}

void UGA_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	TimeActive = 0.f;

	PlayerCharacter = Cast<AShootingCharacter>(ActorInfo->AvatarActor);
	if (!PlayerCharacter.IsValid())
	{
		UE_LOG(LogInteractAbility, Error, TEXT("Interact ability tried to activate on non-PlayerCharacter actor"));
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	FindInteractionWidget();

	if (!TargetActor)
	{
		SpawnTargetActor();
	}

	auto LineTraceActor = Cast<ATargetActor_LineTrace>(TargetActor);
	if (LineTraceActor)
	{
		const FVector CameraLocation = PlayerCharacter->GetFirstPersonCamera()->GetComponentLocation();
		const FVector EndTrace =
			CameraLocation + PlayerCharacter->GetFirstPersonCamera()->GetForwardVector() * PlayerCharacter->GetInteractionLength();

		LineTraceActor->Configure(CameraLocation, EndTrace, INTERACTION_TRACE_COLLISION);
	}

	WaitTargetDataTask = UAbilityTask_WaitTargetDataUsingActor::WaitTargetDataWithReusableActor(
		this, NAME_None, EGameplayTargetingConfirmation::Instant, TargetActor, true);

	WaitTargetDataTask->ValidData.AddDynamic(this, &UGA_Interact::OnFirstValidDataAcquired);
	WaitTargetDataTask->ReadyForActivation();
}

void UGA_Interact::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	if (InteractionComponent.IsValid())
	{
		SetInteractionProgress(0.f);
	}
}

void UGA_Interact::SpawnTargetActor()
{
	if (!TargetActor)
	{
		AGameplayAbilityTargetActor* SpawnedActor = nullptr;

		UAbilityTask_WaitTargetData* SpawnTargetActorTask = UAbilityTask_WaitTargetData::WaitTargetData(
			this, NAME_None, EGameplayTargetingConfirmation::Instant, ATargetActor_LineTrace::StaticClass());

		SpawnTargetActorTask->BeginSpawningActor(this, ATargetActor_LineTrace::StaticClass(), SpawnedActor);
		SpawnTargetActorTask->FinishSpawningActor(this, SpawnedActor);

		TargetActor = SpawnedActor;
	}
}

void UGA_Interact::OnValidDataAcquired(const FGameplayAbilityTargetDataHandle& Data)
{
	const FGameplayAbilityTargetData* DataPtr = Data.Get(0);
	if (DataPtr && DataPtr->GetHitResult())
	{
		if (InteractionComponent == Cast<UInteractionComponent>(DataPtr->GetHitResult()->GetComponent()))
		{
			return;
		}
	}

	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->EndTask();
	}
	if (TickTask)
	{
		TickTask->EndTask();
	}

	CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
}

void UGA_Interact::SetInteractionProgress(const float CurrentInteractionProgress)
{
	InteractionProgress = CurrentInteractionProgress;
	InteractionWidget->SetInteractionProgress(InteractionProgress);
}

void UGA_Interact::InteractTick(float DeltaTime)
{
	if (InteractionComponent->GetActivationTime() <= TimeActive)
	{
		ActivateInteractionAbility();
		SetInteractionProgress(0.f);
		return;
	}

	check(InteractionComponent->GetActivationTime() != 0.f);
	const float CurrentInteractionProgress = TimeActive / InteractionComponent->GetActivationTime();

	SetInteractionProgress(CurrentInteractionProgress);

	if (!TargetActor)
	{
		SpawnTargetActor();
	}

	auto LineTraceActor = Cast<ATargetActor_LineTrace>(TargetActor);
	if (LineTraceActor)
	{
		const FVector CameraLocation = PlayerCharacter->GetFirstPersonCamera()->GetComponentLocation();
		const FVector EndTrace =
			CameraLocation + PlayerCharacter->GetFirstPersonCamera()->GetForwardVector() * PlayerCharacter->GetInteractionLength();

		LineTraceActor->Configure(CameraLocation, EndTrace, INTERACTION_TRACE_COLLISION);
	}

	TimeActive += DeltaTime;

	WaitTargetDataTask = UAbilityTask_WaitTargetDataUsingActor::WaitTargetDataWithReusableActor(
		this, NAME_None, EGameplayTargetingConfirmation::Instant, TargetActor, true);

	WaitTargetDataTask->ValidData.AddDynamic(this, &UGA_Interact::OnValidDataAcquired);
	WaitTargetDataTask->ReadyForActivation();
}

void UGA_Interact::OnInputRelease(float TimeHeld)
{
	if (TickTask)
	{
		TickTask->EndTask();
	}

	CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
}

void UGA_Interact::ActivateInteractionAbility()
{
	TSubclassOf<UGameplayAbility> InteractionAbility = InteractionComponent->GetInteractionAbility();
	if (InteractionAbility)
	{
		bool bActivated = TargetAbilitySystemComponent->TryActivateAbilityByClass(InteractionAbility);

		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
	}
}

void UGA_Interact::OnFirstValidDataAcquired(const FGameplayAbilityTargetDataHandle& Data)
{
	const FGameplayAbilityTargetData* DataPtr = Data.Get(0);
	if (!(DataPtr && DataPtr->GetHitResult()))
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
		return;
	}

	InteractionComponent = Cast<UInteractionComponent>(DataPtr->GetHitResult()->GetComponent());
	if (!InteractionComponent.IsValid())
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
		return;
	}

	TargetAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();

	if (InteractionComponent->IsInstant())
	{
		ActivateInteractionAbility();
	}
	else
	{
		if (GetCurrentActorInfo()->IsLocallyControlled())
		{
			TickTask = UAbilityTask_Tick::AbilityTaskTick(this, NAME_None);
			TickTask->OnTick.AddDynamic(this, &UGA_Interact::InteractTick);
			TickTask->ReadyForActivation();

			WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
			WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_Interact::OnInputRelease);
			WaitInputReleaseTask->ReadyForActivation();
		}
	}
}
