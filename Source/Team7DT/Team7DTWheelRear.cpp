// Copyright Epic Games, Inc. All Rights Reserved.

#include "Team7DTWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UTeam7DTWheelRear::UTeam7DTWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}