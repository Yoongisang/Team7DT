// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Team7DTPlayerController.generated.h"

class UInputMappingContext;
class ATeam7DTVehicle;        
class USensorViewWidget;       
class UTextureRenderTarget2D;
class UTeam7DTUI;

UCLASS(abstract)
class TEAM7DT_API ATeam7DTPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// 센서뷰 토글 (Pawn에서 호출)
	UFUNCTION(BlueprintCallable, Category="UI")
	void ToggleSensorView(UTextureRenderTarget2D* CameraRT);

	UFUNCTION(BlueprintCallable, Category="UI")
	void ToggleLidarView(UTexture2D* LidarTex);

	UFUNCTION(BlueprintPure, Category="UI")
	bool IsLidarViewVisible() const;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	bool bUseSteeringWheelControls = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta=(EditCondition="bUseSteeringWheelControls"))
	UInputMappingContext* SteeringWheelInputMappingContext;

	/** Pointer to the controlled vehicle pawn */
	TObjectPtr<ATeam7DTVehicle> VehiclePawn;

	/** Type of the UI to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<UTeam7DTUI> VehicleUIClass;

	/** Pointer to the UI widget */
	TObjectPtr<UTeam7DTUI> VehicleUI;

	// 센서뷰 위젯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	TSubclassOf<USensorViewWidget> SensorViewWidgetClass;

	UPROPERTY()
	TObjectPtr<USensorViewWidget> SensorViewWidget;
	
protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	virtual void Tick(float Delta) override;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	
};
