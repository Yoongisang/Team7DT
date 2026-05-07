#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraSensorComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UENUM(BlueprintType)
enum class ECameraSensorPreset : uint8
{
    Wide_120FOV UMETA(DisplayName = "Wide 120 FOV"),
    Tele_30FOV UMETA(DisplayName = "Tele 30 FOV")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEAM7DT_API UCameraSensorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCameraSensorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction
    ) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Sensor")
    USceneCaptureComponent2D* SceneCapture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Sensor")
    UTextureRenderTarget2D* RenderTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Sensor")
    ECameraSensorPreset CameraPreset = ECameraSensorPreset::Wide_120FOV;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distortion")
    float K1 = 0.3f;

    UFUNCTION(BlueprintCallable, Category = "Camera Sensor")
    void ApplyCameraPreset();
};