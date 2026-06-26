#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IXRTrackingSystem.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/SphereComponent.h"
#include "BoneVisualizer.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "OculusXRHandComponent.h"
#include "MotionControllerComponent.h"
#include "Materials/MaterialInterface.h"
#include "HanTracking.generated.h"

UCLASS()

class METAVERSEXRSYSTEM_API AHanTracking : public AActor
{
	GENERATED_BODY()
private:
	static AHanTracking* HandTrackingInstance;
public:
	AHanTracking();
	APlayerController* PlayerControllerRef = nullptr;

	static AHanTracking* GetInstance();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void SetUpPostProcessVolume();


	//Rotation Variables
	UPROPERTY(EditAnywhere, Category = "Pinch Orbit")
	bool    bPinchActiveLatched = false;
	UPROPERTY(EditAnywhere, Category = "Pinch Orbit")
	FVector PrevSmMid = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Pinch Orbit")
	FVector BaseMid = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "XR|YAW ROTATION")
	float YawDegreePerMeter = 10.f;
	UPROPERTY(EditAnywhere, Category = "XR|YAW ROTATION")
	float YawDeadZoneMeters = 0.015f;
	UPROPERTY(EditAnywhere, Category = "XR|YAW ROTATION")
	float MaxYawDegreePerSec = 420.f;
	UPROPERTY(EditAnywhere, Category = "XR|YAW ROTATION")
	float YawSmoothingHalflife = 0.06f;
	float deltaZ_Meters = 0.0f;
	float YawRateSmoothDegPerSec = 0.f;
	float YawSpeedDegreePerSec = 0.f;



	bool    bArmed = false;
	float   PinchSmoothHz = 8.0f;
	float   HandDeltaMaxUU = 60.0f;


	UPROPERTY(EditAnywhere, Category = "Interation Collider")
	USphereComponent* Interaction_Collider;

	UPROPERTY(EditAnywhere, Category = "MR|Feed")
	APostProcessVolume* PostProcessVolume = nullptr;
	UPROPERTY(EditAnywhere, Category = "MR material")
	UMaterialInterface* PP_MR = nullptr;



	UStaticMeshComponent* SphereMesh;
	UMaterialInterface* SphereMaterial;
	FXRMotionControllerState ControllerDataRight;
	FXRMotionControllerState ControllerDataLeft;

	FTimerHandle ReEnableTimerHandle;

	float PinchDeadzoneUU = 0.2f;
	float BaselineArmUU = 2.0f;


	APlayerCameraManager* CamManager;

	TArray<FVector> LP, RP;  TArray<FQuat> LR, RR;  TArray<float> LRd, RRd;

	bool bL = false;
	bool bR = false;
	bool bLPinch = false, bRPinch = false;

	UInstancedStaticMeshComponent* TipSpheresL = nullptr;
	UInstancedStaticMeshComponent* TipSpheresR = nullptr;
	ABoneVisualizer* LeftBoneVisualizerActor;
	ABoneVisualizer* RightBoneVisualizerActor;

	UPROPERTY(EditAnywhere, Category = "Pinch Deadzone")
	float PositionDeadZone = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Rotation Deadzone")
	float RotationDeadZone = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Widget Interactor")
	UWidgetInteractionComponent* WidgetInteractor;

	UPROPERTY(EditAnywhere, Category = "XR|Ray")
	float PosSmoothHz = 12.f;
	UPROPERTY(EditAnywhere, Category = "XR|Interaction Distance")
	float InteractioinDistance = 7.0f;
	bool bPrevPinching = false;
	static bool GetHandKeypoints(
		EControllerHand Hand,
		TArray<FVector>& OutPositions,
		TArray<FQuat>& OutRotations,
		TArray<float>& OutRadii);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	bool IsGrabbing(const TArray<FVector>& Positions, float PinchThresholdCm = 4.0f) const;

	void LogXRInfo() const;

	void UpdateRaycastFromAim(EControllerHand Hand, const FXRMotionControllerState& ControllerState, const FVector& AimPosition, bool bIsPinching);

	bool IsPinching(
		const TArray<FVector>& Positions,
		float PinchThresholdCm = 2.0f,
		float PinchReleaseThresholdCm = 7.0f);
	bool IsPalmFacingCamera(
		const TArray<FQuat>& Rotations, const TArray<FVector>& Locations) const;
	bool IsPointing(
		const TArray<FVector>& Positions,
		const TArray<FQuat>& Rotations,
		float TipThreshold = 4.0f,
		float CurlThreshold = 2.0f
	) const;

	void DrawHandsMesh();
	void DrawPinchRay(float DeltaSeconds);

	bool bPrevLeftPinching = false;
	bool bPrevRightPinching = false;
	bool ToggleMenuVisibility = true;
	bool PinchState = true;

	bool bMenuVisible = false;
	bool bMenuLockedByGrab = false;
	bool bPrevGrabState = false;

	struct FPinchRayState
	{
		bool bActive = false;
		FVector OriginWS = FVector::ZeroVector;
		FVector DirWS = FVector::ForwardVector; // locked while active
		FVector LastPinchPos = FVector::ZeroVector; // NEW: to compute deltas
		FHitResult Hit;
		TWeakObjectPtr<AActor> HitActor;
	};

	TArray<int32> TipInstanceIdxL;
	TArray<int32> TipInstanceIdxR;
	FPinchRayState LeftPinch, RightPinch;
	float TipSphereRadiusCm = 1.8f;

	bool bIsMoving = false;
	float currentSpeed = 0.f;
	float TwoHandZoomDeadzoneCm = 0.2f;

	TWeakObjectPtr<USceneComponent> TwoHandTargetComp;
	float   TwoHandBaselineDist = 10.0f;

	// Tuning
	float RayLength = 10000000.f;
	float RaySmoothHz = 18.f;   // origin smoothing
	bool  bLockDirOnPinchStart = true;

	float height = 0.f;

	void InitTipSpheresFor(EControllerHand Hand);

	void InitTipSpheresBoth();

	void UpdateTipSpheresFor(const TArray<FVector>& Positions, EControllerHand Hand);

	void UpdateTipSpheresBoth(const TArray<FVector>& LeftPositions,
		const TArray<FVector>& RightPositions);

	void ApplyYawRotationToPawn(const TArray<FVector>& HandPositions, // note: const ref
		bool bPinchActive,
		float DeltaSeconds);

	void HideTipSpheres(EControllerHand Hand);

	FVector GetPinchMid(const TArray<FVector>& Positions) const;

	void UpdatePinchRay(EControllerHand Hand,
		const TArray<FVector>& Positions,
		const TArray<FQuat>& Rotations,
		bool bIsPinching,
		float DeltaSeconds);

	float CalculateTwoHandSpeed(const TArray<FVector>& LeftP,
		const TArray<FVector>& RightP,
		bool bLeftPinch, bool bRightPinch,
		float DeltaSeconds);
	static float   ExpAlpha(float Hz, float Dt) { return (Hz <= 0.f) ? 1.f : 1.f - FMath::Exp(-Hz * Dt); }
	static FVector LerpExp(const FVector& A, const FVector& B, float Hz, float Dt) { return FMath::Lerp(A, B, ExpAlpha(Hz, Dt)); }
	static FQuat   SlerpExp(const FQuat& A, const FQuat& B, float Hz, float Dt) { return FQuat::Slerp(A, B, ExpAlpha(Hz, Dt)).GetNormalized(); }

	// New function (rotation-aware, smoothed)
	void UpdateRaycastFromAim(EControllerHand Hand, const FXRMotionControllerState& ControllerData, bool bIsPinching);
	bool bIsClicking = false;
	bool bIsGrabTriggered = true;
	bool bIsPointing = false;

	const float ClickDistance = 1.5;
	float Distance = 0;
	FVector HitLocation;

	UPROPERTY(EditAnywhere)
	UOculusXRHandComponent* LeftHandComponent = nullptr;

	UPROPERTY(EditAnywhere)
	UOculusXRHandComponent* RightHandComponent = nullptr;


	UPROPERTY(EditAnywhere)
	FVector RightHandComponentSize = FVector(1,1,1);

	UPROPERTY(EditAnywhere)
	FVector LeftHandComponentSize = FVector(1, 1, 1);

	UPROPERTY(EditAnywhere)
	UMotionControllerComponent* LeftMotionController = nullptr;

	UPROPERTY(EditAnywhere)
	UMotionControllerComponent* RightMotionController = nullptr;

	void InitMetaQuestHandMeshes();
	void StartQuestCasting();
	bool IsMetaQuestHMD() const;
	void UpdateMetaQuestHandVisibility();


	UPROPERTY(EditAnywhere, Category = "Hand Tracking")
	TObjectPtr<UMaterialInterface> QuestHandMaterial = nullptr;



};