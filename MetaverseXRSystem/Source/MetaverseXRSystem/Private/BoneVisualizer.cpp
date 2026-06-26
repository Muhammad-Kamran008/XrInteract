#include "BoneVisualizer.h"
#include "IXRTrackingSystem.h"
#include "IHandTracker.h"
#include "UObject/ConstructorHelpers.h"

#include "Components/InstancedStaticMeshComponent.h"

const TArray<TPair<EHandKeypoint, EHandKeypoint>> BonePairs = {
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::ThumbMetacarpal, EHandKeypoint::ThumbProximal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::ThumbProximal, EHandKeypoint::ThumbDistal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::ThumbDistal, EHandKeypoint::ThumbTip),

	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::IndexMetacarpal, EHandKeypoint::IndexProximal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::IndexProximal, EHandKeypoint::IndexIntermediate),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::IndexIntermediate, EHandKeypoint::IndexDistal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::IndexDistal, EHandKeypoint::IndexTip),

	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::MiddleMetacarpal, EHandKeypoint::MiddleProximal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::MiddleProximal, EHandKeypoint::MiddleIntermediate),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::MiddleIntermediate, EHandKeypoint::MiddleDistal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::MiddleDistal, EHandKeypoint::MiddleTip),

	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::RingMetacarpal, EHandKeypoint::RingProximal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::RingProximal, EHandKeypoint::RingIntermediate),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::RingIntermediate, EHandKeypoint::RingDistal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::RingDistal, EHandKeypoint::RingTip),

	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::LittleMetacarpal, EHandKeypoint::LittleProximal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::LittleProximal, EHandKeypoint::LittleIntermediate),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::LittleIntermediate, EHandKeypoint::LittleDistal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::LittleDistal, EHandKeypoint::LittleTip),

	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::Wrist, EHandKeypoint::MiddleMetacarpal),
};

const TArray<TPair<EHandKeypoint, EHandKeypoint>> BetweenBonePairs = {
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::ThumbMetacarpal, EHandKeypoint::IndexMetacarpal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::IndexMetacarpal, EHandKeypoint::MiddleMetacarpal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::MiddleMetacarpal, EHandKeypoint::RingMetacarpal),
	TPair<EHandKeypoint, EHandKeypoint>(EHandKeypoint::RingMetacarpal, EHandKeypoint::LittleMetacarpal),
};

ABoneVisualizer::ABoneVisualizer()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderAsset.Succeeded()) {
		BoneCyilnderMesh = CylinderAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CylinderMaterial(TEXT("/MetaverseXRSystem/MixedReality/M_BoneMaterial.M_BoneMaterial"));
	if (CylinderMaterial.Succeeded()) {
		BoneMaterial = CylinderMaterial.Object;
	}

}

void ABoneVisualizer::BeginPlay() {
	Super::BeginPlay();

	for (int32 i = 0; i < BonePairs.Num(); i++) {
		BoneComp = NewObject<UStaticMeshComponent>(this);
		BoneComp->SetupAttachment(RootComponent);
		BoneComp->RegisterComponent();

		if (BoneCyilnderMesh)
			BoneComp->SetStaticMesh(BoneCyilnderMesh);

		if (BoneMaterial)
			BoneComp->SetMaterial(0, BoneMaterial);

		BoneMeshes.Add(BoneComp);
	}


}

void ABoneVisualizer::UpdateBones(const TArray<FVector>& LeftJoints)
{
	
	for (int32 i = 0; i < BonePairs.Num(); i++) {
		StartIdx = (int32)BonePairs[i].Key;
		EndIdx = (int32)BonePairs[i].Value;

		if (LeftJoints.Num() != 0) {
			if (LeftJoints.IsValidIndex(StartIdx) && LeftJoints.IsValidIndex(EndIdx)) {
				UpdateBoneMesh(i, LeftJoints[StartIdx], LeftJoints[EndIdx]);
			}
		}
	}
	
}

void ABoneVisualizer::UpdateBoneMesh(int32 BoneIndex, const FVector& Start, const FVector& End)
{
	if (!BoneMeshes.IsValidIndex(BoneIndex) || !BoneMeshes[BoneIndex]) return;

	MidPoint = (Start + End) * 0.5;
	BoneDir = (End - Start);
	BoneLength = BoneDir.Size();

	Rotation = FRotationMatrix::MakeFromZ(BoneDir).Rotator();

	BoneMeshes[BoneIndex]->SetWorldLocation(MidPoint, false);
	BoneMeshes[BoneIndex]->SetWorldRotation(Rotation);

	MeshScale = FVector(0.006f, 0.006f, BoneLength / 120.f);
	BoneMeshes[BoneIndex]->SetWorldScale3D(MeshScale);
}
