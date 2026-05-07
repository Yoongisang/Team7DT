// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "DriveTypes.h" 
#include "Team7DTVehicle.generated.h"

class UCameraComponent;
class UChaosWheeledVehicleMovementComponent; 
class USpringArmComponent;
class UInputAction;
class USplineFollowerComponent;
class ULidarSensorComponent;
class UCameraSensorComponent;
class ULidarBevRenderer;
class UTeam7DTAgentDataLoggerComponent;
struct FInputActionValue;

DECLARE_MULTICAST_DELEGATE(FOnSimulationStarted);
DECLARE_MULTICAST_DELEGATE(FOnSimulationStopped);

UCLASS()
class TEAM7DT_API ATeam7DTVehicle : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	ATeam7DTVehicle();
	// 자동 주행 제어
	UFUNCTION(BlueprintCallable, Category = "AutoDrive")
	void DoSteering(float SteeringValue);
	UFUNCTION(BlueprintCallable, Category = "AutoDrive")
	void DoThrottle(float ThrottleValue);
	UFUNCTION(BlueprintCallable, Category = "AutoDrive")
	void DoBrake(float BrakeValue);
	UFUNCTION(BlueprintCallable, Category = "AutoDrive")
	void DoBrakeStart();
	UFUNCTION(BlueprintCallable, Category = "AutoDrive")
	void DoBrakeStop();

	// 시뮬 제어
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void StartSimulation();
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void StopSimulation();
	// Getter
	FORCEINLINE USplineFollowerComponent* GetSplineFollower() const { return SplineFollower; }
	
	FORCEINLINE ULidarSensorComponent* GetLidarSensor() const { return LidarSensor; }
	FORCEINLINE UCameraSensorComponent* GetCameraSensor() const { return CameraSensor; }
	FORCEINLINE ULidarBevRenderer* GetBevRenderer() const { return BevRenderer; }
	FORCEINLINE UTeam7DTAgentDataLoggerComponent* GetDataLogger() const { return DataLogger; }
	
	FORCEINLINE UCameraComponent* GetFrontCamera() const { return FrontCamera; }
	FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
	// 델리게이트
	FOnDriveStateUpdated OnDriveStateUpdated;
	FOnSimulationStarted OnSimulationStarted;
	FOnSimulationStopped OnSimulationStopped;
	
protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponentPlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float Delta) override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle")
	void BrakeLights(bool bBraking);
	// 넘어졌는지 체크
	UFUNCTION()
	void FlippedCheck();
	
private:
	void WireComponents();
	
	FDriveState BuildCurrentDriveState() const;
	// 입력 핸들러
	void ResetVehicleInput(const FInputActionValue&);
	void ToggleCameraInput(const FInputActionValue&);
	void ToggleSensorViewInput(const FInputActionValue&);
	void ToggleLidarViewInput(const FInputActionValue&);
	
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void DoResetVehicle();
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void DoToggleCamera();
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void DoToggleSensorView();
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void DoToggleLidarView();
	// 맴버 변수
	// 카메라 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USpringArmComponent> FrontSpringArm;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UCameraComponent> FrontCamera;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USpringArmComponent> BackSpringArm;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UCameraComponent> BackCamera;

    // 자동 주행 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USplineFollowerComponent> SplineFollower;

    // 센서
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<ULidarSensorComponent> LidarSensor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UCameraSensorComponent> CameraSensor;

    // BEV
    UPROPERTY()
    TObjectPtr<ULidarBevRenderer> BevRenderer;

    // 데이터 로거
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UTeam7DTAgentDataLoggerComponent> DataLogger;

    // 차량 무브먼트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

    // 입력 액션
    UPROPERTY(EditAnywhere, Category="Input") 
	TObjectPtr<UInputAction> ResetVehicleAction;
    UPROPERTY(EditAnywhere, Category="Input") 
	TObjectPtr<UInputAction> ToggleCameraAction;
    UPROPERTY(EditAnywhere, Category="Input") 
	TObjectPtr<UInputAction> ToggleSensorViewAction;
    UPROPERTY(EditAnywhere, Category="Input") 
	TObjectPtr<UInputAction> ToggleLidarViewAction;

    // Flip Check
    UPROPERTY(EditAnywhere, Category="FlipCheck", meta=(Units="s"))
    float FlipCheckTime = 3.f;

    UPROPERTY(EditAnywhere, Category="FlipCheck")
    float FlipCheckMinDot = -0.2f;

    // 주행 상태 발행 주기
    UPROPERTY(EditAnywhere, Category="DriveState", meta=(ClampMin="0.1", ClampMax="60.0", Units="Hz"))
    float DriveStateBroadcastHz = 10.f;

    FTimerHandle FlipCheckTimer;
    FTimerHandle DriveStateTimer;

    bool bFrontCameraActive = false;
    bool bPreviousFlipCheck = false;
    bool bSimulationRunning = false;

    float SimulationElapsedTime = 0.f;

    float CurrentSteering = 0.f;
    float CurrentThrottle = 0.f;
    float CurrentBrake    = 0.f;
};
