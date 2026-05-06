#include "Team7DT/Public/Sensor/SensorViewWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"

void USensorViewWidget::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	if (!InRenderTarget || !SensorImage) return;

	// UI 이미지에 텍스처 넣음
	SensorImage->SetBrushResourceObject(InRenderTarget);
}

void USensorViewWidget::SetLidarRenderTarget(UTexture2D* InRenderTarget)
{
	if (!InRenderTarget || !LidarImage) return;

	// UI 이미지에 텍스처 넣음
	LidarImage->SetBrushResourceObject(InRenderTarget);
}

void USensorViewWidget::ToggleCameraView()
{
	if (!SensorBorder) return;

	const ESlateVisibility NewVis = IsCameraViewVisible()
		                                ? ESlateVisibility::Collapsed
		                                : ESlateVisibility::SelfHitTestInvisible;
	SensorBorder->SetVisibility(NewVis);
}

void USensorViewWidget::ToggleLidarView()
{
	if (!LidarBorder) return;

	const ESlateVisibility NewVis = IsLidarViewVisible()
		                                ? ESlateVisibility::Collapsed
		                                : ESlateVisibility::SelfHitTestInvisible;
	LidarBorder->SetVisibility(NewVis);
}

bool USensorViewWidget::IsCameraViewVisible() const
{
	return SensorBorder && SensorBorder->GetVisibility() != ESlateVisibility::Collapsed;
}

bool USensorViewWidget::IsLidarViewVisible() const
{
	return LidarBorder && LidarBorder->GetVisibility() != ESlateVisibility::Collapsed;
}

void USensorViewWidget::NativeConstruct()
{
	// AddToViewport 때 호출
	Super::NativeConstruct();

	if (SensorBorder)
	{
		SensorBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (LidarBorder)
	{
		LidarBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}
