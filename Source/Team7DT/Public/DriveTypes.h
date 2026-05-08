// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DriveTypes.generated.h"

// 주행 상태 구조체
USTRUCT(BlueprintType)
struct FDriveState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) 
	FVector Location  = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) 
	FRotator Rotation  = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadOnly) 
	float SpeedKmh  = 0.f;
	UPROPERTY(BlueprintReadOnly) 
	float Throttle  = 0.f;
	UPROPERTY(BlueprintReadOnly) 
	float Steering  = 0.f;
	UPROPERTY(BlueprintReadOnly) 
	float Brake     = 0.f;
	UPROPERTY(BlueprintReadOnly) 
	float TimeStamp = 0.f;
};

// 델리게이트 시그니처
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDriveStateUpdated, const FDriveState&);