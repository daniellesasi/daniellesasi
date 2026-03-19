#include "FighterAIController.h"
#include "CombatCharacter.h"
#include "CombatGame.h"
#include "Kismet/GameplayStatics.h"

AFighterAIController::AFighterAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFighterAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AFighterAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Get references
	if (!ControlledFighter)
	{
		ControlledFighter = Cast<ACombatCharacter>(GetPawn());
		if (!ControlledFighter) return;
	}
	if (!OpponentFighter)
	{
		OpponentFighter = ControlledFighter->OpponentFighter;
		if (!OpponentFighter) return;
	}

	if (!ControlledFighter->bInputEnabled) return;

	// Cooldown
	ActionCooldown -= DeltaTime;

	// Decision timer
	DecisionTimer -= DeltaTime;
	if (DecisionTimer <= 0.0f)
	{
		DecisionTimer = DecisionInterval * FMath::FRandRange(0.8f, 1.2f);
		// Harder AI decides faster
		DecisionTimer *= (1.5f - Difficulty);
		DecideBehavior();
	}

	ExecuteBehavior(DeltaTime);
}

void AFighterAIController::DecideBehavior()
{
	if (!ControlledFighter || !OpponentFighter) return;

	float Distance = GetDistanceToOpponent();
	EFighterState OpponentState = OpponentFighter->CurrentState;
	EFighterState MyState = ControlledFighter->CurrentState;

	// Don't change behavior mid-attack
	if (MyState == EFighterState::Attacking) return;
	if (MyState == EFighterState::HitStun || MyState == EFighterState::BlockStun) return;

	// Punish window: opponent is in recovery
	if (OpponentState == EFighterState::Attacking && OpponentFighter->RemainingAttackFrames <= 5)
	{
		if (FMath::FRand() < GetPunishChance() && Distance < CloseRange)
		{
			CurrentBehavior = EAIBehavior::Punishing;
			return;
		}
	}

	// Block incoming attack
	if (OpponentState == EFighterState::Attacking && Distance < MidRange)
	{
		if (FMath::FRand() < GetBlockAccuracy())
		{
			CurrentBehavior = EAIBehavior::Blocking;
			return;
		}
	}

	// Opponent knocked down - pressure
	if (OpponentState == EFighterState::KnockedDown || OpponentState == EFighterState::GettingUp)
	{
		CurrentBehavior = EAIBehavior::WakeupAttack;
		return;
	}

	// Opponent in blockstun - pressure
	if (OpponentState == EFighterState::BlockStun)
	{
		if (FMath::FRand() < Difficulty * 0.7f)
		{
			CurrentBehavior = EAIBehavior::Pressuring;
			return;
		}
	}

	// Distance-based decisions
	if (Distance > FarRange)
	{
		CurrentBehavior = EAIBehavior::Approaching;
	}
	else if (Distance > MidRange)
	{
		float Roll = FMath::FRand();
		if (Roll < 0.5f)
			CurrentBehavior = EAIBehavior::Approaching;
		else if (Roll < 0.7f)
			CurrentBehavior = EAIBehavior::Sidestepping;
		else
			CurrentBehavior = EAIBehavior::Idle;
	}
	else if (Distance > CloseRange)
	{
		float Roll = FMath::FRand();
		if (Roll < 0.4f + Difficulty * 0.3f)
			CurrentBehavior = EAIBehavior::Attacking;
		else if (Roll < 0.6f)
			CurrentBehavior = EAIBehavior::Sidestepping;
		else
			CurrentBehavior = EAIBehavior::Retreating;
	}
	else
	{
		// Close range - attack heavy
		float Roll = FMath::FRand();
		if (Roll < 0.5f + Difficulty * 0.3f)
			CurrentBehavior = EAIBehavior::Attacking;
		else if (Roll < 0.7f)
			CurrentBehavior = EAIBehavior::Blocking;
		else
			CurrentBehavior = EAIBehavior::Retreating;
	}
}

void AFighterAIController::ExecuteBehavior(float DeltaTime)
{
	switch (CurrentBehavior)
	{
	case EAIBehavior::Approaching:
		PerformApproach(DeltaTime);
		break;
	case EAIBehavior::Attacking:
		PerformAttack();
		break;
	case EAIBehavior::Blocking:
		PerformBlock();
		break;
	case EAIBehavior::Retreating:
		PerformRetreat(DeltaTime);
		break;
	case EAIBehavior::Sidestepping:
		PerformSidestep();
		break;
	case EAIBehavior::Punishing:
		PerformPunish();
		break;
	case EAIBehavior::Pressuring:
		PerformPressure();
		break;
	case EAIBehavior::WakeupAttack:
		PerformWakeupAttack();
		break;
	default:
		break;
	}
}

void AFighterAIController::PerformApproach(float DeltaTime)
{
	if (!ControlledFighter || !OpponentFighter) return;

	FVector Direction = (OpponentFighter->GetActorLocation() - ControlledFighter->GetActorLocation()).GetSafeNormal();
	Direction.Z = 0.0f;
	ControlledFighter->AddMovementInput(Direction, 1.0f);

	// Harder AI sometimes dash-approaches
	if (Difficulty > 0.6f && FMath::FRand() < 0.1f * Difficulty)
	{
		ControlledFighter->PerformDash(true);
	}
}

void AFighterAIController::PerformAttack()
{
	if (ActionCooldown > 0.0f) return;

	FAttackData Attack = SelectAttack();
	ControlledFighter->ExecuteAttack(Attack);
	ActionCooldown = GetReactionTime();
	CurrentBehavior = EAIBehavior::Idle;
}

