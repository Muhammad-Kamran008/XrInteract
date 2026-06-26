#include "HanTracking.h"
#include "IHandTracker.h"
#include "HeadMountedDisplayTypes.h"
#include "IHeadMountedDisplay.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include <Kismet/KismetMathLibrary.h>
#include "Camera/CameraComponent.h"
#include "TimerManager.h"           
#include "Materials/MaterialInstanceDynamic.h"  
#include "Features/IModularFeatures.h"
#include "IMotionController.h"  
#include "UObject/ConstructorHelpers.h"  
#include "Engine/StaticMesh.h"           
#include "Materials/MaterialInterface.h" 
#include "DrawDebugHelpers.h"              
#include "EngineUtils.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Modules/ModuleManager.h"
#include "UObject/ConstructorHelpers.h"
#include "OculusXRInputFunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/OutputDeviceDebug.h"


AHanTracking* AHanTracking::HandTrackingInstance = nullptr;

AHanTracking::AHanTracking()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Interaction_Collider = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	Interaction_Collider->SetupAttachment(RootComponent);
	Interaction_Collider->InitSphereRadius(1.0f);
	Interaction_Collider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Interaction_Collider->SetCollisionObjectType(ECC_WorldDynamic);
	Interaction_Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
	Interaction_Collider->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Interaction_Collider->ComponentTags.Add(FName("Index_Tip_Collider"));
	Interaction_Collider->SetGenerateOverlapEvents(true);

	TipSpheresL = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TipSpheres_Left"));
	TipSpheresR = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TipSpheres_Right"));


	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HandMatObj(
		TEXT("/MetaverseXRSystem/MixedReality/M_QuestHand.M_QuestHand")
	);

	if (HandMatObj.Succeeded())
	{
		QuestHandMaterial = HandMatObj.Object;
	}

	//LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	//LeftMotionController->SetupAttachment(RootComponent);
	//LeftMotionController->SetTrackingSource(EControllerHand::Left);
	//LeftMotionController->SetTrackingMotionSource("Left");


	//RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	//RightMotionController->SetupAttachment(RootComponent);
	//RightMotionController->SetTrackingSource(EControllerHand::Right);
	//RightMotionController->SetTrackingMotionSource("Right");


	//LeftHandComponent = CreateDefaultSubobject<UOculusXRHandComponent>(TEXT("LeftOculusXRHand"));
	//LeftHandComponent->SetupAttachment(LeftMotionController);
	//LeftHandComponent->SkeletonType = EOculusXRHandType::HandLeft;
	//LeftHandComponent->MeshType = EOculusXRHandType::HandLeft;
	//LeftHandComponent->ConfidenceBehavior = EOculusXRConfidenceBehavior::HideActor;
	//LeftHandComponent->SystemGestureBehavior = EOculusXRSystemGestureBehavior::SwapMaterial;
	//LeftHandComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	//RightHandComponent = CreateDefaultSubobject<UOculusXRHandComponent>(TEXT("RightOculusXRHand"));
	//RightHandComponent->SetupAttachment(RightMotionController);
	//RightHandComponent->SkeletonType = EOculusXRHandType::HandRight;
	//RightHandComponent->MeshType = EOculusXRHandType::HandRight;
	//RightHandComponent->ConfidenceBehavior = EOculusXRConfidenceBehavior::HideActor;
	//RightHandComponent->SystemGestureBehavior = EOculusXRSystemGestureBehavior::SwapMaterial;
	//RightHandComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);



}

