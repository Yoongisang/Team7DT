// Copyright Epic Games, Inc. All Rights Reserved.

#include "Team7DTWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UTeam7DTWheelFront::UTeam7DTWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}