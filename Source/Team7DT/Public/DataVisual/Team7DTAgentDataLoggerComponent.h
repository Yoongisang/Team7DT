#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DriveTypes.h" 
#include "Team7DTAgentDataLoggerComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAM7DT_API UTeam7DTAgentDataLoggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeam7DTAgentDataLoggerComponent();
	
	UFUNCTION()
	void HandleDriveState(const FDriveState& State);
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
private:
	static int32 GetUtmZone(double Longitude);
	static void LatLonToUtm(double Lat, double Lon, int32 Zone, double& OutEasting, double& OutNorthing);
	void WorldToUtm(const FVector& WorldLocation, double& OutEasting, double& OutNorthing) const;
	void CreateCsvFile();

	UFUNCTION(BlueprintCallable, Category = "Data Logger")
	void StartRecording();

	UFUNCTION(BlueprintCallable, Category = "Data Logger")
	void StopRecording();

	UFUNCTION(BlueprintPure, Category = "Data Logger")
	bool IsRecording() const { return bIsRecording; }

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger",
		meta = (AllowPrivateAccess = "true"))
	bool bEnableLogging = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|UTM Reference",
		meta = (ClampMin = "-90.0", ClampMax = "90.0", Units = "deg", AllowPrivateAccess = "true"))
	double OriginLatitude = 36.4800;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|UTM Reference",
		meta = (ClampMin = "-180.0", ClampMax = "180.0", Units = "deg", AllowPrivateAccess = "true"))
	double OriginLongitude = 127.0000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Logger|Max Speed", meta = (AllowPrivateAccess = "true"))
	float MaxSpeedForDebug = 150.f; //km/h

protected:
	float Alpha = 0.f;

private:
	FVector LastLocation = FVector::ZeroVector;
		
	FLinearColor Green = FLinearColor::Green;
	FLinearColor Red = FLinearColor::Red;

private:
	double OriginUtmEasting = 0.0;
	double OriginUtmNorthing = 0.0;
	int32 OriginUtmZone = 0;

	FString CsvFilePath;
	bool bIsRecording = false;
	
	// 가속도 계산용
	float PreviousSpeedKmh   = 0.f;
	float PreviousTimeStamp  = 0.f;

	// 급감속 임계값 (km/h/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data Logger|Visualization", meta=(AllowPrivateAccess="true"))
	float HardBrakeThreshold = -50.f;

	// DrawDebugString 표시 간격 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data Logger|Visualization", meta=(AllowPrivateAccess="true"))
	float DebugStringInterval = 500.f;

	FVector LastDebugStringLocation = FVector::ZeroVector;
};