void AHanTracking::InitMetaQuestHandMeshes()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	APawn* ActivePawn = PC->GetPawn();
	if (!ActivePawn) return;

	USceneComponent* AttachRoot = ActivePawn->GetRootComponent();
	if (!AttachRoot) return;

	LeftMotionController = NewObject<UMotionControllerComponent>(
		ActivePawn,
		TEXT("LeftMotionController")
	);

	LeftMotionController->SetTrackingSource(EControllerHand::Left);
	LeftMotionController->SetTrackingMotionSource(FName("Left"));
	LeftMotionController->RegisterComponent();
	LeftMotionController->AttachToComponent(
		AttachRoot,
		FAttachmentTransformRules::KeepRelativeTransform
	);

	RightMotionController = NewObject<UMotionControllerComponent>(
		ActivePawn,
		TEXT("RightMotionController")
	);

	RightMotionController->SetTrackingSource(EControllerHand::Right);
	RightMotionController->SetTrackingMotionSource(FName("Right"));
	RightMotionController->RegisterComponent();
	RightMotionController->AttachToComponent(
		AttachRoot,
		FAttachmentTransformRules::KeepRelativeTransform
	);


	LeftHandComponent = NewObject<UOculusXRHandComponent>(
		ActivePawn,
		TEXT("LeftOculusXRHand")
	);

	LeftHandComponent->SkeletonType = EOculusXRHandType::HandLeft;
	LeftHandComponent->MeshType = EOculusXRHandType::HandLeft;
	LeftHandComponent->ConfidenceBehavior = EOculusXRConfidenceBehavior::HideActor;
	LeftHandComponent->SystemGestureBehavior = EOculusXRSystemGestureBehavior::SwapMaterial;
	LeftHandComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftHandComponent->RegisterComponent();
	LeftHandComponent->AttachToComponent(
		LeftMotionController,
		FAttachmentTransformRules::KeepRelativeTransform
	);

	LeftHandComponent->SetRelativeLocation(FVector(-5, 0, 0));

	if (QuestHandMaterial)
	{
		LeftHandComponent->MaterialOverride = QuestHandMaterial;
	}

	RightHandComponent = NewObject<UOculusXRHandComponent>(
		ActivePawn,
		TEXT("RightOculusXRHand")
	);

	RightHandComponent->SkeletonType = EOculusXRHandType::HandRight;
	RightHandComponent->MeshType = EOculusXRHandType::HandRight;
	RightHandComponent->ConfidenceBehavior = EOculusXRConfidenceBehavior::HideActor;
	RightHandComponent->SystemGestureBehavior = EOculusXRSystemGestureBehavior::SwapMaterial;
	RightHandComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightHandComponent->RegisterComponent();
	RightHandComponent->AttachToComponent(
		RightMotionController,
		FAttachmentTransformRules::KeepRelativeTransform
	);

	RightHandComponent->SetRelativeLocation(FVector(-5, 0, 0));


	if (QuestHandMaterial)
	{
	  RightHandComponent->MaterialOverride = QuestHandMaterial;
	}
}



void AHanTracking::StartQuestCasting()
{
#if PLATFORM_WINDOWS

	const FString BatFile = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectContentDir() / TEXT("QuestCasting/start_quest_usb_casting.bat")
	);

	if (!FPaths::FileExists(BatFile))
	{
		UE_LOG(LogTemp, Error, TEXT("Batch file not found: %s"), *BatFile);
		return;
	}

	const FString CmdExe = TEXT("C:/Windows/System32/cmd.exe");
	const FString WorkingDir = FPaths::GetPath(BatFile);

	// /S /C preserves quoted command behavior correctly.
	const FString Params = FString::Printf(
		TEXT("/S /C \"\"%s\"\""),
		*BatFile
	);

	uint32 ProcessID = 0;

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(
		*CmdExe,
		*Params,
		true,       // bLaunchDetached
		false,      // bLaunchHidden
		false,      // bLaunchReallyHidden
		&ProcessID,
		0,
		*WorkingDir,
		nullptr,
		nullptr
	);

	if (!ProcHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to launch cmd.exe for batch file."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Quest casting batch launched. PID: %u"), ProcessID);
	FPlatformProcess::CloseProc(ProcHandle);

#else
	UE_LOG(LogTemp, Error, TEXT("Quest casting batch launch is Windows-only."));
#endif
}


bool AHanTracking::IsMetaQuestHMD() const
{
	const FString HMDName = UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName().ToString();

	return HMDName.Contains(TEXT("Oculus"), ESearchCase::IgnoreCase) ||
		HMDName.Contains(TEXT("Meta"), ESearchCase::IgnoreCase) ||
		HMDName.Contains(TEXT("Quest"), ESearchCase::IgnoreCase);
}



void AHanTracking::BeginPlay()
{
	Super::BeginPlay();

	SetUpPostProcessVolume();
	if (HandTrackingInstance == nullptr)
	{
		HandTrackingInstance = this;
	}
	PlayerControllerRef = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FActorSpawnParameters SpawnParams;
	CamManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayConnected())
	{
		UHeadMountedDisplayFunctionLibrary::EnableHMD(true);
		PlayerControllerRef->GetPawn()->FindComponentByClass<UCameraComponent>()->bLockToHmd = true;

	}
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::LocalFloor);

	if (IsMetaQuestHMD()) {
		StartQuestCasting();
		InitMetaQuestHandMeshes();
	}

	else {
		LeftBoneVisualizerActor = GetWorld()->SpawnActor<ABoneVisualizer>(
			ABoneVisualizer::StaticClass(),
			FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		RightBoneVisualizerActor = GetWorld()->SpawnActor<ABoneVisualizer>(
			ABoneVisualizer::StaticClass(),
			FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	}
}


void AHanTracking::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//RightHandComponent->SetWorldScale3D(RightHandComponentSize);
	//LeftHandComponent->SetWorldScale3D(LeftHandComponentSize);

	DrawHandsMesh();
	UpdateMetaQuestHandVisibility();
	DrawPinchRay(DeltaSeconds);

}


