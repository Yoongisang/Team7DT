#include "DataVisual/Team7DTAgentDataLoggerComponent.h"
#include "DrawDebugHelpers.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UTeam7DTAgentDataLoggerComponent::UTeam7DTAgentDataLoggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UTeam7DTAgentDataLoggerComponent::HandleDriveState(const FDriveState& State)
{
	if (bIsRecording == false) 
		return;
	// 가속도 계산
	float Acceleration = 0.f;
	if (PreviousTimeStamp > 0.f)
	{
		const float Dt = State.TimeStamp - PreviousTimeStamp;
		if (Dt > KINDA_SMALL_NUMBER)
			Acceleration = (State.SpeedKmh - PreviousSpeedKmh) / Dt;
	}
	PreviousSpeedKmh  = State.SpeedKmh;
	PreviousTimeStamp = State.TimeStamp;
	
	// UTM 변환
	double UtmEasting = 0.0;
	double UtmNorthing = 0.0;
	WorldToUtm(State.Location, UtmEasting, UtmNorthing);

	// CSV append
	const FString Row = FString::Printf(
		TEXT("%.3f,%.2f,%.2f,%.2f,%.4f,%.4f,%d,%.2f,%.4f,%.3f,%.3f,%.3f,%.2f\n"),
		State.TimeStamp,
		State.Location.X, State.Location.Y, State.Location.Z,
		UtmEasting, UtmNorthing, OriginUtmZone,
		State.SpeedKmh,
		State.Rotation.Yaw,
		State.Throttle, State.Steering, State.Brake,
		Acceleration  
	);

	FFileHelper::SaveStringToFile(Row, *CsvFilePath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
		&IFileManager::Get(),
		EFileWrite::FILEWRITE_Append);
	
	// 트래젝토리 라인 (속도 색)
	Alpha = FMath::Clamp(State.SpeedKmh / MaxSpeedForDebug, 0.f, 1.f);
	const FColor LineColor = FLinearColor::LerpUsingHSV(Green, Red, Alpha).ToFColor(true);
	DrawDebugLine(GetWorld(), LastLocation, State.Location, LineColor, false, 10.f, 0, 2.f);

	// 급감속 지점 표시
	if (Acceleration < HardBrakeThreshold)
	{
		DrawDebugPoint(GetWorld(), State.Location,
			20.f, FColor::Magenta, false, 30.f);
	}

	// DrawDebugString (일정 거리마다 속도/yaw)
	const float DistMoved = FVector::Dist(LastDebugStringLocation, State.Location);
	if (DistMoved >= DebugStringInterval)
	{
		const FString Info = FString::Printf(
			TEXT("%.1f km/h\nYaw: %.1f°"),
			State.SpeedKmh, State.Rotation.Yaw);

		DrawDebugString(GetWorld(),
			State.Location + FVector(0, 0, 100),
			Info, nullptr, FColor::White, 30.f, false, 1.f);

		LastDebugStringLocation = State.Location;
	}

	LastLocation = State.Location;
}


void UTeam7DTAgentDataLoggerComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		LastLocation = OwnerPawn->GetActorLocation();
	}

	OriginUtmZone = GetUtmZone(OriginLongitude);
	LatLonToUtm(OriginLatitude, OriginLongitude, OriginUtmZone, OriginUtmEasting, OriginUtmNorthing);

	if (bEnableLogging)
	{
		StartRecording();
	}
}

void UTeam7DTAgentDataLoggerComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	StopRecording();
	Super::EndPlay(EndPlayReason);
}

int32 UTeam7DTAgentDataLoggerComponent::GetUtmZone(double Longitude)
{
	return FMath::FloorToInt((Longitude + 180.0) / 6.0) + 1;
}

