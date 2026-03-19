#include "CombatGameInstance.h"
#include "CombatGame.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

UCombatGameInstance::UCombatGameInstance()
{
	RoundsToWin = 2;
	RoundTimeLimit = 60.0f;
	bPlayer2IsAI = true;
	AIDifficulty = 0.5f;
}

void UCombatGameInstance::Init()
{
	Super::Init();

	// Load character data from data table if set
	if (UDataTable* DT = CharacterDataTable.LoadSynchronous())
	{
		TArray<FCharacterData*> Rows;
		DT->GetAllRows<FCharacterData>(TEXT("CombatGameInstance"), Rows);
		for (FCharacterData* Row : Rows)
		{
			if (Row)
			{
				AvailableCharacters.Add(*Row);
			}
		}
		UE_LOG(LogCombatGame, Log, TEXT("Loaded %d characters from data table"), AvailableCharacters.Num());
	}

	// Default stages
	AvailableStages.Add(FName("ArenaMap"));
	AvailableStages.Add(FName("TempleMap"));
	AvailableStages.Add(FName("StreetsMap"));
}

void UCombatGameInstance::SetPlayer1Character(int32 Index)
{
	if (AvailableCharacters.IsValidIndex(Index))
	{
		Player1Character = AvailableCharacters[Index];
	}
}

void UCombatGameInstance::SetPlayer2Character(int32 Index)
{
	if (AvailableCharacters.IsValidIndex(Index))
	{
		Player2Character = AvailableCharacters[Index];
	}
}

void UCombatGameInstance::StartMatch()
{
	FName MapName = SelectedStage.IsNone() ? FName("ArenaMap") : SelectedStage;
	FString MapPath = FString::Printf(TEXT("/Game/Maps/%s"), *MapName.ToString());
	UGameplayStatics::OpenLevel(this, FName(*MapPath));
}
