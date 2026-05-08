#pragma once

#include "CoreMinimal.h"
#include "SensorTypes.generated.h"

// LiDAR 포인트 클라우드 데이터
USTRUCT(BlueprintType)
struct FLidarPointCloudData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PointCloud")
	TArray<FVector> Points;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PointCloud")
	TArray<float> Intensities;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PointCloud")
	int32 PointCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PointCloud")
	int64 FrameNumber = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PointCloud")
	FTransform SensorTransform; 
	
	void Reset()
	{
		Points.Reset();
		Intensities.Reset();
		PointCount = 0;
		SensorTransform = FTransform::Identity;
	}
};

// BEV 렌더 설정 
USTRUCT(BlueprintType)
struct FBevRenderConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BEV", meta=(ClampMin="64", ClampMax="2048"))
	int32 ImageSize = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BEV", meta=(ClampMin="100.0"))
	float ViewRange = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BEV")
	FLinearColor PointColor = FLinearColor(0.0f, 1.0f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BEV")
	FLinearColor BackgroundColor = FLinearColor(0.f, 0.f, 0.f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BEV", meta=(ClampMin="1.0", ClampMax="8.0"))
	float PointSize = 2.0f;
};