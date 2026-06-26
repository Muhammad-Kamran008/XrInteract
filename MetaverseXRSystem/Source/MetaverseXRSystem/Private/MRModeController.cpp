#include "MRModeController.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "InputCoreTypes.h"
#include "Engine/Engine.h"
#include "OculusXRPassthroughSubsystem.h"
#include "EngineUtils.h"
#include "OpenXRBlueprintFunctionLibrary.h"


#include "OculusXRPersistentPassthroughInstance.h"
#include "OculusXRPassthroughLayerShapes.h"
#include "OculusXRPassthroughLayerComponent.h"



AMRModeController::AMRModeController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMRModeController::BeginPlay()
{
	Super::BeginPlay();

	bIsMetaQuestHMD = IsMetaQuestHMD();

	UE_LOG(LogTemp, Warning, TEXT("MRModeController: HMD Device Name = %s"),
		*UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName().ToString());

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		EnableInput(PC);
		BindInput();
	}

	IConsoleVariable* PropagateAlphaCVar =
		IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessing.PropagateAlpha"));

	// Current HMD is quest
	if (bIsMetaQuestHMD && GEngine)
	{
		UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());

		if (GameInstance)
		{
			PassthroughSubsystem =
				GameInstance->GetSubsystem<UOculusXRPassthroughSubsystem>();
		}


		if (!PassthroughSubsystem)
		{
			UE_LOG(LogTemp, Error, TEXT("MRModeController: Failed to get OculusXRPassthroughSubsystem."));
		}

		PropagateAlphaCVar->Set(0, ECVF_SetByCode);
	}
	// Current HMD is Varjo:
	else {
		PropagateAlphaCVar->Set(1, ECVF_SetByCode);
	}

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag("Ultra_Dynamic_Sky"))
		{
			UltraDynamicSkyActor = *It;
			VROnlyObjects.Add(UltraDynamicSkyActor);
			UE_LOG(LogTemp, Warning, TEXT("Found Ultra Dynamic Sky Actor: %s"), *UltraDynamicSkyActor->GetName());
			break;
		}
	}

	DisableMR();
}




void AMRModeController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsMetaQuestHMD)
	{
		DestroyMetaPassthrough();
	}

	Super::EndPlay(EndPlayReason);
}

void AMRModeController::BindInput()
{
	if (!InputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("MRModeController: InputComponent is null."));
		return;
	}

	InputComponent->BindKey(EKeys::N, IE_Pressed, this, &AMRModeController::ToggleMRMode);
	InputComponent->BindKey(EKeys::B, IE_Pressed, this, &AMRModeController::ToggleBackground);
}

bool AMRModeController::IsMetaQuestHMD() const
{
	if (bForceMetaQuestMode)
	{
		return true;
	}

	const FString HMDName = UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName().ToString();

	return HMDName.Contains(TEXT("Oculus"), ESearchCase::IgnoreCase) ||
		HMDName.Contains(TEXT("Meta"), ESearchCase::IgnoreCase) ||
		HMDName.Contains(TEXT("Quest"), ESearchCase::IgnoreCase);
}

void AMRModeController::ToggleMRMode()
{
	bMRActive = !bMRActive;

	if (bMRActive)
	{
		EnableMR();
	}
	else
	{
		DisableMR();
	}
}

void AMRModeController::EnableMR()
{
	UE_LOG(LogTemp, Warning, TEXT("MRModeController: Enable MR"));


	UOpenXRBlueprintFunctionLibrary::SetEnvironmentBlendMode(EOpenXREnvironmentBlendMode::AlphaBlend);



	if (bIsMetaQuestHMD)
	{
		InitializeMetaPassthrough();
	}
}

void AMRModeController::DisableMR()
{
	UE_LOG(LogTemp, Warning, TEXT("MRModeController: Disable MR / Enable VR"));

	if (bIsMetaQuestHMD)
	{
		DestroyMetaPassthrough();
	}
	UOpenXRBlueprintFunctionLibrary::SetEnvironmentBlendMode(EOpenXREnvironmentBlendMode::Opaque);

}

void AMRModeController::InitializeMetaPassthrough()
{
	if (!PassthroughSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("MRModeController: Cannot initialize passthrough. Subsystem is null."));
		return;
	}

	FOculusXRPersistentPassthroughParameters Parameters;

	Parameters.bVisible = true;
	Parameters.Priority = 0;

	UOculusXRStereoLayerShapeReconstructed* ReconstructedShape =
		NewObject<UOculusXRStereoLayerShapeReconstructed>(this);

	if (!ReconstructedShape)
	{
		UE_LOG(LogTemp, Error, TEXT("MRModeController: Failed to create reconstructed passthrough shape."));
		return;
	}

	// Equivalent to Blueprint Layer Placement = Underlay
	ReconstructedShape->SetLayerPlacement(
		EOculusXRPassthroughLayerOrder::PassthroughLayerOrder_Underlay
	);


	ReconstructedShape->SetTextureOpacity(1.0f);

	ReconstructedShape->EnableEdgeColor(false);
	ReconstructedShape->EnableColorMap(false);
	//ReconstructedShape->SetColorMapType(
	//    EOculusXRColorMapType::ColorMapType_None
	//);

	Parameters.Shape = ReconstructedShape;
	Parameters.ApplyShape();

	FOculusXRPassthrough_LayerResumed_Single LayerResumed;

	UOculusXRPersistentPassthroughInstance* Instance =
		PassthroughSubsystem->InitializePersistentPassthrough(
			Parameters,
			LayerResumed
		);

	if (!Instance)
	{
		UE_LOG(LogTemp, Error, TEXT("MRModeController: InitializePersistentPassthrough returned null."));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("MRModeController: Persistent passthrough initialized."));
}

void AMRModeController::DestroyMetaPassthrough()
{
	if (!PassthroughSubsystem)
	{
		return;
	}

	PassthroughSubsystem->DestroyPersistentPassthrough();

	UE_LOG(LogTemp, Warning, TEXT("MRModeController: Destroy Persistent Passthrough called."));
}

void AMRModeController::ToggleBackground()
{
	bBackgroundVisible = !bBackgroundVisible;
	SetVROnlyObjectsVisible(bBackgroundVisible);
}

void AMRModeController::SetVROnlyObjectsVisible(bool bVisible)
{
	for (AActor* Actor : VROnlyObjects)
	{
		if (!Actor)
		{
			continue;
		}

		Actor->SetActorHiddenInGame(!bVisible);
		Actor->SetActorEnableCollision(bVisible);

		TArray<USceneComponent*> SceneComponents;
		Actor->GetComponents<USceneComponent>(SceneComponents);

		for (USceneComponent* SceneComponent : SceneComponents)
		{
			if (SceneComponent)
			{
				SceneComponent->SetVisibility(bVisible, true);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("MRModeController: VR-only objects visible = %s"),
		bVisible ? TEXT("TRUE") : TEXT("FALSE"));
}