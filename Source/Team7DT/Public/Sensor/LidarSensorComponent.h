// LidarSensorComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sensor/SensorTypes.h"
#include "LidarSensorComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPointCloudUpdated, const FLidarPointCloudData&);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEAM7DT_API ULidarSensorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULidarSensorComponent();
	
	UFUNCTION(BlueprintCallable, Category="LiDAR")
	void StartScan();

	UFUNCTION(BlueprintCallable, Category="LiDAR")
	void StopScan();

	UFUNCTION(BlueprintPure, Category="LiDAR")
	const FLidarPointCloudData& GetPointCloud() const { return LastPointCloud; }
	
	FOnPointCloudUpdated OnPointCloudUpdated;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	// 최대 탐지 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiDAR")
	float MaxDistance = 3000.0f;

	// 수평 회전 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiDAR")
	float HorizontalAngleStep = 5.0f;

	// 수직 채널 각도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiDAR")
	TArray<float> ChannelElevations =
	{
		-10.0f,
		0.0f,
		10.0f
	};

	// 거리 노이즈
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiDAR")
	float RangeNoise = 0.0f;

	// 디버그 표시 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiDAR")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiDAR")
	FVector SensorOffset = FVector(0.0f, 0.0f, 0.0f);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LiDAR")
	FLidarPointCloudData LastPointCloud;
	
private:
	bool bScanning = true;
	int64 FrameCount = 0;
};