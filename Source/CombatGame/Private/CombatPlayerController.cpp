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

	UE_LOG(LogCombatGame, Warning, TEXT("CombatPlayerController::BeginPlay. bMappingContextAdded=%d, GetPawn=%s, IsLocalController=%d"),
		bMappingContextAdded, GetPawn() ? *GetPawn()->GetName() : TEXT("NULL"), IsLocalController());

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
	UE_LOG(LogCombatGame, Warning, TEXT("CombatPlayerController::OnPossess called. Pawn=%s"), InPawn ? *InPawn->GetName() : TEXT("NULL"));
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
	UE_LOG(LogCombatGame, Warning, TEXT("AddFighterMappingContext: ENTERED. IsLocalController=%d, GetLocalPlayer=%s"),
		IsLocalController(), GetLocalPlayer() ? TEXT("Valid") : TEXT("NULL"));

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

	UE_LOG(LogCombatGame, Warning, TEXT("AddFighterMappingContext: IMC found = %s"), *IMC->GetName());

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(IMC, 0);
		bMappingContextAdded = true;
		UE_LOG(LogCombatGame, Warning, TEXT("CombatPlayerController: Added fighter mapping context successfully"));
	}
	else
	{
		UE_LOG(LogCombatGame, Warning, TEXT("CombatPlayerController: EnhancedInput subsystem not ready! GetLocalPlayer=%s"),
			GetLocalPlayer() ? TEXT("Valid") : TEXT("NULL"));
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
