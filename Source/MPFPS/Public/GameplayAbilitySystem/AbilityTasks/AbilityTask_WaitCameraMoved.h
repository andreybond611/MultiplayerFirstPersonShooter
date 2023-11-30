// Copyright Andrei Bondarenko 2023

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WaitCameraMoved.generated.h"

class APlayerCharacter;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCameraMoved);

/**
 *
 */
UCLASS()
class MPFPS_API UAbilityTask_WaitCameraMoved : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_WaitCameraMoved();

	UPROPERTY(BlueprintAssignable)
	FOnCameraMoved OnCameraMoved;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
			  meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_WaitCameraMoved* WaitCameraMoved(UGameplayAbility* OwningAbility, FName TaskInstanceName, APlayerCharacter* PlayerCharacter);

	virtual void Activate() override;

private:
	TWeakObjectPtr<APlayerCharacter> Character;

	UFUNCTION()
	void CameraMoved();
};