void AHanTracking::UpdateMetaQuestHandVisibility()
{
	const bool bHMDEnabled = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();

	if (LeftHandComponent)
	{
		LeftHandComponent->SetVisibility(bHMDEnabled && bL, true);
	}

	if (RightHandComponent)
	{
		RightHandComponent->SetVisibility(bHMDEnabled && bR, true);
	}
}



bool AHanTracking::IsPointing(
	const TArray<FVector>& Positions,
	const TArray<FQuat>& Rotations,
	float TipThreshold,    // how far index tip must be from palm
	float CurlThreshold   // how close other fingers must be to palm
) const
{
	int32 PalmIndex = (int32)EHandKeypoint::Palm;
	int32 IndexTip = (int32)EHandKeypoint::IndexTip;
	int32 MiddleTip = (int32)EHandKeypoint::MiddleTip;
	int32 RingTip = (int32)EHandKeypoint::RingTip;
	int32 LittleTip = (int32)EHandKeypoint::LittleTip;

	if (!Positions.IsValidIndex(PalmIndex) ||
		!Positions.IsValidIndex(IndexTip) ||
		!Positions.IsValidIndex(MiddleTip) ||
		!Positions.IsValidIndex(RingTip) ||
		!Positions.IsValidIndex(LittleTip))
	{
		return false;
	}
	float IndexDist = FVector::Dist(Positions[IndexTip], Positions[PalmIndex]);
	float MiddleDist = FVector::Dist(Positions[MiddleTip], Positions[PalmIndex]);
	float RingDist = FVector::Dist(Positions[RingTip], Positions[PalmIndex]);
	float LittleDist = FVector::Dist(Positions[LittleTip], Positions[PalmIndex]);

	bool bIndexExtended = (IndexDist > TipThreshold);
	bool bOthersCurled = (MiddleDist < CurlThreshold &&
		RingDist < CurlThreshold &&
		LittleDist < CurlThreshold);

	if (!(bIndexExtended && bOthersCurled))
	{

		return false;
	}
	FVector PointingDir = (Positions[IndexTip] - Positions[PalmIndex]).GetSafeNormal();

	if (!CamManager) return false;

	FVector CameraForward = CamManager->GetActorForwardVector();

	float Dot = FVector::DotProduct(PointingDir, CameraForward);

	return (Dot > 0.7f);
}




void AHanTracking::DrawHandsMesh()
{
	bL = GetHandKeypoints(EControllerHand::Left, LP, LR, LRd);
	bR = GetHandKeypoints(EControllerHand::Right, RP, RR, RRd);

	const int32 IndexTipIndex = static_cast<int32>(EHandKeypoint::IndexTip);

	if (bL)
	{
		if (LP.IsValidIndex(IndexTipIndex))
		{
			Interaction_Collider->SetWorldLocation(LP[IndexTipIndex]);
		}

		if (LeftHandComponent)
		{
			LeftHandComponent->SetHiddenInGame(false);
			LeftHandComponent->SetVisibility(true, true);
		}
		if (LeftBoneVisualizerActor) {
			LeftBoneVisualizerActor->SetActorHiddenInGame(false);
			LeftBoneVisualizerActor->UpdateBones(LP);
		}
	}
	else
	{
		HideTipSpheres(EControllerHand::Left);

		if (LeftHandComponent)
		{
			LeftHandComponent->SetHiddenInGame(true);
			LeftHandComponent->SetVisibility(false, true);
		}
		if (LeftBoneVisualizerActor) {
			TipSpheresL->SetVisibility(false);
			HideTipSpheres(EControllerHand::Left);
			LeftBoneVisualizerActor->SetActorHiddenInGame(true);
		}
	}

	if (bR)
	{
		if (RightHandComponent)
		{
			RightHandComponent->SetHiddenInGame(false);
			RightHandComponent->SetVisibility(true, true);
		}
		if (RightBoneVisualizerActor) {
			RightBoneVisualizerActor->SetActorHiddenInGame(false);
			RightBoneVisualizerActor->UpdateBones(RP);
		}
	}
	else
	{
		if (RightHandComponent)
		{
			RightHandComponent->SetHiddenInGame(true);
			RightHandComponent->SetVisibility(false, true);
		}
		if (RightBoneVisualizerActor) {
			RightBoneVisualizerActor->SetActorHiddenInGame(true);
		}
	}
}