void UTeam7DTAgentDataLoggerComponent::LatLonToUtm(double Lat, double Lon, int32 Zone, double& OutEasting, double& OutNorthing)
{
	// WGS-84 ellipsoid constants
	constexpr double a = 6378137.0;            // semi-major axis (m)
	constexpr double f = 1.0 / 298.257223563;  // flattening
	constexpr double k0 = 0.9996;               // UTM scale factor

	const double e2 = 2.0 * f - f * f;          // first eccentricity squared
	const double ep2 = e2 / (1.0 - e2);         // second eccentricity squared

	const double LatRad = FMath::DegreesToRadians(Lat);
	const double CentralMeridian = (Zone - 1) * 6.0 - 180.0 + 3.0;
	const double DeltaLon = FMath::DegreesToRadians(Lon - CentralMeridian);

	const double SinLat = FMath::Sin(LatRad);
	const double CosLat = FMath::Cos(LatRad);
	const double TanLat = FMath::Tan(LatRad);

	const double N = a / FMath::Sqrt(1.0 - e2 * SinLat * SinLat);
	const double T = TanLat * TanLat;
	const double C = ep2 * CosLat * CosLat;
	const double A = CosLat * DeltaLon;

	// Meridional arc (M) — series expansion
	const double e4 = e2 * e2;
	const double e6 = e4 * e2;
	const double M = a * (
		(1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0) * LatRad
		- (3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0) * FMath::Sin(2.0 * LatRad)
		+ (15.0 * e4 / 256.0 + 45.0 * e6 / 1024.0) * FMath::Sin(4.0 * LatRad)
		- (35.0 * e6 / 3072.0) * FMath::Sin(6.0 * LatRad));

	const double A2 = A * A;
	const double A4 = A2 * A2;
	const double A6 = A4 * A2;

	OutEasting = k0 * N * (
		A
		+ (1.0 - T + C) * A2 * A / 6.0
		+ (5.0 - 18.0 * T + T * T + 72.0 * C - 58.0 * ep2) * A4 * A / 120.0
		) + 500000.0;   // false easting

	OutNorthing = k0 * (M + N * TanLat * (
		A2 / 2.0
		+ (5.0 - T + 9.0 * C + 4.0 * C * C) * A4 / 24.0
		+ (61.0 - 58.0 * T + T * T + 600.0 * C - 330.0 * ep2) * A6 / 720.0
		));

	// Southern hemisphere offset
	if (Lat < 0.0)
		OutNorthing += 10000000.0;
}

void UTeam7DTAgentDataLoggerComponent::WorldToUtm(const FVector& WorldLocation, double& OutEasting, double& OutNorthing) const
{
	const double OffsetEastM = WorldLocation.X * 0.01;
	const double OffsetNorthM = -WorldLocation.Y * 0.01;

	OutEasting = OriginUtmEasting + OffsetEastM;
	OutNorthing = OriginUtmNorthing + OffsetNorthM;
}

void UTeam7DTAgentDataLoggerComponent::StartRecording()
{
	if (bIsRecording)
	{
		return;
	}

	CreateCsvFile();
	bIsRecording = true;
}

void UTeam7DTAgentDataLoggerComponent::StopRecording()
{
	bIsRecording = false;
}

void UTeam7DTAgentDataLoggerComponent::CreateCsvFile()
{
	const FString OutputDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Output"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*OutputDir);

	const FDateTime Now = FDateTime::Now();
	const FString FileName = FString::Printf(
		TEXT("AgentData-%04d_%02d-%02d-%02d-%02d-%02d.csv"),
		Now.GetYear(), Now.GetMonth(), Now.GetDay(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond()
	);

	CsvFilePath = FPaths::Combine(OutputDir, FileName);

	const FString Header =
		TEXT("Timestamp,World_X,World_Y,World_Z,UTM_Easting,UTM_Northing,UTM_Zone,"
			 "Velocity_kmh,Yaw,Throttle,Steering,Brake,Acceleration_kmh_per_s\n");
	
	FFileHelper::SaveStringToFile(Header, *CsvFilePath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM
	);

	UE_LOG(LogTemp, Log, TEXT("[AgentDataLogger] Recording to: %s"), *CsvFilePath);
}
