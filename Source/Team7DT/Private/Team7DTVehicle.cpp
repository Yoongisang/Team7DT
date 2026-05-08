// Fill out your copyright notice in the Description page of Project Settings.


#include "Team7DTVehicle.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "TimerManager.h"
#include "Component/SplineFollowerComponent.h"
#include "Team7DTPlayerController.h"
#include "Sensor/LidarSensorComponent.h"
#include "Sensor/CameraSensorComponent.h"
#include "Sensor/LidarBevRenderer.h"
#include "DataVisual/Team7DTAgentDataLoggerComponent.h"

ATeam7DTVehicle::ATeam7DTVehicle()
{
	PrimaryActorTick.bCanEverTick = true;
	// Front Camera
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FrontSpringArm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.f;
	FrontSpringArm->SetRelativeLocation(FVector(30.f, 0.f, 120.f));
	
	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FrontCamera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;
	
	// Back Camera
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("BackSpringArm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.f;
	BackSpringArm->SocketOffset.Z = 150.f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.f;
	BackSpringArm->CameraLagMaxDistance = 50.f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BackCamera"));
	BackCamera->SetupAttachment(BackSpringArm);
	
	// Mesh / Physics
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	// 컴포넌트 생성
	SplineFollower = CreateDefaultSubobject<USplineFollowerComponent>(TEXT("SplineFollower"));

	LidarSensor = CreateDefaultSubobject<ULidarSensorComponent>(TEXT("LidarSensor"));

	CameraSensor = CreateDefaultSubobject<UCameraSensorComponent>(TEXT("CameraSensor"));

	DataLogger = CreateDefaultSubobject<UTeam7DTAgentDataLoggerComponent>(TEXT("DataLogger"));
}

void ATeam7DTVehicle::DoSteering(float SteeringValue)
{
	CurrentSteering = SteeringValue;
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ATeam7DTVehicle::DoThrottle(float ThrottleValue)
{
	UE_LOG(LogTemp, Warning, TEXT("[DoThrottle] Value: %.3f"), ThrottleValue);
    
	CurrentThrottle = ThrottleValue;
	CurrentBrake    = 0.f;
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
	ChaosVehicleMovement->SetBrakeInput(0.f);
}

void ATeam7DTVehicle::DoBrake(float BrakeValue)
{
	CurrentBrake    = BrakeValue;
	CurrentThrottle = 0.f;
	ChaosVehicleMovement->SetBrakeInput(BrakeValue);
	ChaosVehicleMovement->SetThrottleInput(0.f);
}

void ATeam7DTVehicle::DoBrakeStart()
{
	BrakeLights(true);
}

void ATeam7DTVehicle::DoBrakeStop()
{
	BrakeLights(false);
	ChaosVehicleMovement->SetBrakeInput(0.f);
	CurrentBrake = 0.f;
}

void ATeam7DTVehicle::StartSimulation()
{
	if (bSimulationRunning == true) 
		return;
	
	bSimulationRunning = true;
	SimulationElapsedTime = 0.f;

	OnSimulationStarted.Broadcast();

	const float Interval = 1.f / FMath::Max(DriveStateBroadcastHz, 0.1f);
	TWeakObjectPtr<ATeam7DTVehicle> WeakThis(this);
	
	GetWorld()->GetTimerManager().SetTimer(
		DriveStateTimer,
		FTimerDelegate::CreateLambda([WeakThis]()
		{
			if (WeakThis.IsValid() == false)
				return;
			WeakThis->OnDriveStateUpdated.Broadcast(WeakThis->BuildCurrentDriveState());
		}),
		Interval, true);
}

void ATeam7DTVehicle::StopSimulation()
{
	if (bSimulationRunning == false) 
		return;
	
	bSimulationRunning = false;
	GetWorld()->GetTimerManager().ClearTimer(DriveStateTimer);
	OnSimulationStopped.Broadcast();
}

void ATeam7DTVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(ResetVehicleAction,     ETriggerEvent::Triggered, this, &ATeam7DTVehicle::ResetVehicleInput);
		EIC->BindAction(ToggleCameraAction,     ETriggerEvent::Started,   this, &ATeam7DTVehicle::ToggleCameraInput);
		EIC->BindAction(ToggleSensorViewAction, ETriggerEvent::Started,   this, &ATeam7DTVehicle::ToggleSensorViewInput);
		EIC->BindAction(ToggleLidarViewAction,  ETriggerEvent::Started,   this, &ATeam7DTVehicle::ToggleLidarViewInput);
	}
}