void AHanTracking::DrawPinchRay(float DeltaSeconds)
{
	bL = GetHandKeypoints(EControllerHand::Left, LP, LR, LRd);
	bR = GetHandKeypoints(EControllerHand::Right, RP, RR, RRd);

	const int32 IndexTipIndex = static_cast<int32>(EHandKeypoint::IndexTip);

	// LEFT HAND
	bLPinch = IsPinching(LP);
	if (bL && LP.IsValidIndex(IndexTipIndex))
	{
		const FVector LeftIndexTipPos = LP[IndexTipIndex];

		// Use hand keypoint as AimPosition, controller state as palm/base
		UpdateRaycastFromAim(
			EControllerHand::Left,
			ControllerDataLeft,      // FXRMotionControllerState
			LeftIndexTipPos,
			bLPinch
		);

		//  UpdatePinchRay(EControllerHand::Left, LP, LR, bLPinch, DeltaSeconds);
	}
	else
	{
		LeftPinch.bActive = false;
		LeftPinch.HitActor = nullptr;
	}

	// RIGHT HAND
	bRPinch = IsPinching(RP);
	if (bR && RP.IsValidIndex(IndexTipIndex))
	{
		const FVector RightIndexTipPos = RP[IndexTipIndex];

		//  UpdatePinchRay(EControllerHand::Right, RP, RR, bRPinch, DeltaSeconds);
		UpdateTipSpheresFor(RP, EControllerHand::Right);

		// FIX: use Right hand here, and pass the index tip as AimPosition
		UpdateRaycastFromAim(
			EControllerHand::Right,
			ControllerDataRight,     // FXRMotionControllerState
			RightIndexTipPos,
			bRPinch
		);
	}
	else
	{
		RightPinch.bActive = false;
		RightPinch.HitActor = nullptr;
	}
}


void AHanTracking::InitTipSpheresFor(EControllerHand Hand)
{
	UInstancedStaticMeshComponent* Comp = (Hand == EControllerHand::Left) ? TipSpheresL : TipSpheresR;
	TArray<int32>& Indices = (Hand == EControllerHand::Left) ? TipInstanceIdxL : TipInstanceIdxR;

	if (!Comp) return;

	const int32 First = (int32)EHandKeypoint::Palm;
	const int32 Last = (int32)EHandKeypoint::LittleTip;

	Indices.SetNum(Last - First + 1);
	Comp->ClearInstances();

	const float Scale = FMath::Max(0.001f, TipSphereRadiusCm / 50.f); // engine sphere ~50cm radius at (1,1,1)
	for (int32 i = First; i <= Last; ++i)
	{
		const FTransform T(FQuat::Identity, FVector::ZeroVector, FVector(Scale));
		const int32 Idx = Comp->AddInstance(T);
		Indices[i - First] = Idx;
	}

	Comp->MarkRenderStateDirty();
	Comp->SetVisibility(true, true);
}

void AHanTracking::InitTipSpheresBoth()
{
	InitTipSpheresFor(EControllerHand::Left);
	InitTipSpheresFor(EControllerHand::Right);
}

void AHanTracking::UpdateTipSpheresFor(const TArray<FVector>& Positions, EControllerHand Hand)
{
	UInstancedStaticMeshComponent* Comp = (Hand == EControllerHand::Left) ? TipSpheresL : TipSpheresR;
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/MetaverseXRSystem/MixedReality/M_BoneMaterial.M_BoneMaterial"));
	if (BaseMat) {
		UMaterialInstanceDynamic* DyeMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
		Comp->SetMaterial(0, BaseMat);
	}
	const TArray<int32>& Indices = (Hand == EControllerHand::Left) ? TipInstanceIdxL : TipInstanceIdxR;

	if (!Comp) return;

	const int32 First = (int32)EHandKeypoint::Palm;
	const int32 Last = (int32)EHandKeypoint::LittleTip;

	for (int32 kp = First; kp <= Last; ++kp)
	{
		const int32 local = kp - First;
		const int32 instance = Indices.IsValidIndex(local) ? Indices[local] : -1;
		if (instance < 0) continue;

		if (Positions.IsValidIndex(kp))
		{
			const FVector WorldPos = Positions[kp];

			FTransform Xf;
			Comp->GetInstanceTransform(instance, Xf, /*bWorldSpace*/ true);
			Xf.SetLocation(WorldPos);

			// keep your visual size consistent (or remove this to keep Init scale)
			Xf.SetScale3D(FVector(0.005f));
			Comp->UpdateInstanceTransform(instance, Xf, /*bWorldSpace*/ true, /*bMarkRenderStateDirty*/ true, /*bTeleport*/ true);
		}
	}

	Comp->MarkRenderStateDirty();
}

