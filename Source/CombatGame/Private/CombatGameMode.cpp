#include "CombatGameMode.h"
#include "CombatGame.h"
#include "CombatGameInstance.h"
#include "CombatCharacter.h"
#include "CombatPlayerController.h"
#include "FighterAIController.h"
#include "FightingCameraActor.h"
#include "CombatHUD.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

ACombatGameMode::ACombatGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = nullptr; // We spawn fighters manually
	PlayerControllerClass = ACombatPlayerController::StaticClass();
	HUDClass = ACombatHUD::StaticClass();

	CurrentMatchState = EMatchState::WaitingToStart;
	CurrentRound = 0;
	Player1RoundWins = 0;
	Player2RoundWins = 0;
	RoundsToWin = 2;
	RoundTimeRemaining = 60.0f;
	RoundTimeLimit = 60.0f;
}

void ACombatGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Get settings from game instance
	if (UCombatGameInstance* GI = Cast<UCombatGameInstance>(GetGameInstance()))
	{
		RoundsToWin = GI->RoundsToWin;
		RoundTimeLimit = GI->RoundTimeLimit;
	}

	SpawnFighters();
	StartRound();
}

void ACombatGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentMatchState == EMatchState::Fighting)
	{
		HandleFighting(DeltaTime);
	}
}

void ACombatGameMode::SpawnFighters()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UCombatGameInstance* GI = Cast<UCombatGameInstance>(GetGameInstance());

	// Spawn Player 1
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Use character class from selection, or try BP_TestFighter, or fall back to C++
	UClass* P1Class = nullptr;
	if (GI && GI->Player1Character.FighterClass.IsValid())
	{
		P1Class = GI->Player1Character.FighterClass.LoadSynchronous();
	}
	if (!P1Class)
	{
		// Try loading the Blueprint version first (has input actions configured)
		P1Class = StaticLoadClass(ACombatCharacter::StaticClass(),
			nullptr, TEXT("/Game/Characters/TestFighter/BP_TestFighter.BP_TestFighter_C"));
	}
	if (!P1Class)
	{
		P1Class = ACombatCharacter::StaticClass();
	}

	FRotator P1Rot = FRotator(0.0f, 0.0f, 0.0f);
	Fighter1 = World->SpawnActor<ACombatCharacter>(P1Class, Player1SpawnLocation, P1Rot, SpawnParams);
	if (Fighter1)
	{
		Fighter1->PlayerIndex = 0;
		Fighter1->OpponentFighter = nullptr; // Set after both spawn

		// Possess with player controller
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			PC->Possess(Fighter1);
		}

		// Apply character stats
		if (GI && GI->Player1Character.MaxHealth > 0)
		{
			Fighter1->MaxHealth = GI->Player1Character.MaxHealth;
			Fighter1->CurrentHealth = GI->Player1Character.MaxHealth;
		}
	}

	// Spawn Player 2
	UClass* P2Class = nullptr;
	if (GI && GI->Player2Character.FighterClass.IsValid())
	{
		P2Class = GI->Player2Character.FighterClass.LoadSynchronous();
	}
	if (!P2Class)
	{
		P2Class = StaticLoadClass(ACombatCharacter::StaticClass(),
			nullptr, TEXT("/Game/Characters/TestFighter/BP_TestFighter.BP_TestFighter_C"));
	}
	if (!P2Class)
	{
		P2Class = ACombatCharacter::StaticClass();
	}

	FRotator P2Rot = FRotator(0.0f, 180.0f, 0.0f);
	Fighter2 = World->SpawnActor<ACombatCharacter>(P2Class, Player2SpawnLocation, P2Rot, SpawnParams);
	if (Fighter2)
	{
		Fighter2->PlayerIndex = 1;

		// AI or Player 2
		bool bIsAI = GI ? GI->bPlayer2IsAI : true;
		if (bIsAI)
		{
			// Spawn AI controller
			AFighterAIController* AICon = World->SpawnActor<AFighterAIController>(
				AFighterAIController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			if (AICon)
			{
				AICon->Possess(Fighter2);
				AICon->Difficulty = GI ? GI->AIDifficulty : 0.5f;
			}
		}
		else
		{
			// Create and assign Player 2 controller
			FActorSpawnParameters ControllerParams;
			APlayerController* PC2 = World->SpawnActor<APlayerController>(
				APlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, ControllerParams);
			if (PC2)
			{
				UGameplayStatics::CreatePlayer(World, 1, true);
				APlayerController* P2Controller = UGameplayStatics::GetPlayerController(World, 1);
				if (P2Controller)
				{
					P2Controller->Possess(Fighter2);
				}
			}
		}

		if (GI && GI->Player2Character.MaxHealth > 0)
		{
			Fighter2->MaxHealth = GI->Player2Character.MaxHealth;
			Fighter2->CurrentHealth = GI->Player2Character.MaxHealth;
		}
	}

	// Link opponents
	if (Fighter1 && Fighter2)
	{
		Fighter1->OpponentFighter = Fighter2;
		Fighter2->OpponentFighter = Fighter1;
	}

	// Spawn fighting camera
	FActorSpawnParameters CamParams;
	AFightingCameraActor* FightCam = World->SpawnActor<AFightingCameraActor>(
		AFightingCameraActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, CamParams);
	if (FightCam)
	{
		FightCam->Fighter1 = Fighter1;
		FightCam->Fighter2 = Fighter2;

		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC)
		{
			PC->SetViewTarget(FightCam);
		}
	}
}

void ACombatGameMode::SetMatchState(EMatchState NewState)
{
	CurrentMatchState = NewState;
	OnMatchStateChanged.Broadcast(NewState);
}

