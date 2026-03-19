// CombatGameInstance.h - Persists data between levels (character selection, settings)
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CombatTypes.h"
#include "CombatGameInstance.generated.h"

UCLASS()
class COMBATGAME_API UCombatGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UCombatGameInstance();

	// Selected character data for each player
	UPROPERTY(BlueprintReadWrite, Category = "Match")
	FCharacterData Player1Character;

	UPROPERTY(BlueprintReadWrite, Category = "Match")
	FCharacterData Player2Character;

	// Is Player 2 controlled by AI?
	UPROPERTY(BlueprintReadWrite, Category = "Match")
	bool bPlayer2IsAI = true;

	// AI difficulty (0.0 = easy, 1.0 = hardest)
	UPROPERTY(BlueprintReadWrite, Category = "Match")
	float AIDifficulty = 0.5f;

	// Number of rounds to win
	UPROPERTY(BlueprintReadWrite, Category = "Match")
	int32 RoundsToWin = 2;

	// Round time limit in seconds (0 = infinite)
	UPROPERTY(BlueprintReadWrite, Category = "Match")
	float RoundTimeLimit = 60.0f;

	// Selected stage/arena map name
	UPROPERTY(BlueprintReadWrite, Category = "Match")
	FName SelectedStage;

	// Master volume (0.0 - 1.0)
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float MasterVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float MusicVolume = 0.8f;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float SFXVolume = 1.0f;

	// All available characters (loaded from data table)
	UPROPERTY(BlueprintReadOnly, Category = "Characters")
	TArray<FCharacterData> AvailableCharacters;

	// Available stage names
	UPROPERTY(BlueprintReadOnly, Category = "Stages")
	TArray<FName> AvailableStages;

	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetPlayer1Character(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetPlayer2Character(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void StartMatch();

	// Character data table reference
	// Create a DataTable using FCharacterData row struct
	// Name: DT_Characters
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TSoftObjectPtr<class UDataTable> CharacterDataTable;
};
