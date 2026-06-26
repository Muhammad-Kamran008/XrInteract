#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <MotionControllerComponent.h>
#include "ControllerTracking.generated.h"

UCLASS()
class METAVERSEXRSYSTEM_API AControllerTracking : public AActor
{
	GENERATED_BODY()

public:
	AControllerTracking();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:

	UMotionControllerComponent* LeftMotionController;
	UMotionControllerComponent* RightMotionController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category ="ControllerTracking")
	UMotionControllerComponent* LeftController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ControllerTracking")
	UMotionControllerComponent* RightController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ControllerTracking")
	UStaticMeshComponent* LeftControllerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ControllerTracking")
	UStaticMeshComponent* RightControllerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ControllerTracking")
	UStaticMesh* DefaultLeftMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ControllerTracking")
	UStaticMesh* DefaultRightMesh;

	UPROPERTY(BlueprintReadWrite, Category = "ControllerTracking")
	bool bIsControllerConnected;

	UPROPERTY(BlueprintReadWrite, Category = "ControllerTracking")
	bool bIsCurrentlyConnected;

	UFUNCTION(BlueprintCallable, Category = "ControllerTracking")
	bool AreControllersConnected();

	UFUNCTION(BlueprintCallable, Category = "ControllerTracking")
	void SetControllersActive(bool bActivate);

	IXRTrackingSystem* XRSystem;
	TArray<int32> DevicesIds;
	APlayerController* PC;

private:
	void UpdateConnectionState();
	void InitializeMeshes();
	void PlayHaptic(UHapticFeedbackEffect_Base* Effect, EControllerHand Hand);

};