void ACombatGameMode::StartRound()
{
	CurrentRound++;
	RoundTimeRemaining = RoundTimeLimit;

	ResetFightersForRound();
	SetMatchState(EMatchState::RoundIntro);

	OnRoundStarted.Broadcast(CurrentRound);

	// Delay before fighting begins
	GetWorldTimerManager().SetTimer(RoundIntroTimerHandle, [this]()
	{
		SetMatchState(EMatchState::Fighting);

		// Enable fighter input
		if (Fighter1) Fighter1->SetFighterInputEnabled(true);
		if (Fighter2) Fighter2->SetFighterInputEnabled(true);

	}, RoundIntroTime, false);
}

void ACombatGameMode::HandleFighting(float DeltaTime)
{
	// Update timer
	if (RoundTimeLimit > 0.0f)
	{
		RoundTimeRemaining -= DeltaTime;
		OnTimerUpdated.Broadcast(RoundTimeRemaining);

		if (RoundTimeRemaining <= 0.0f)
		{
			RoundTimeRemaining = 0.0f;
			EndRound(DetermineTimeOutWinner());
		}
	}
}

void ACombatGameMode::ReportDamage(ACombatCharacter* DamagedFighter, float Damage)
{
	if (!DamagedFighter || CurrentMatchState != EMatchState::Fighting) return;

	int32 PlayerIdx = DamagedFighter->PlayerIndex;
	float HealthPercent = DamagedFighter->CurrentHealth / DamagedFighter->MaxHealth;
	OnFighterDamaged.Broadcast(PlayerIdx, HealthPercent);

	if (DamagedFighter->CurrentHealth <= 0.0f)
	{
		ReportKO(DamagedFighter);
	}
}

void ACombatGameMode::ReportKO(ACombatCharacter* DefeatedFighter)
{
	if (!DefeatedFighter) return;

	ERoundResult Result;
	if (DefeatedFighter == Fighter1)
	{
		Result = ERoundResult::Player2Win;
	}
	else
	{
		Result = ERoundResult::Player1Win;
	}

	EndRound(Result);
}

void ACombatGameMode::EndRound(ERoundResult Result)
{
	if (CurrentMatchState != EMatchState::Fighting) return;

	SetMatchState(EMatchState::RoundEnd);

	// Disable fighter input
	if (Fighter1) Fighter1->SetFighterInputEnabled(false);
	if (Fighter2) Fighter2->SetFighterInputEnabled(false);

	// Record round data
	FRoundData RoundData;
	RoundData.RoundNumber = CurrentRound;
	RoundData.Result = Result;
	RoundData.Player1HealthRemaining = Fighter1 ? Fighter1->CurrentHealth : 0.0f;
	RoundData.Player2HealthRemaining = Fighter2 ? Fighter2->CurrentHealth : 0.0f;
	RoundData.RoundTime = RoundTimeLimit - RoundTimeRemaining;
	RoundHistory.Add(RoundData);

	// Update wins
	if (Result == ERoundResult::Player1Win)
	{
		Player1RoundWins++;
	}
	else if (Result == ERoundResult::Player2Win)
	{
		Player2RoundWins++;
	}

	OnRoundEnded.Broadcast(CurrentRound, Result);

	// Check for match end
	GetWorldTimerManager().SetTimer(RoundEndTimerHandle, [this, Result]()
	{
		if (Player1RoundWins >= RoundsToWin)
		{
			EndMatch(ERoundResult::Player1Win);
		}
		else if (Player2RoundWins >= RoundsToWin)
		{
			EndMatch(ERoundResult::Player2Win);
		}
		else
		{
			StartRound();
		}
	}, RoundEndTime, false);
}

void ACombatGameMode::EndMatch(ERoundResult FinalResult)
{
	SetMatchState(EMatchState::MatchEnd);
	OnMatchEnded.Broadcast(FinalResult);

	// Play victory animation
	if (FinalResult == ERoundResult::Player1Win && Fighter1)
	{
		Fighter1->SetFighterState(EFighterState::Victory);
	}
	else if (FinalResult == ERoundResult::Player2Win && Fighter2)
	{
		Fighter2->SetFighterState(EFighterState::Victory);
	}
}

void ACombatGameMode::ResetFightersForRound()
{
	if (Fighter1)
	{
		Fighter1->SetActorLocation(Player1SpawnLocation);
		Fighter1->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
		Fighter1->ResetForRound();
		Fighter1->SetFighterInputEnabled(false);
	}
	if (Fighter2)
	{
		Fighter2->SetActorLocation(Player2SpawnLocation);
		Fighter2->SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
		Fighter2->ResetForRound();
		Fighter2->SetFighterInputEnabled(false);
	}
}

ERoundResult ACombatGameMode::DetermineTimeOutWinner()
{
	if (!Fighter1 || !Fighter2) return ERoundResult::Draw;

	float P1Pct = Fighter1->CurrentHealth / Fighter1->MaxHealth;
	float P2Pct = Fighter2->CurrentHealth / Fighter2->MaxHealth;

	if (P1Pct > P2Pct) return ERoundResult::Player1Win;
	if (P2Pct > P1Pct) return ERoundResult::Player2Win;
	return ERoundResult::Draw;
}

void ACombatGameMode::ReturnToCharacterSelect()
{
	UGameplayStatics::OpenLevel(this, FName("/Game/Maps/CharacterSelectMap"));
}

void ACombatGameMode::ReturnToMainMenu()
{
	UGameplayStatics::OpenLevel(this, FName("/Game/Maps/MainMenuMap"));
}
