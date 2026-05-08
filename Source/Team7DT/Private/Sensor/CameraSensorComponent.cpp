#include "Sensor/CameraSensorComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Actor.h"

UCameraSensorComponent::UCameraSensorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
}

void UCameraSensorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        SceneCapture->RegisterComponent();
        SceneCapture->AttachToComponent(
            Owner->GetRootComponent(),
            FAttachmentTransformRules::KeepRelativeTransform
        );

        SceneCapture->SetRelativeLocation(FVector(300.0f, 0.0f, 120.0f));
        SceneCapture->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    }

    ApplyCameraPreset();

    UE_LOG(LogTemp, Warning, TEXT("Camera Sensor Start"));
}

void UCameraSensorComponent::ApplyCameraPreset()
{
    if (!SceneCapture) return;

    switch (CameraPreset)
    {
    case ECameraSensorPreset::Wide_120FOV:

        SceneCapture->FOVAngle = 120.0f;

        if (RenderTarget)
        {
            RenderTarget->ResizeTarget(1920, 1080);
        }

        break;

    case ECameraSensorPreset::Tele_30FOV:

        SceneCapture->FOVAngle = 15.0f;

        if (RenderTarget)
        {
            RenderTarget->ResizeTarget(1280, 720);
        }

        break;
    }

    SceneCapture->TextureTarget = RenderTarget;

    if (SceneCapture->PostProcessSettings.WeightedBlendables.Array.Num() > 0)
    {
        if (UMaterialInstanceDynamic* DynMat =
            Cast<UMaterialInstanceDynamic>(
                SceneCapture->PostProcessSettings.WeightedBlendables.Array[0].Object))
        {
            DynMat->SetScalarParameterValue(TEXT("K1"), K1);
        }
    }
}

void UCameraSensorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}