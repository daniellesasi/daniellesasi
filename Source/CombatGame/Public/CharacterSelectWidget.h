// CharacterSelectWidget.h - Character selection screen
// Create Widget Blueprint: WBP_CharacterSelect based on this class
// Required widget names:
//   - CharacterGrid (UniformGridPanel) - portraits are added here dynamically
//   - P1SelectedImage (Image) - full body render of P1's selection
//   - P2SelectedImage (Image) - full body render of P2's selection
//   - P1NameText (TextBlock)
//   - P2NameText (TextBlock)
//   - ReadyButton (Button) - starts the match
//   - BackButton (Button) - returns to main menu
//   - StageNameText (TextBlock) [optional]
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatTypes.h"
#include "CharacterSelectWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UUniformGridPanel;

UCLASS()
class COMBATGAME_API UCharacterSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UUniformGridPanel* CharacterGrid;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UImage* P1SelectedImage;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UImage* P2SelectedImage;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* P1NameText;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* P2NameText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* ReadyButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BackButton;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* StageNameText;

	// Portrait button widget class (create as WBP_CharPortraitButton)
	// Should contain: PortraitImage (Image), SelectButton (Button)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> PortraitButtonClass;

	UFUNCTION(BlueprintCallable, Category = "Character Select")
	void SelectCharacter(int32 PlayerIndex, int32 CharacterIndex);

	UFUNCTION(BlueprintCallable, Category = "Character Select")
	void CycleStage(bool bNext);

private:
	UFUNCTION()
	void OnReadyClicked();

	UFUNCTION()
	void OnBackClicked();

	void PopulateGrid();
	void UpdateSelection(int32 PlayerIndex, int32 CharacterIndex);

	int32 P1SelectedIndex = 0;
	int32 P2SelectedIndex = 0;
	bool bP1Ready = false;
	bool bP2Ready = false;
	int32 CurrentStageIndex = 0;
};
