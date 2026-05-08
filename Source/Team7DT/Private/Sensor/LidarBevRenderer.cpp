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
	// 그리드 그리기 (10m 간격)
	const float GridSpacingMeters = 10.f;        // 10m 간격
	const float GridSpacingCm = GridSpacingMeters * 100.f;
	const int32 GridPixelStep = FMath::RoundToInt32(GridSpacingCm * Scale);
	const FColor GridColor(40, 40, 40, 255);     // 어두운 회색

	if (GridPixelStep > 0)
	{
		// 가로선
		for (int32 y = 0; y < ImgSize; y += GridPixelStep)
		{
			const int32 Row = y * ImgSize;
			for (int32 x = 0; x < ImgSize; ++x)
				Pixels[Row + x] = GridColor;
		}
		// 세로선
		for (int32 x = 0; x < ImgSize; x += GridPixelStep)
		{
			for (int32 y = 0; y < ImgSize; ++y)
				Pixels[y * ImgSize + x] = GridColor;
		}
	}

	// 월드 좌표 > 센서 기준 좌표 변환
	const FTransform InvSensor = SensorTransform.Inverse();
	const int32 PointCount = PointCloud.PointCount;
	// 3D 좌표
	const FVector* RESTRICT Points = PointCloud.Points.GetData();
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
		constexpr float MinZ = -200.f;   // cm 단위 (LiDAR 기준 -2m)
		constexpr float MaxZ = 500.f;    // cm 단위 (LiDAR 기준 +5m)
		const float ZNorm = FMath::Clamp((LocalPt.Z - MinZ) / (MaxZ - MinZ), 0.f, 1.f);
		const FColor Color = ColorLUT[
			static_cast<uint8>(FMath::Clamp(ZNorm * 255.f, 0.f, 255.f))];

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

	// 차량 방향 화살표 (위쪽이 차량 정면)
	const int32 CX = FMath::RoundToInt32(HalfSize);
	const int32 CY = FMath::RoundToInt32(HalfSize);
	const FColor ArrowColor(255, 255, 0, 255); 
	const int32 ArrowSize = 8; 

	// 삼각형: (CX-w, CY+h) → (CX+w, CY+h) → (CX, CY-h)
	for (int32 dy = -ArrowSize; dy <= ArrowSize; ++dy)
	{
		const int32 Y = CY + dy;
		if (Y < 0 || Y >= ImgSize) continue;

		// 위쪽이 뾰족, 아래쪽이 넓은 삼각형
		const float Progress = float(dy + ArrowSize) / float(2 * ArrowSize);
		const int32 HalfWidth = FMath::RoundToInt32(Progress * ArrowSize);

		const int32 Row = Y * ImgSize;
		for (int32 dx = -HalfWidth; dx <= HalfWidth; ++dx)
		{
			const int32 X = CX + dx;
			if (X < 0 || X >= ImgSize) continue;
			Pixels[Row + X] = ArrowColor;
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