void AFighterAIController::PerformBlock()
{
	if (!ControlledFighter || !OpponentFighter) return;

	// Decide block height based on opponent's attack
	bool bShouldCrouch = false;
	if (OpponentFighter->bIsAttacking)
	{
		EHitHeight Height = OpponentFighter->CurrentAttack.HitHeight;
		if (Height == EHitHeight::Low)
		{
			bShouldCrouch = true;
		}
		// Harder AI reads mixups better
		if (FMath::FRand() > GetBlockAccuracy())
		{
			bShouldCrouch = !bShouldCrouch; // Wrong guess
		}
	}

	ControlledFighter->StartBlock();

	// After a short time, stop blocking
	ActionCooldown = FMath::FRandRange(0.3f, 0.8f);
}

void AFighterAIController::PerformRetreat(float DeltaTime)
{
	if (!ControlledFighter || !OpponentFighter) return;

	FVector Direction = (ControlledFighter->GetActorLocation() - OpponentFighter->GetActorLocation()).GetSafeNormal();
	Direction.Z = 0.0f;
	ControlledFighter->AddMovementInput(Direction, 1.0f);

	if (Difficulty > 0.5f && FMath::FRand() < 0.15f)
	{
		ControlledFighter->PerformDash(false);
		CurrentBehavior = EAIBehavior::Idle;
	}
}

void AFighterAIController::PerformSidestep()
{
	if (ActionCooldown > 0.0f) return;

	ControlledFighter->PerformSidestep(FMath::RandBool());
	ActionCooldown = 0.5f;
	CurrentBehavior = EAIBehavior::Idle;
}

void AFighterAIController::PerformPunish()
{
	if (ActionCooldown > 0.0f) return;

	// Use a fast move for punishing
	FAttackData Punish;
	for (const FAttackData& Move : ControlledFighter->MoveList)
	{
		if (Move.StartupFrames <= 12)
		{
			Punish = Move;
			break;
		}
	}

	if (Punish.MoveName.IsNone() && ControlledFighter->MoveList.Num() > 0)
	{
		Punish = ControlledFighter->MoveList[0];
	}

	ControlledFighter->ExecuteAttack(Punish);
	ActionCooldown = GetReactionTime();
	CurrentBehavior = EAIBehavior::Idle;
}

void AFighterAIController::PerformPressure()
{
	if (ActionCooldown > 0.0f) return;
	PerformAttack();
}

void AFighterAIController::PerformWakeupAttack()
{
	if (ActionCooldown > 0.0f) return;

	// Approach then attack on wakeup
	float Distance = GetDistanceToOpponent();
	if (Distance > CloseRange)
	{
		PerformApproach(GetWorld()->GetDeltaSeconds());
	}
	else
	{
		PerformAttack();
	}
}

FAttackData AFighterAIController::SelectAttack()
{
	if (!ControlledFighter || ControlledFighter->MoveList.Num() == 0)
	{
		// Return a default attack
		FAttackData Default;
		Default.MoveName = FName("Jab");
		Default.AttackType = EAttackType::HighPunch;
		Default.HitHeight = EHitHeight::High;
		Default.Damage = 10.0f;
		Default.StartupFrames = 10;
		Default.ActiveFrames = 3;
		Default.RecoveryFrames = 12;
		Default.HitStunFrames = 18;
		Default.BlockStunFrames = 8;
		return Default;
	}

	// Easy AI: mostly basic attacks
	// Hard AI: uses launchers, mixups, and optimal moves
	if (Difficulty < 0.3f)
	{
		// Just pick a random basic move
		int32 Idx = FMath::RandRange(0, FMath::Min(2, ControlledFighter->MoveList.Num() - 1));
		return ControlledFighter->MoveList[Idx];
	}
	else if (Difficulty < 0.7f)
	{
		// Use variety
		int32 Idx = FMath::RandRange(0, ControlledFighter->MoveList.Num() - 1);
		return ControlledFighter->MoveList[Idx];
	}
	else
	{
		// Smart selection based on opponent state
		if (OpponentFighter && OpponentFighter->CurrentState == EFighterState::Crouching)
		{
			// Use mid attacks against crouching opponent
			for (const FAttackData& Move : ControlledFighter->MoveList)
			{
				if (Move.HitHeight == EHitHeight::Mid) return Move;
			}
		}

		// Look for launchers when opponent is open
		if (OpponentFighter && OpponentFighter->RemainingStunFrames > 0)
		{
			for (const FAttackData& Move : ControlledFighter->MoveList)
			{
				if (Move.AttackType == EAttackType::Launcher) return Move;
			}
		}

		int32 Idx = FMath::RandRange(0, ControlledFighter->MoveList.Num() - 1);
		return ControlledFighter->MoveList[Idx];
	}
}

float AFighterAIController::GetReactionTime() const
{
	// Easy: 0.5s, Hard: 0.1s
	return FMath::Lerp(0.5f, 0.1f, Difficulty);
}

float AFighterAIController::GetBlockAccuracy() const
{
	// Easy: 30%, Hard: 90%
	return FMath::Lerp(0.3f, 0.9f, Difficulty);
}

float AFighterAIController::GetPunishChance() const
{
	// Easy: 10%, Hard: 80%
	return FMath::Lerp(0.1f, 0.8f, Difficulty);
}

float AFighterAIController::GetDistanceToOpponent() const
{
	if (!ControlledFighter || !OpponentFighter) return 9999.0f;
	return FVector::Dist(ControlledFighter->GetActorLocation(), OpponentFighter->GetActorLocation());
}
