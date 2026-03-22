// FighterAIController.h - AI opponent with difficulty-scaled behavior
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CombatTypes.h"
#include "FighterAIController.generated.h"

UCLASS()
class COMBATGAME_API AFighterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AFighterAIController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 0.0 = easy (slow reactions, rarely blocks, basic attacks)
	// 1.0 = hard (fast reactions, optimal blocking, combos, punishment)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Difficulty = 0.5f;

	// Reference to controlled fighter
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	class ACombatCharacter* ControlledFighter;

	// Reference to opponent
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	class ACombatCharacter* OpponentFighter;

private:
	// AI behavior state
	enum class EAIBehavior : uint8
	{
		Idle,
		Approaching,
		Attacking,
		Blocking,
		Retreating,
		Sidestepping,
		Punishing,    // Opponent is in recovery
		Pressuring,   // Opponent is in blockstun
		WakeupAttack  // Opponent getting up
	};

	EAIBehavior CurrentBehavior = EAIBehavior::Idle;

	void DecideBehavior();
	void ExecuteBehavior(float DeltaTime);

	void PerformApproach(float DeltaTime);
	void PerformAttack();
	void PerformBlock();
	void PerformRetreat(float DeltaTime);
	void PerformSidestep();
	void PerformPunish();
	void PerformPressure();
	void PerformWakeupAttack();

	// Selects a random attack weighted by difficulty
	FAttackData SelectAttack();

	// Difficulty-scaled reaction time (seconds)
	float GetReactionTime() const;

	// Probability of choosing the correct block type
	float GetBlockAccuracy() const;

	// Probability of attempting a punish on opponent recovery
	float GetPunishChance() const;

	// Distance to opponent
	float GetDistanceToOpponent() const;

	// Timers
	float DecisionTimer = 0.0f;
	float DecisionInterval = 0.15f; // How often AI re-evaluates
	float ActionCooldown = 0.0f;

	// Range thresholds
	float CloseRange = 200.0f;
	float MidRange = 350.0f;
	float FarRange = 500.0f;
};
