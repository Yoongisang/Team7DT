// LidarSensorComponent.cpp

#include "LidarSensorComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

ULidarSensorComponent::ULidarSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULidarSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Lidar Sensor Start"));
}

void ULidarSensorComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(
		DeltaTime,
		TickType,
		ThisTickFunction
	);

	AActor* Owner = GetOwner();

	if (!Owner)
	{
		return;
	}

	LidarPoints.Empty();

	FVector Origin =
		Owner->GetActorLocation()
		+ Owner->GetActorRotation().RotateVector(SensorOffset);

	for (float VerticalAngle : ChannelElevations)
	{
		for (float HorizontalAngle = 0.0f;
			HorizontalAngle < 360.0f;
			HorizontalAngle += HorizontalAngleStep)
		{
			FRotator Rotation(
				VerticalAngle,
				HorizontalAngle + Owner->GetActorRotation().Yaw,
				0.0f
			);

			FVector Direction = Rotation.Vector();

			FVector End =
				Origin + Direction * MaxDistance;

			FHitResult Hit;

			bool bHit = GetWorld()->LineTraceSingleByChannel(
				Hit,
				Origin,
				End,
				ECC_Visibility
			);

			FLidarPoint PointData;

			if (bHit)
			{
				float Noise =
					FMath::FRandRange(-RangeNoise, RangeNoise);

				float FinalDistance =
					Hit.Distance + Noise;

				PointData.Location =
					Origin + Direction * FinalDistance;

				PointData.Distance = FinalDistance;
				PointData.bHit = true;

				if (bDrawDebug)
				{
					DrawDebugPoint(
						GetWorld(),
						PointData.Location,
						8.0f,
						FColor::Red,
						false,
						0.05f
					);

					DrawDebugLine(
						GetWorld(),
						Origin,
						PointData.Location,
						FColor::Green,
						false,
						0.05f,
						0,
						1.0f
					);
				}
			}
			else
			{
				PointData.Location = End;
				PointData.Distance = MaxDistance;
				PointData.bHit = false;

				if (bDrawDebug)
				{
					DrawDebugLine(
						GetWorld(),
						Origin,
						End,
						FColor::Blue,
						false,
						0.05f,
						0,
						0.5f
					);
				}
			}

			LidarPoints.Add(PointData);
		}
	}
}

const TArray<FLidarPoint>& ULidarSensorComponent::GetLidarPoints() const
{
	return LidarPoints;
}