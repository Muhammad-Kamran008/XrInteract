#include "MetaverseXRInputManager.h"
#include "Kismet/GameplayStatics.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "ControllerTracking.h"
#include "HanTracking.h"
#include "IXRTrackingSystem.h"
#include "XRTrackingSystemBase.h"

AMetaverseXRInputManager::AMetaverseXRInputManager()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentMode = EXRInputMode::HandTracking;

}

void AMetaverseXRInputManager::BeginPlay()
{
	Super::BeginPlay();

	HandTrackingActor = GetWorld()->SpawnActor<AHanTracking>(
		AHanTracking::StaticClass(),
		FVector(0, 0, 0),
		FRotator(0, 0, 0));
	ControllerTrackingActor = GetWorld()->SpawnActor<AControllerTracking>(
		AControllerTracking::StaticClass(),
		FVector(0, 0, 0),
		FRotator(0, 0, 0));
	SetXRMode(EXRInputMode::HandTracking);

}

void AMetaverseXRInputManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckXRMode();
}

void AMetaverseXRInputManager::CheckXRMode()
{
	bControllersConnected = AreControllersConnected();

	EXRInputMode DesiredMode = bControllersConnected
		? EXRInputMode::ControllerTracking
		: EXRInputMode::HandTracking;

	if (DesiredMode != CurrentMode)
		SetXRMode(DesiredMode);
}

void AMetaverseXRInputManager::SetXRMode(EXRInputMode InputMode)
{
	CurrentMode = InputMode;

	switch (InputMode)
	{
	case EXRInputMode::HandTracking:
		if (HandTrackingActor) {
			HandTrackingActor->SetActorHiddenInGame(false);
			HandTrackingActor->SetActorTickEnabled(true);
			HandTrackingActor->EnableInput(nullptr);
		}
		if (ControllerTrackingActor) {
			ControllerTrackingActor->SetActorHiddenInGame(true);
			ControllerTrackingActor->SetActorTickEnabled(false);
		}
		break;
	case EXRInputMode::ControllerTracking:
		if (HandTrackingActor) {
			HandTrackingActor->SetActorHiddenInGame(true);
			HandTrackingActor->SetActorTickEnabled(false);
			HandTrackingActor->EnableInput(nullptr);
		}
		if (ControllerTrackingActor) {
			ControllerTrackingActor->SetActorHiddenInGame(false);
			ControllerTrackingActor->SetActorTickEnabled(true);
		}
		break;
	}
}

bool AMetaverseXRInputManager::AreControllersConnected() {
	if (GEngine && GEngine->XRSystem.IsValid()) {
		XRSystem = GEngine->XRSystem.Get();
		XRSystem->EnumerateTrackedDevices(DevicesIds, EXRTrackedDeviceType::Controller);
		return DevicesIds.Num() > 0;
	}

	return false;
}