// Convenience: update both
void AHanTracking::UpdateTipSpheresBoth(const TArray<FVector>& LeftPositions,

	const TArray<FVector>& RightPositions)
{
	UpdateTipSpheresFor(LeftPositions, EControllerHand::Left);
	UpdateTipSpheresFor(RightPositions, EControllerHand::Right);
}


void AHanTracking::HideTipSpheres(EControllerHand Hand)
{
	if (Hand == EControllerHand::Left)
	{
		TipSpheresL->SetVisibility(false, true);
	}
	if (Hand == EControllerHand::Right)
	{
		TipSpheresR->SetVisibility(false, true);
	}
}

bool AHanTracking::IsPalmFacingCamera(const TArray<FQuat>& Rotations, const TArray<FVector>& Locations) const
{
	int32 PalmIndex = (int32)EHandKeypoint::Palm;
	if (!Rotations.IsValidIndex(PalmIndex) && !Locations.IsValidIndex(PalmIndex))
		return false;

	FQuat PalmRot = Rotations[PalmIndex];
	FVector PalmLoc = Locations[PalmIndex];
	FVector PalmNormal = PalmRot.GetUpVector();


	if (!CamManager) return false;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (!PC) return false;

	FVector CameraLoc = CamManager->GetCameraLocation();
	FVector ToCamera = (CameraLoc - PalmLoc).GetSafeNormal();

	float Dot = FVector::DotProduct(PalmNormal, ToCamera);
	if (Dot > 0.4)
		return false;

	int32 ViewportX, ViewportY;
	PC->GetViewportSize(ViewportX, ViewportY);

	FVector2D ScreenPos;
	bool bProjected = PC->ProjectWorldLocationToScreen(PalmLoc, ScreenPos);
	if (!bProjected) return false;

	if (ScreenPos.X < 0 || ScreenPos.X > ViewportX ||
		ScreenPos.Y < 0 || ScreenPos.Y > ViewportY)
		return false;

	return true;
}

static IHandTracker* GetHandTracker()
{
	const FName Feature = IHandTracker::GetModularFeatureName();
	if (!IModularFeatures::Get().IsModularFeatureAvailable(Feature))
		return nullptr;

	const int32 Count = IModularFeatures::Get().GetModularFeatureImplementationCount(Feature);
	return (Count > 0)
		? static_cast<IHandTracker*>(IModularFeatures::Get().GetModularFeatureImplementation(Feature, 0))
		: nullptr;
}

bool AHanTracking::GetHandKeypoints(
	EControllerHand Hand,
	TArray<FVector>& OutPositions,
	TArray<FQuat>& OutRotations,
	TArray<float>& OutRadii)
{
	OutPositions.Reset();
	OutRotations.Reset();
	OutRadii.Reset();

	/*   if (!(GEngine && GEngine->XRSystem.IsValid()))
		   return false;*/

	IHandTracker* HT = GetHandTracker();
	if (!HT || !HT->IsHandTrackingStateValid())
		return false;
	bool outResult = false;
	const bool bOK = HT->GetAllKeypointStates(Hand, OutPositions, OutRotations, OutRadii, outResult);
	return bOK && OutPositions.Num() > 0 && OutRotations.Num() == OutPositions.Num();
}

bool AHanTracking::IsPinching(
	const TArray<FVector>& Positions,
	float PinchStartThresholdCm,
	float PinchReleaseThresholdCm
)
{
	const int32 ThumbTip = (int32)EHandKeypoint::ThumbTip;
	const int32 IndexTip = (int32)EHandKeypoint::IndexTip;

	if (!Positions.IsValidIndex(ThumbTip) || !Positions.IsValidIndex(IndexTip))
		return false;

	const float DistCm = FVector::Dist(Positions[ThumbTip], Positions[IndexTip]);

	if (DistCm <= PinchStartThresholdCm)
	{
		bPrevPinching = true;
		return true;
	}

	if (DistCm > PinchReleaseThresholdCm && bPrevPinching)
	{
		bPrevPinching = false;
		return false;
	}

	return bPrevPinching;
}

FVector AHanTracking::GetPinchMid(const TArray<FVector>& Positions) const
{
	const int32 ThumbTip = (int32)EHandKeypoint::ThumbTip;
	const int32 IndexTip = (int32)EHandKeypoint::IndexTip;
	if (!Positions.IsValidIndex(ThumbTip) || !Positions.IsValidIndex(IndexTip))
		return FVector::ZeroVector;
	return 0.5f * (Positions[ThumbTip] + Positions[IndexTip]);
}

