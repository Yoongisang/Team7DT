// LidarSensorComponent.cpp

#include "Sensor/LidarSensorComponent.h"
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
	
	if (bScanning == false) 
		return;
	
	AActor* Owner = GetOwner();

	if (!Owner)
	{
		return;
	}
	// 포인트 클라우드 초기화 
	LastPointCloud.Reset();
	LastPointCloud.SensorTransform = Owner->GetActorTransform();

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

			if (bHit)
			{
				float Noise =
					FMath::FRandRange(-RangeNoise, RangeNoise);

				float FinalDistance =
					Hit.Distance + Noise;
				
				FVector HitPoint = Origin + Direction * FinalDistance;
				
				LastPointCloud.Points.Add(HitPoint);
				LastPointCloud.Intensities.Add(
					FMath::Clamp(1.f - (FinalDistance / MaxDistance), 0.f, 1.f));
				

				if (bDrawDebug)
				{
					DrawDebugPoint(
						GetWorld(),
						HitPoint,
						8.0f,
						FColor::Red,
						false,
						0.05f
					);

					DrawDebugLine(
						GetWorld(),
						Origin,
						HitPoint,
						FColor::Green,
						false,
						0.05f,
						0,
						1.0f
					);
				}
			}
			else if (bDrawDebug)
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
	}
	LastPointCloud.PointCount  = LastPointCloud.Points.Num();
	LastPointCloud.FrameNumber = ++FrameCount;

	OnPointCloudUpdated.Broadcast(LastPointCloud);
}

void ULidarSensorComponent::StartScan()
{
	bScanning = true;
	SetComponentTickEnabled(true);
}

void ULidarSensorComponent::StopScan()
{
	bScanning = false;
	SetComponentTickEnabled(false);
}