// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Team7DTPawn.h"
#include "Team7DTSportsCar.generated.h"

class UCameraSensorComponent;
class ULidarSensorComponent;
/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class TEAM7DT_API ATeam7DTSportsCar : public ATeam7DTPawn
{
	GENERATED_BODY()
	
public:

	ATeam7DTSportsCar();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sensor")
	UCameraSensorComponent* CameraSensor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sensor")
	ULidarSensorComponent* LidarSensor;
};
