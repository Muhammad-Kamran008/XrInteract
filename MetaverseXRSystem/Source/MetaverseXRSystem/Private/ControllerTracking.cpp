#include "ControllerTracking.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IXRTrackingSystem.h"
#include "XRTrackingSystemBase.h"

AControllerTracking::AControllerTracking()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetupAttachment(RootComponent);
	LeftController->SetTrackingSource(EControllerHand::Left);

	LeftControllerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftControllerMesh"));
	LeftControllerMesh->SetupAttachment(LeftController);
	LeftControllerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftControllerMesh->SetVisibility(false);

	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	RightController->SetupAttachment(RootComponent);
	RightController->SetTrackingSource(EControllerHand::Right);

	RightControllerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightControllerMesh"));
	RightControllerMesh->SetupAttachment(LeftController);
	RightControllerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightControllerMesh->SetVisibility(false);

	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	RightMotionController->SetTrackingSource(EControllerHand::Right);
	RightControllerMesh->SetupAttachment(RightMotionController);

	LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	LeftMotionController->SetTrackingSource(EControllerHand::Left);
	LeftControllerMesh->SetupAttachment(LeftMotionController);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ControllerMeshAsset(TEXT("/MetaverseXRSystem/ControllerMesh/Controller/scene"));
	if (ControllerMeshAsset.Succeeded()) {
		LeftControllerMesh->SetStaticMesh(ControllerMeshAsset.Object);
		RightControllerMesh->SetStaticMesh(ControllerMeshAsset.Object);
	}

	bIsControllerConnected = false;

}

void AControllerTracking::BeginPlay()
{
	Super::BeginPlay();
	UpdateConnectionState();

}

void AControllerTracking::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateConnectionState();
}

bool AControllerTracking::AreControllersConnected()
{
	if (GEngine && GEngine->XRSystem.IsValid()) {
		XRSystem = GEngine->XRSystem.Get();
		XRSystem->EnumerateTrackedDevices(DevicesIds, EXRTrackedDeviceType::Controller);
		return DevicesIds.Num() > 0;
	}

	return false;
}

void AControllerTracking::SetControllersActive(bool bActivate)
{
	LeftController->SetVisibility(bActivate, true);
	RightController->SetVisibility(bActivate, true);

	LeftControllerMesh->SetVisibility(bActivate, true);
	RightControllerMesh->SetVisibility(bActivate, true);

	SetActorTickEnabled(true);

}

void AControllerTracking::UpdateConnectionState()
{
	bIsCurrentlyConnected = AreControllersConnected();
	if (bIsCurrentlyConnected != bIsControllerConnected) {
		bIsControllerConnected = bIsCurrentlyConnected;
		SetControllersActive(bIsControllerConnected);
	}
}

void AControllerTracking::InitializeMeshes()
{
	if (DefaultLeftMesh) {
		LeftControllerMesh->SetStaticMesh(DefaultLeftMesh);
	}
	if (DefaultRightMesh) {
		RightControllerMesh->SetStaticMesh(DefaultRightMesh);
	}
}

void AControllerTracking::PlayHaptic(UHapticFeedbackEffect_Base* Effect, EControllerHand Hand)
{
	PC = GetWorld()->GetFirstPlayerController();

	if (PC) {
		PC->PlayHapticEffect(Effect, Hand);
	}
}