void AHanTracking::UpdatePinchRay(EControllerHand Hand,
	const TArray<FVector>& Positions,
	const TArray<FQuat>& Rotations,
	bool bIsPinching,
	float DeltaSeconds)
{
	FPinchRayState& Ray = (Hand == EControllerHand::Left) ? LeftPinch : RightPinch;

	// 1) Detect pinch start/end and lock direction at start
	if (bIsPinching && !Ray.bActive)
	{
		Ray.bActive = true;
		Ray.HitActor = nullptr;
		Ray.Hit = FHitResult();

		// origin starts at pinch midpoint
		Ray.OriginWS = GetPinchMid(Positions);

		// lock a stable direction (camera forward) to avoid flicker
		const APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
		const FVector LockedDir = Cam ? Cam->GetActorForwardVector() : FVector::ForwardVector;
		Ray.DirWS = LockedDir.GetSafeNormal();
	}
	else if (!bIsPinching && Ray.bActive)
	{
		Ray.bActive = false;
		Ray.HitActor = nullptr;
		Ray.Hit = FHitResult();

		// If either hand released, stop any two-hand scaling latch
		if (TwoHandTargetComp.IsValid())
		{
			TwoHandTargetComp.Reset();
			TwoHandBaselineDist = 0.f;
		}
	}

	if (!Ray.bActive)
		return;


	// 2) Update origin (smoothed), keep direction locked (or optionally smooth to palm normal)
	const FVector PinchPos = GetPinchMid(Positions);
	Ray.OriginWS = LerpExp(Ray.OriginWS, PinchPos, RaySmoothHz, DeltaSeconds);

	// Optional: if you want to *slowly* follow palm normal instead of fully locked:
	if (!bLockDirOnPinchStart && Rotations.IsValidIndex((int32)EHandKeypoint::Palm)) {
		const FVector PalmDown = Rotations[(int32)EHandKeypoint::Palm].GetUpVector() * -1.f;
		Ray.DirWS = LerpExp(Ray.DirWS, PalmDown.GetSafeNormal(), 6.f, DeltaSeconds).GetSafeNormal();
	}

	// 3) Raycast
	const FVector Start = Ray.OriginWS;
	const FVector End = Start + Ray.DirWS * RayLength;

	FCollisionQueryParams QP(SCENE_QUERY_STAT(HandPinchRay), false, this);


	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QP);

	Ray.Hit = Hit;
	Ray.HitActor = bHit ? Hit.GetActor() : nullptr;
	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::MakeFromColorTemperature(5) : FColor::Blue, false, 0.f, 0, 0.4f);
	DrawDebugSphere(GetWorld(), Start, 1.2f, 8, FColor::White, false, 0.f, 0, 0.1f);
	if (bHit) DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.f, FColor::Cyan, false, 0.3f);
}

float AHanTracking::CalculateTwoHandSpeed(const TArray<FVector>& LeftP,
	const TArray<FVector>& RightP,
	bool bLeftPinch, bool bRightPinch,
	float DeltaSeconds)   // <-- use DeltaTime here for smooth scaling
{
	// Only calculate movement if both pinches are active
	static float TwoHandBaseDistCm = -1.f;
	if (!(bLeftPinch && bRightPinch))
	{
		bIsMoving = false;
		currentSpeed = 0.f;
		TwoHandBaseDistCm = -1.f;

		return 0.f;
	}

	// On gesture start (when baseline is latched), also lock basis/rotation next tick
	if (TwoHandBaseDistCm == -1.f) {
		const FVector LPinch = GetPinchMid(LeftP);
		const FVector RPinch = GetPinchMid(RightP);
		TwoHandBaseDistCm = FVector::Dist(LPinch, RPinch);


		return 0.f;
	}

	// Get the current pinch positions and calculate the current distance between the two pinches
	const FVector LPinch = GetPinchMid(LeftP);
	const FVector RPinch = GetPinchMid(RightP);
	const float CurrentDist = FVector::Dist(LPinch, RPinch);
	if (CurrentDist <= KINDA_SMALL_NUMBER) return 0.f;

	// Calculate the change in distance from the initial baseline distance
	float deltaCm = CurrentDist - TwoHandBaseDistCm;

	// Deadzone: Ignore small changes to prevent jitter
	if (FMath::Abs(deltaCm) < TwoHandZoomDeadzoneCm)
		deltaCm = 0.f;
	else
		deltaCm -= FMath::Sign(deltaCm) * TwoHandZoomDeadzoneCm;

	// Apply exponential speed factor based on distance change

	float speed = FMath::Pow(FMath::Abs(deltaCm), 1.5f) * height;
	UE_LOG(LogTemp, Warning, TEXT("Height: %f"), height);// Exponential speed increase
	UE_LOG(LogTemp, Warning, TEXT("Speed: %f"), speed);
	// Direction: Move out if the distance is decreasing (pinching) and move towards if the distance is increasing
	if (deltaCm < 0)
	{
		// If pinch distance is decreasing, set the movement direction to outwards
		speed = -speed;  // Move **out** when the pinch distance decreases

		// Ensure that the speed is consistent while moving outwards, not stopping unless the pinch stops
		if (!bIsMoving)
		{
			bIsMoving = true;
			currentSpeed = speed;  // Start moving outwards with the calculated speed
		}
		else
		{
			// Continue applying the current speed while the pinch is still decreasing
			currentSpeed = speed;  // Maintain the same speed for outward movement
		}
	}
	else
	{
		// If pinch distance is increasing, move towards
		speed = speed;   // Move **towards** when the pinch distance increases

		// Ensure that the movement towards continues, adjusting speed as needed
		if (!bIsMoving)
		{
			bIsMoving = true;
			currentSpeed = speed;  // Start moving with calculated speed towards
		}
		else
		{
			// Keep applying the last calculated speed if pinch distance doesn't change
			if (deltaCm != 0)
			{
				currentSpeed = speed;  // Update speed if distance increases
			}
		}
	}

	// Return the calculated speed
	return currentSpeed;
}


