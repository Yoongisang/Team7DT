// Copyright Epic Games, Inc. All Rights Reserved.


#include "Team7DTPlayerController.h"
#include "Team7DTVehicle.h"          
#include "Team7DTUI.h"
#include "Sensor/SensorViewWidget.h"   
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"

void ATeam7DTPlayerController::ToggleSensorView(UTextureRenderTarget2D* CameraRT)
{
	if (!SensorViewWidget) return;
	SensorViewWidget->SetRenderTarget(CameraRT);
	SensorViewWidget->ToggleCameraView();
}

void ATeam7DTPlayerController::ToggleLidarView(UTexture2D* LidarTex)
{
	if (!SensorViewWidget) return;
	SensorViewWidget->SetLidarRenderTarget(LidarTex);
	SensorViewWidget->ToggleLidarView();
}

bool ATeam7DTPlayerController::IsLidarViewVisible() const
{
	return SensorViewWidget ? SensorViewWidget->IsLidarViewVisible() : false;
}

void ATeam7DTPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 메인 UI
	VehicleUI = CreateWidget<UTeam7DTUI>(this, VehicleUIClass);
	check(VehicleUI);
	VehicleUI->AddToViewport();

	// ── 센서뷰 위젯 생성 ──
	if (SensorViewWidgetClass)
	{
		SensorViewWidget = CreateWidget<USensorViewWidget>(this, SensorViewWidgetClass);
		if (SensorViewWidget)
		{
			SensorViewWidget->AddToViewport();
		}
	}
}

void ATeam7DTPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);

		if (bUseSteeringWheelControls && SteeringWheelInputMappingContext)
		{
			Subsystem->AddMappingContext(SteeringWheelInputMappingContext, 1);
		}
	}
}

void ATeam7DTPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}
}

void ATeam7DTPlayerController::OnPossess(APawn* InPawn)
{
	// ATeam7DTVehicle로 캐스팅 (CheckedCast 대신 일반 Cast로 안전성 ↑)
	Super::OnPossess(InPawn);
	VehiclePawn = Cast<ATeam7DTVehicle>(InPawn);
}
