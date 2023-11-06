// Copyright Andrei Bondarenko 2023

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "FPSTypes.generated.h"

USTRUCT()
struct FWeaponStats
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	float MaxClipAmmo;

	UPROPERTY(EditAnywhere)
	float MaxReserveAmmo;

	UPROPERTY(EditAnywhere)
	float DamagePerBullet;
	UPROPERTY(EditAnywhere)
	float HeadshotDamage;
	UPROPERTY(EditAnywhere)
	float LegshotDamage;

	UPROPERTY(EditAnywhere)
	float ShotCooldown;

	UPROPERTY(EditAnywhere)
	float StandSpread = 0.01f;
	UPROPERTY(EditAnywhere)
	float MaxStandSpread = 0.2f;
	UPROPERTY(EditAnywhere)
	float WalkSpread = 0.3f;
	UPROPERTY(EditAnywhere)
	float MaxWalkSpread = 0.5f;
	UPROPERTY(EditAnywhere)
	float SpreadDecay = 0.5f;
	UPROPERTY(EditAnywhere)
	float Range = 9999.f;
};

UENUM(BlueprintType)
enum class EFPSTeam : uint8
{
	Neutral UMETA(DisplayName = "Neutral"),
	Player UMETA(DisplayName = "Player"),
	Zombies UMETA(DisplayName = "Zombies"),
};

USTRUCT(BlueprintType, meta = (ScriptName = "Attitude"))
struct FTeamAttitude
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<TEnumAsByte<ETeamAttitude::Type>> Attitude;

	FTeamAttitude() = default;

	FTeamAttitude(std::initializer_list<TEnumAsByte<ETeamAttitude::Type>> InAttitudes)
		: Attitude(MoveTemp(InAttitudes))
	{
	}
};
