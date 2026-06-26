#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "BoneVisualizer.generated.h"

UCLASS()
class METAVERSEXRSYSTEM_API ABoneVisualizer : public AActor
{
	GENERATED_BODY()

public:
	ABoneVisualizer();

	virtual void BeginPlay() override;

	// Update all bone meshes from keypoint positions
	void UpdateBones(const TArray<FVector>& LeftJointPositions);

	// Update a single bone mesh between Start and End
	void UpdateBoneMesh(int32 BoneIndex, const FVector& Start, const FVector& End);

	// Runtime array; not editor-exposed
	UPROPERTY(EditAnywhere,Category="BoneMesh")
	TArray<UStaticMeshComponent*> BoneMeshes;

	// Editor-exposed – needs Category
	UPROPERTY(EditAnywhere, Category = "HandTracking")
	UStaticMesh* BoneCyilnderMesh;

	// Editor-exposed – needs Category
	UPROPERTY(EditAnywhere, Category = "HandTracking")
	UMaterialInterface* BoneMaterial;

	// Runtime-only working vars (no editor exposure)
	UStaticMeshComponent* BoneComp = nullptr;
	int32 StartIdx;
	int32 EndIdx;
	FVector MidPoint;
	FVector BoneDir;
	float BoneLength;
	FRotator Rotation;
	FVector MeshScale;
};
