#include "Sensor/LidarBevRenderer.h"
#include "Microsoft/AllowMicrosoftPlatformTypes.h"

void ULidarBevRenderer::Initialize(const FBevRenderConfig& InConfig)
{
	Config = InConfig;
	CreateTexture();
	BuildColorLUT();
}

void ULidarBevRenderer::RenderPointCloud(const FLidarPointCloudData& PointCloud, const FTransform& SensorTransform)
{
	// 3D > 2D
	if (!DynamicTexture) return;

	const int32 ImgSize = Config.ImageSize;
	const int32 TotalPixels = ImgSize * ImgSize;
	const float HalfSize = static_cast<float>(ImgSize) * 0.5f;
	const float Scale = HalfSize / Config.ViewRange;
	const int32 PtSize = FMath::Max(Config.PointSize, 1);
	const int32 PtHalf = PtSize / 2;

	const FColor BgColor = Config.BackgroundColor.ToFColor(true);
	FColor* RESTRICT Pixels = PixelBuffer.GetData();
	// 전체 화면 배경색으로 초기화
	for (int32 i = 0; i < TotalPixels; ++i)
	{
		Pixels[i] = BgColor;
	}

	// 월드 좌표 > 센서 기준 좌표 변환
	const FTransform InvSensor = SensorTransform.Inverse();
	const int32 PointCount = PointCloud.PointCount;
	// 3D 좌표
	const FVector* RESTRICT Points = PointCloud.Points.GetData();
	// 반사 강도
	const float* RESTRICT Intensities = PointCloud.Intensities.GetData();
	const int32 IntensityCount = PointCloud.Intensities.Num();

	// Bev 변환
	for (int32 i = 0; i < PointCount; ++i)
	{
		// 월드 > 로컬 좌표 변환
		const FVector LocalPt = InvSensor.TransformPosition(Points[i]);

		// 2D 픽셀 좌표 변환
		const int32 CX = FMath::RoundToInt32(HalfSize + LocalPt.Y * Scale);
		const int32 CY = FMath::RoundToInt32(HalfSize - LocalPt.X * Scale);

		// 이미지 범위 밖 포인트 제거
		if (CX < PtHalf || CX >= ImgSize - PtHalf || CY < PtHalf || CY >= ImgSize - PtHalf)
		{
			continue;
		}

		// 색 변환
		const float Intensity = (i < IntensityCount) ? Intensities[i] : 0.5f;
		const FColor Color = ColorLUT[
			static_cast<uint8>(FMath::Clamp(Intensity * 255.f, 0.f, 255.f))];

		if (PtSize == 1)
		{
			Pixels[CY * ImgSize + CX] = Color;
		}
		else
		{
			for (int32 dy = -PtHalf; dy < PtSize - PtHalf; ++dy)
			{
				const int32 Row = (CY + dy) * ImgSize;
				for (int32 dx = -PtHalf; dx < PtSize - PtHalf; ++dx)
				{
					Pixels[Row + CX + dx] = Color;
				}
			}
		}
	}

	// 화면 중앙 표시(센서 위치)
	const int32 C = FMath::RoundToInt32(HalfSize);
	const FColor White(255, 255, 255, 255);
	for (int32 dy = -3; dy < 3; ++dy)
	{
		const int32 Row = (C + dy) * ImgSize;
		for (int32 dx = -3; dx < 3; ++dx)
		{
			Pixels[Row + C + dx] = White;
		}
	}

	// cpu 픽셀 버퍼 > gpu 텍스처 반영
	DynamicTexture->UpdateTextureRegions(
		0, 1, &UpdateRegion,
		ImgSize * sizeof(FColor),
		sizeof(FColor),
		reinterpret_cast<uint8*>(Pixels)
	);
}

void ULidarBevRenderer::UpdateConfig(const FBevRenderConfig& InConfig)
{
	// 해상도 변경 시 새로 업뎃
	const bool bSizeChanged = (Config.ImageSize != InConfig.ImageSize);
	Config = InConfig;
	BuildColorLUT();
	if (bSizeChanged)
	{
		CreateTexture();
	}
}

void ULidarBevRenderer::HandlePointCloud(const FLidarPointCloudData& PointCloud)
{
	RenderPointCloud(PointCloud, PointCloud.SensorTransform);
}

void ULidarBevRenderer::CreateTexture()
{
	const int32 Size = Config.ImageSize; // 크기 설정

	// 런타임 전용 텍스처
	DynamicTexture = UTexture2D::CreateTransient(Size, Size, PF_B8G8R8A8);
	// 필터 TF_Nearest
	DynamicTexture->Filter = TF_Nearest;
	// 색 처리
	DynamicTexture->SRGB = true;
	// Gpu 메모리에 텍스처 생성
	DynamicTexture->UpdateResource();

	PixelBuffer.SetNumUninitialized(Size * Size);

	// 텍스처 전체 업뎃
	UpdateRegion = FUpdateTextureRegion2D(0, 0, 0, 0, Size, Size);
}

void ULidarBevRenderer::BuildColorLUT()
{
	// 값 > 컬러 맵핑

	const FLinearColor DarkColor = FLinearColor(0.0f, 0.f, 0.3f);
	const FLinearColor BrightColor = Config.PointColor;
	// 크기 256
	for (int32 i = 0; i < 256; ++i)
	{
		// 선형 보간
		ColorLUT[i] = FMath::Lerp(
			DarkColor,
			BrightColor,
			static_cast<float>(i) / 255.0f
		).ToFColor(true);
	}
}
