#include "CharacterSelectWidget.h"
#include "CombatGameInstance.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void UCharacterSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReadyButton) ReadyButton->OnClicked.AddDynamic(this, &UCharacterSelectWidget::OnReadyClicked);
	if (BackButton) BackButton->OnClicked.AddDynamic(this, &UCharacterSelectWidget::OnBackClicked);

	PopulateGrid();

	// Default selections
	SelectCharacter(0, 0);
	SelectCharacter(1, 0);
}

void UCharacterSelectWidget::PopulateGrid()
{
	UCombatGameInstance* GI = Cast<UCombatGameInstance>(GetGameInstance());
	if (!GI || !CharacterGrid) return;

	// Character portraits are populated in Blueprint using PortraitButtonClass
	// Each portrait button calls SelectCharacter when clicked
	// You can also do this purely in C++ if you prefer
}

void UCharacterSelectWidget::SelectCharacter(int32 PlayerIndex, int32 CharacterIndex)
{
	UCombatGameInstance* GI = Cast<UCombatGameInstance>(GetGameInstance());
	if (!GI) return;

	if (!GI->AvailableCharacters.IsValidIndex(CharacterIndex)) return;

	const FCharacterData& CharData = GI->AvailableCharacters[CharacterIndex];

	if (PlayerIndex == 0)
	{
		P1SelectedIndex = CharacterIndex;
		GI->SetPlayer1Character(CharacterIndex);
		if (P1NameText) P1NameText->SetText(CharData.DisplayName);
		if (P1SelectedImage)
		{
			if (UTexture2D* Tex = CharData.FullBodyRender.LoadSynchronous())
			{
				P1SelectedImage->SetBrushFromTexture(Tex);
			}
		}
	}
	else
	{
		P2SelectedIndex = CharacterIndex;
		GI->SetPlayer2Character(CharacterIndex);
		if (P2NameText) P2NameText->SetText(CharData.DisplayName);
		if (P2SelectedImage)
		{
			if (UTexture2D* Tex = CharData.FullBodyRender.LoadSynchronous())
			{
				P2SelectedImage->SetBrushFromTexture(Tex);
			}
		}
	}

	UpdateSelection(PlayerIndex, CharacterIndex);
}

void UCharacterSelectWidget::UpdateSelection(int32 PlayerIndex, int32 CharacterIndex)
{
	// Visual feedback - highlight selected portrait in grid
	// Override in Blueprint for custom highlight effects
}

void UCharacterSelectWidget::CycleStage(bool bNext)
{
	UCombatGameInstance* GI = Cast<UCombatGameInstance>(GetGameInstance());
	if (!GI || GI->AvailableStages.Num() == 0) return;

	if (bNext)
	{
		CurrentStageIndex = (CurrentStageIndex + 1) % GI->AvailableStages.Num();
	}
	else
	{
		CurrentStageIndex = (CurrentStageIndex - 1 + GI->AvailableStages.Num()) % GI->AvailableStages.Num();
	}

	GI->SelectedStage = GI->AvailableStages[CurrentStageIndex];
	if (StageNameText)
	{
		StageNameText->SetText(FText::FromName(GI->AvailableStages[CurrentStageIndex]));
	}
}

void UCharacterSelectWidget::OnReadyClicked()
{
	UCombatGameInstance* GI = Cast<UCombatGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->StartMatch();
	}
}

void UCharacterSelectWidget::OnBackClicked()
{
	UGameplayStatics::OpenLevel(this, FName("/Game/Maps/MainMenuMap"));
}