bool AHanTracking::IsGrabbing(const TArray<FVector>& Positions, float GrabThreshold) const
{
	bool bIsGrabbing = true; // assume grabbing until proven false

	const int32 Palm = (int32)EHandKeypoint::Palm;
	const int32 IndexTip = (int32)EHandKeypoint::IndexTip;
	const int32 MiddleTip = (int32)EHandKeypoint::MiddleTip;
	const int32 RingTip = (int32)EHandKeypoint::RingTip;
	const int32 LittleTip = (int32)EHandKeypoint::LittleTip;



	TArray<int32> FingerTips = { IndexTip, MiddleTip, RingTip, LittleTip };

	for (int32 Tip : FingerTips)
	{
		if (!Positions.IsValidIndex(Tip) || !Positions.IsValidIndex(Palm))
			return false;

		float Dist = FVector::Dist(Positions[Tip], Positions[Palm]);
		if (Dist > GrabThreshold) // if any finger too far = not grabbing
		{
			bIsGrabbing = false;
			break;
		}
	}

	return bIsGrabbing;
}

void AHanTracking::LogXRInfo() const
{
	/*   if (GEngine && GEngine->XRSystem.IsValid())
	   {
		   UE_LOG(LogTemp, Log, TEXT("XR System: %s"), *GEngine->XRSystem->GetSystemName().ToString());
		   IHandTracker* HT = GEngine->XRSystem->GetHandTrackerDevice();
		   UE_LOG(LogTemp, Log, TEXT("Hand tracking: %s"),
			   (HT && HT->IsHandTrackingStateValid()) ? TEXT("ENABLED") : TEXT("DISABLED/NULL"));
	   }
	   else
	   {
		   UE_LOG(LogTemp, Warning, TEXT("No XR System active."));
	   }*/
}


void AHanTracking::UpdateRaycastFromAim(
	EControllerHand Hand,
	const FXRMotionControllerState& ControllerState,
	const FVector& AimPosition,   // new: passed in explicitly
	bool bIsPinching)
{
	// In UE 5.7, FXRMotionControllerState gives you ControllerLocation/Rotation.
	// Make sure when you fill ControllerState you request the Palm (or Grip) pose
	// via IXRTrackingSystem::GetMotionControllerState (XRControllerPoseType::Palm, etc).
	const FVector PalmPos = ControllerState.ControllerLocation;

	const FVector AimDir = AimPosition - PalmPos;
	if (AimDir.IsNearlyZero())
	{
		return;
	}

	const FVector NormalizedAimDir = AimDir.GetSafeNormal();
	const float TraceDistance = 1000.0f; // tweak as needed

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(PinchRay), /*bTraceComplex=*/false);
	CollisionParams.AddIgnoredActor(this);

	const FVector TraceStart = PalmPos;
	const FVector TraceEnd = PalmPos + NormalizedAimDir * TraceDistance;

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		CollisionParams
	);

	if (bHit && bIsPinching)
	{
		if (AStaticMeshActor* HitActor = Cast<AStaticMeshActor>(HitResult.GetActor()))
		{
			if (UStaticMeshComponent* MeshComp = HitActor->GetStaticMeshComponent())
			{
				const FVector CurrentScale = MeshComp->GetComponentScale();
				const FVector NewScale = CurrentScale * 1.003f; // scaling while pinching
				MeshComp->SetWorldScale3D(NewScale);
			}
		}
	}
}


