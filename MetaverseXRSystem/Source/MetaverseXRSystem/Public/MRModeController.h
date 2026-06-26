
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MRModeController.generated.h"

class UOculusXRPassthroughSubsystem;

UCLASS()
class METAVERSEXRSYSTEM_API AMRModeController : public AActor
{
    GENERATED_BODY()

public:
    AMRModeController();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(EditAnywhere, Category = "MR")
    TArray<AActor*> VROnlyObjects;

    UPROPERTY(EditAnywhere, Category = "MR")
    bool bForceMetaQuestMode = false;

    UPROPERTY()
    UOculusXRPassthroughSubsystem* PassthroughSubsystem = nullptr;

    bool bIsMetaQuestHMD = false;
    bool bMRActive = false;
    bool bBackgroundVisible = true;

    UPROPERTY() AActor* UltraDynamicSkyActor = nullptr;

private:
    void BindInput();
    void ToggleMRMode();
    void ToggleBackground();

    bool IsMetaQuestHMD() const;

    void EnableMR();
    void DisableMR();

    void InitializeMetaPassthrough();
    void DestroyMetaPassthrough();

    void SetVROnlyObjectsVisible(bool bVisible);
};