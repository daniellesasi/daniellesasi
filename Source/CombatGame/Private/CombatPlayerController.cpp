// CombatPlayerController.cpp
#include "CombatPlayerController.h"
#include "CombatCharacter.h"
#include "CombatGame.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

ACombatPlayerController::ACombatPlayerController()
{
	// Fighting game: disable mouse/stick look so input goes to character, not camera
	bAutoManageActiveCameraTarget = false;
}

void ACombatPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Disable default mouse look — fighting game camera is fixed
	bShowMouseCursor = false;
	SetIgnoreLookInput(true);
}

void ACombatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// The Enhanced Input subsystem is guaranteed ready by BeginPlay.
	// OnPossess may have been called earlier (during SpawnFighters) when the
	// subsystem wasn't ready yet, so retry adding the mapping context here.
	if (!bMappingContextAdded && GetPawn())
	{
		AddFighterMappingContext();
	}
}

void ACombatPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AddFighterMappingContext();
}

void ACombatPlayerController::OnUnPossess()
{
	bMappingContextAdded = false;
	RemoveFighterMappingContext();
	Super::OnUnPossess();
}

void ACombatPlayerController::AddFighterMappingContext()
{
	// First try the mapping context set on the controller itself
	UInputMappingContext* IMC = FighterMappingContext;

	// Load IMC_Fighter by path as fallback
	if (!IMC)
	{
		IMC = Cast<UInputMappingContext>(
			StaticLoadObject(UInputMappingContext::StaticClass(), nullptr,
				TEXT("/Game/Input/IMC_Fighter.IMC_Fighter")));
	}

	if (!IMC)
	{
		UE_LOG(LogCombatGame, Warning, TEXT("CombatPlayerController: No InputMappingContext found!"));
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(IMC, 0);
		bMappingContextAdded = true;
		UE_LOG(LogCombatGame, Log, TEXT("CombatPlayerController: Added fighter mapping context successfully"));
	}
	else
	{
		UE_LOG(LogCombatGame, Warning, TEXT("CombatPlayerController: EnhancedInput subsystem not ready yet"));
	}
}

void ACombatPlayerController::RemoveFighterMappingContext()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
	}
}
