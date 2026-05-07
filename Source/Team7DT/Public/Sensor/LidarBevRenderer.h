#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LidarBevRenderer.generated.h"
//#include "CameraSensorTypes.h"

#pragma region 임시 CameraSensorTypes Data Structs
USTRUCT(BlueprintType)
struct FBevRenderConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BEV", meta = (ClampMin = "64", ClampMax = "2048"))
	int32 ImageSize = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BEV", meta = (ClampMin = "100.0"))
	float ViewRange = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BEV")
	FLinearColor PointColor = FLinearColor(0.0f, 1.0f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BEV")
	FLinearColor BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BEV", meta = (ClampMin = "1.0", ClampMax = "8.0"))
	float PointSize = 2.0f;
};

USTRUCT(BlueprintType)
struct FLidarPointCloudData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PointCloud")
	TArray<FVector> Points;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PointCloud")
	TArray<float> Intensities;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PointCloud")
	int32 PointCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PointCloud")
	int64 FrameNumber = 0;

	void Reset()
	{
		Points.Reset();
		Intensities.Reset();
		PointCount = 0;
	}
};

#pragma endregion

UCLASS()
class TEAM7DT_API ULidarBevRenderer : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const FBevRenderConfig& InConfig);
	void RenderPointCloud(const FLidarPointCloudData& PointCloud, const FTransform& SensorTransform);
	UTexture2D* GetRenderTarget() const { return DynamicTexture; }
	void UpdateConfig(const FBevRenderConfig& InConfig);

private:
	void CreateTexture();
	void BuildColorLUT();
	
	UPROPERTY()
	TObjectPtr<UTexture2D> DynamicTexture;
	
	TArray<FColor> PixelBuffer;
	FUpdateTextureRegion2D UpdateRegion;
	FBevRenderConfig Config;
	FColor ColorLUT[256];
};
