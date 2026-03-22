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

void ACombatPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AddFighterMappingContext();
}

void ACombatPlayerController::OnUnPossess()
{
	RemoveFighterMappingContext();
	Super::OnUnPossess();
}

void ACombatPlayerController::AddFighterMappingContext()
{
	// First try the mapping context set on the controller itself
	UInputMappingContext* IMC = FighterMappingContext;

	// If not set on the controller, grab it from the possessed fighter
	if (!IMC)
	{
		if (ACombatCharacter* Fighter = Cast<ACombatCharacter>(GetPawn()))
		{
			// Access the character's mapping context via its public getter or UPROPERTY
			// The character stores it as FighterMappingContext (EditDefaultsOnly)
			// We need to load it by asset path as fallback
		}

		// Load IMC_Fighter directly as ultimate fallback
		if (!IMC)
		{
			IMC = Cast<UInputMappingContext>(
				StaticLoadObject(UInputMappingContext::StaticClass(), nullptr,
					TEXT("/Game/Input/IMC_Fighter.IMC_Fighter")));
		}
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
		UE_LOG(LogCombatGame, Log, TEXT("CombatPlayerController: Added fighter mapping context"));
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
