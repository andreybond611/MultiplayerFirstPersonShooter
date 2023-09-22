// Copyright Andrei Bondarenko 2023

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "AbilitySystemInterface.h"
#include "PlayerCharacter.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UFPSAbilitySystemComponent;
struct FInputActionValue;
class UAbilitySystemComponent;
class UCameraComponent;
class USkeletalMeshComponent;

/**
 * 
 */
UCLASS()
class MPFPS_API APlayerCharacter : public ABaseCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UCameraComponent* GetFPCamera() const { return FirstPersonCamera; }
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputDataAsset* InputActions;
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere)
	UFPSAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> AttributeInitializationEffect;

	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* FirstPersonMesh;
	UPROPERTY(EditAnywhere)
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayAbility> FireAbility;

	void GrantAbilities();
	void InitializeAttributes();

	// Input callbacks

	void Look(const FInputActionValue& InputActionValue);
	void Move(const FInputActionValue& InputActionValue);

	void PrimaryActionPressed();
	void PrimaryActionReleased();
	void SecondaryActionActionPressed();
	void SecondaryActionActionReleased();

};