void ATeam7DTVehicle::BeginPlay()
{
	Super::BeginPlay();
	// BevRenderer 동적 생성 (UObject라 CreateDefaultSubobject 안 됨)
	BevRenderer = NewObject<ULidarBevRenderer>(this, TEXT("BevRenderer"));
	
	if (BevRenderer)
	{
		FBevRenderConfig DefaultConfig;
		DefaultConfig.ViewRange = 4000.f;   
		DefaultConfig.PointSize = 3.f;         
		DefaultConfig.ImageSize = 512;         
		BevRenderer->Initialize(DefaultConfig);
	}

	
	// 델리게이트 연결
	WireComponents();   

	GetWorld()->GetTimerManager().SetTimer(
		FlipCheckTimer, this, &ATeam7DTVehicle::FlippedCheck, FlipCheckTime, true);

	StartSimulation();
}

void ATeam7DTVehicle::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	StopSimulation();
	GetWorld()->GetTimerManager().ClearTimer(FlipCheckTimer);
	GetWorld()->GetTimerManager().ClearTimer(DriveStateTimer);
	Super::EndPlay(EndPlayReason);
}

void ATeam7DTVehicle::Tick(float Delta)
{
	Super::Tick(Delta);
	if (bSimulationRunning == true) 
		SimulationElapsedTime += Delta;

	GetMesh()->SetAngularDamping(ChaosVehicleMovement->IsMovingOnGround() ? 0.f : 3.f);

	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.f, Delta, 1.f);
	BackSpringArm->SetRelativeRotation(FRotator(0.f, CameraYaw, 0.f));
}

void ATeam7DTVehicle::FlippedCheck()
{
	const float UpDot = FVector::DotProduct(FVector::UpVector, GetMesh()->GetUpVector());
	if (UpDot >= FlipCheckMinDot)
	{
		bPreviousFlipCheck = false;
		return;
	}
	
	if (bPreviousFlipCheck == true)
		DoResetVehicle();
	
	bPreviousFlipCheck = true;
}

void ATeam7DTVehicle::WireComponents()
{
	if (LidarSensor && BevRenderer)
	    LidarSensor->OnPointCloudUpdated.AddUObject(BevRenderer, &ULidarBevRenderer::HandlePointCloud);
	
	if (DataLogger)
	{
	    OnDriveStateUpdated.AddUObject(
	        DataLogger, &UTeam7DTAgentDataLoggerComponent::HandleDriveState);
	}
	
	if (LidarSensor)
	{
	    OnSimulationStarted.AddUObject(LidarSensor, &ULidarSensorComponent::StartScan);
	    OnSimulationStopped.AddUObject(LidarSensor, &ULidarSensorComponent::StopScan);
	}
}

FDriveState ATeam7DTVehicle::BuildCurrentDriveState() const
{
	FDriveState S;
	S.Location  = GetActorLocation();
	S.Rotation  = GetActorRotation();
	S.SpeedKmh  = GetVelocity().Size() * 0.036f;
	S.Throttle  = CurrentThrottle;
	S.Steering  = CurrentSteering;
	S.Brake     = CurrentBrake;
	S.TimeStamp = SimulationElapsedTime;
	return S;
}

void ATeam7DTVehicle::ResetVehicleInput(const FInputActionValue&)
{
	DoResetVehicle();
}

void ATeam7DTVehicle::ToggleCameraInput(const FInputActionValue&)
{
	DoToggleCamera();
}

void ATeam7DTVehicle::ToggleSensorViewInput(const FInputActionValue&)
{
	DoToggleSensorView();
}

void ATeam7DTVehicle::ToggleLidarViewInput(const FInputActionValue&)
{
	DoToggleLidarView();
}

void ATeam7DTVehicle::DoResetVehicle()
{
	FVector  Loc = GetActorLocation() + FVector(0.f, 0.f, 50.f);
	FRotator Rot = GetActorRotation();
	Rot.Pitch = 0.f;
	Rot.Roll  = 0.f;

	SetActorTransform(FTransform(Rot, Loc, FVector::OneVector),
					  false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

void ATeam7DTVehicle::DoToggleCamera()
{
	bFrontCameraActive = !bFrontCameraActive;
	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ATeam7DTVehicle::DoToggleSensorView()
{
	auto* PC = Cast<ATeam7DTPlayerController>(GetController());
	if (!PC || !CameraSensor) return;

	UTextureRenderTarget2D* RT = CameraSensor->RenderTarget;
	PC->ToggleSensorView(RT);
}

void ATeam7DTVehicle::DoToggleLidarView()
{
	auto* PC = Cast<ATeam7DTPlayerController>(GetController());
	if (!PC || !BevRenderer) return;

	UTexture2D* LidarTex = BevRenderer->GetRenderTarget();
	PC->ToggleLidarView(LidarTex);

	// LiDAR가 안 보일 때는 스캔 정지 (성능 최적화)
	if (LidarSensor)
	{
		if (PC->IsLidarViewVisible())  LidarSensor->StartScan();
		else                            LidarSensor->StopScan();
	}
}
