// CombatGameMode.h - Controls round flow, timer, win conditions
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CombatTypes.h"
#include "CombatGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchStateChanged, EMatchState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundStarted, int32, RoundNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoundEnded, int32, RoundNumber, ERoundResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchEnded, ERoundResult, FinalResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdated, float, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFighterDamaged, int32, PlayerIndex, float, NewHealthPercent);

UCLASS()
class COMBATGAME_API ACombatGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACombatGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Prevent UE from auto-spawning a default pawn — we spawn fighters manually
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	// Current match state
	UPROPERTY(BlueprintReadOnly, Category = "Match")
	EMatchState CurrentMatchState;

	// Round tracking
	UPROPERTY(BlueprintReadOnly, Category = "Match")
	int32 CurrentRound;

	UPROPERTY(BlueprintReadOnly, Category = "Match")
	int32 Player1RoundWins;

	UPROPERTY(BlueprintReadOnly, Category = "Match")
	int32 Player2RoundWins;

	UPROPERTY(BlueprintReadOnly, Category = "Match")
	int32 RoundsToWin;

	// Timer
	UPROPERTY(BlueprintReadOnly, Category = "Match")
	float RoundTimeRemaining;

	UPROPERTY(BlueprintReadOnly, Category = "Match")
	float RoundTimeLimit;

	// Fighter references
	UPROPERTY(BlueprintReadOnly, Category = "Fighters")
	class ACombatCharacter* Fighter1;

	UPROPERTY(BlueprintReadOnly, Category = "Fighters")
	class ACombatCharacter* Fighter2;

	// Round history
	UPROPERTY(BlueprintReadOnly, Category = "Match")
	TArray<FRoundData> RoundHistory;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMatchStateChanged OnMatchStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoundStarted OnRoundStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoundEnded OnRoundEnded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMatchEnded OnMatchEnded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTimerUpdated OnTimerUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFighterDamaged OnFighterDamaged;

	// Called when a fighter takes damage
	UFUNCTION(BlueprintCallable, Category = "Match")
	void ReportDamage(class ACombatCharacter* DamagedFighter, float Damage);

	// Called when a fighter dies
	UFUNCTION(BlueprintCallable, Category = "Match")
	void ReportKO(class ACombatCharacter* DefeatedFighter);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void StartRound();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void EndRound(ERoundResult Result);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void EndMatch(ERoundResult FinalResult);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void ReturnToCharacterSelect();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void ReturnToMainMenu();

private:
	void SpawnFighters();
	void SetMatchState(EMatchState NewState);
	void HandleRoundIntro();
	void HandleFighting(float DeltaTime);
	void HandleRoundEnd();
	void ResetFightersForRound();
	ERoundResult DetermineTimeOutWinner();

	FTimerHandle RoundIntroTimerHandle;
	FTimerHandle RoundEndTimerHandle;

	// Spawn points for fighters
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	FVector Player1SpawnLocation = FVector(-300.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	FVector Player2SpawnLocation = FVector(300.0f, 0.0f, 0.0f);

	float RoundIntroTime = 3.0f;
	float RoundEndTime = 3.0f;
};