static FORCEINLINE FVector LerpExpVec(const FVector& from, const FVector& to, float hz, float dt)
{
	const float a = 1.f - FMath::Exp(-FMath::Max(0.f, hz) * dt);
	return FMath::Lerp(from, to, a);
}




AHanTracking* AHanTracking::GetInstance()
{
	return HandTrackingInstance;
}

void AHanTracking::SetUpPostProcessVolume()
{
	bool IsPluginLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("MetaverseXRSystem"));
	if (IsPluginLoaded)
	{

		// Spawn the post-process volume
		PostProcessVolume = GetWorld()->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
#if WITH_EDITOR
		PostProcessVolume->SetActorLabel("MR POST PROCESS");
#endif
		PostProcessVolume->bUnbound = true;

		// Correct path for the material inside the plugin content folder
		FString MaterialPath = TEXT("/MetaverseXRSystem/MixedReality/PP_MR.PP_MR");

		UMaterialInterface* PP_MRMaterial = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);

		if (PP_MRMaterial)
		{
			// Create the dynamic material instance
			UMaterialInstanceDynamic* MaterialDynamic = UMaterialInstanceDynamic::Create(PP_MRMaterial, this);

			if (MaterialDynamic)
			{
				// Add the dynamic material to the post-process volume's weighted blendables array
#if WITH_EDITOR
				PostProcessVolume->SetActorLabel("MR_PostProcess");
#endif
				PostProcessVolume->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, MaterialDynamic));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to create MaterialInstanceDynamic."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load material from path: %s"), *MaterialPath);
		}
	}
}

void AHanTracking::ApplyYawRotationToPawn(const TArray<FVector>& HandPositions, // note: const ref
	bool bPinchActive,
	float DeltaSeconds)
{
	// --------- Validate input / georef ---------
	const int32 ThumbTip = (int32)EHandKeypoint::ThumbTip;
	const int32 IndexTip = (int32)EHandKeypoint::IndexTip;

	if (!HandPositions.IsValidIndex(ThumbTip) ||
		!HandPositions.IsValidIndex(IndexTip))
	{
		return;
	}

	// --------- Start / end of pinch (now uses member state) ---------
	if (!bPinchActive)
	{
		bPinchActiveLatched = false;
		PrevSmMid = FVector::ZeroVector;
		BaseMid = FVector::ZeroVector;
		bArmed = false;
		return;
	}

	// Pinch midpoint (raw & smoothed)
	const FVector pinchMidRaw = 0.5f * (HandPositions[ThumbTip] + HandPositions[IndexTip]);

	if (!bPinchActiveLatched)
	{
		bPinchActiveLatched = true;
		PrevSmMid = pinchMidRaw;
		BaseMid = pinchMidRaw; // baseline (significant displacement measured from here)
		bArmed = false;
		return;
	}

	const FVector smMid = LerpExpVec(PrevSmMid, pinchMidRaw, PinchSmoothHz, DeltaSeconds);

	// --- Baseline check (significant enough) ---
	const FVector dispFromBase = smMid - BaseMid;
	const float   baseMag = dispFromBase.Size();
	if (!bArmed)
	{
		if (baseMag < BaselineArmUU)
		{
			PrevSmMid = smMid;
			return;
		}
		bArmed = true;
	}

	// --- Per-frame delta with deadzone & clamp ---
	FVector d = smMid - PrevSmMid;
	float   mag = d.Size();
	if (mag <= PinchDeadzoneUU) { PrevSmMid = smMid; return; }
	if (mag > HandDeltaMaxUU) { d *= (HandDeltaMaxUU / mag); mag = HandDeltaMaxUU; }

	const float deltaZ_MetersRaw = (smMid.Z - BaseMid.Z) * 10;

	if (FMath::Abs(deltaZ_MetersRaw) > YawDeadZoneMeters)
	{
		deltaZ_Meters = (FMath::Abs(deltaZ_MetersRaw) - YawDeadZoneMeters) * FMath::Sign(deltaZ_MetersRaw);
		YawSpeedDegreePerSec = -YawDegreePerMeter * deltaZ_Meters;
	}

	YawSpeedDegreePerSec = FMath::Clamp(YawSpeedDegreePerSec, -MaxYawDegreePerSec, +MaxYawDegreePerSec);

	const float yawThisFrameDeg = YawSpeedDegreePerSec * DeltaSeconds;
	if (PlayerControllerRef)
	{
		PlayerControllerRef->GetPawn()->AddActorWorldRotation(FRotator(0.f, yawThisFrameDeg, 0.0f));
	}


}


void AHanTracking::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PostProcessVolume = nullptr;
	delete PostProcessVolume;
	if (HandTrackingInstance == this) {
		HandTrackingInstance = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}