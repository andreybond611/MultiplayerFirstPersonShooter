// Copyright Andrei Bondarenko 2023

#include "GameplayAbilitySystem/TargetActors/TargetActor_LineTrace.h"
#include "Abilities/GameplayAbility.h"
#include "Camera/CameraComponent.h"
#include "Characters/PlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Types/CollisionTypes.h"

ATargetActor_LineTrace::ATargetActor_LineTrace()
{
	ShouldProduceTargetDataOnServer = false;
}

TArray<FHitResult> ATargetActor_LineTrace::PerformTrace()
{
	auto Character = Cast<APlayerCharacter>(OwningAbility->GetCurrentSourceObject());
	if (ensure(Character))
	{
		const FVector StartTrace = Character->GetFirstPersonCamera()->GetComponentLocation();
		const FVector EndTrace = StartTrace + Character->GetBaseAimRotation().Vector() * 9999.f;

		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(Character);
		TArray<FHitResult> Results;

		const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(BULLET_TRACE_COLLISION);

		TArray<FHitResult> LineTraceMultiResults;
		UKismetSystemLibrary::LineTraceMulti(GetWorld(), StartTrace, EndTrace, TraceType, true, ActorsToIgnore,
											 EDrawDebugTrace::None, LineTraceMultiResults, true, FLinearColor::Yellow);

		// make hits from LineTraceMulti unique
		TArray<AActor*> HitActors;
		for (FHitResult MultiResult : LineTraceMultiResults)
		{
			if (!HitActors.Contains(MultiResult.GetActor()))
			{
				Results.Add(MultiResult);
				HitActors.Add(MultiResult.GetActor());
			}
		}

		if (Results.IsEmpty())
		{
			FHitResult NoHitHappenedHitResult = FHitResult();
			NoHitHappenedHitResult.TraceStart = StartTrace;
			NoHitHappenedHitResult.TraceEnd = EndTrace;
			NoHitHappenedHitResult.Location = EndTrace;
			Results.Add(NoHitHappenedHitResult);
		}

		return Results;
	}
	return {};
}

void ATargetActor_LineTrace::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
}

bool ATargetActor_LineTrace::IsConfirmTargetingAllowed()
{
	return Super::IsConfirmTargetingAllowed();
}

void ATargetActor_LineTrace::ConfirmTargetingAndContinue()
{
	if (IsConfirmTargetingAllowed())
	{
		TArray<FHitResult> HitResults = PerformTrace();
		FGameplayAbilityTargetDataHandle Handle = MakeTargetData(HitResults);
		TargetDataReadyDelegate.Broadcast(Handle);
	}
}

FGameplayAbilityTargetDataHandle ATargetActor_LineTrace::MakeTargetData(const TArray<FHitResult>& HitResults) const
{
	FGameplayAbilityTargetDataHandle ReturnDataHandle;

	for (int32 i = 0; i < HitResults.Num(); i++)
	{
		/** Note: These are cleaned up by the FGameplayAbilityTargetDataHandle (via an internal TSharedPtr) */
		FGameplayAbilityTargetData_SingleTargetHit* ReturnData = new FGameplayAbilityTargetData_SingleTargetHit();
		ReturnData->HitResult = HitResults[i];
		ReturnDataHandle.Add(ReturnData);
	}

	return ReturnDataHandle;
}

void ATargetActor_LineTrace::ConfirmTargeting()
{
	Super::ConfirmTargeting();
}

void ATargetActor_LineTrace::CancelTargeting()
{
	Super::CancelTargeting();

}

void ATargetActor_LineTrace::BindToConfirmCancelInputs()
{
	Super::BindToConfirmCancelInputs();
}

bool ATargetActor_LineTrace::ShouldProduceTargetData() const
{
	return Super::ShouldProduceTargetData();
}

bool ATargetActor_LineTrace::OnReplicatedTargetDataReceived(FGameplayAbilityTargetDataHandle& Data) const
{
	return Super::OnReplicatedTargetDataReceived(Data);
}

void ATargetActor_LineTrace::BeginPlay()
{
	Super::BeginPlay();
}
