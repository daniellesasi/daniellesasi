// PauseMenuWidget.h - In-match pause menu
// Create Widget Blueprint: WBP_PauseMenu
// Required widget names:
//   - ResumeButton (Button)
//   - CharSelectButton (Button)
//   - MainMenuButton (Button)
//   - MoveListButton (Button) [optional]
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;

UCLASS()
class COMBATGAME_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* ResumeButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* CharSelectButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* MainMenuButton;

private:
	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnCharSelectClicked();

	UFUNCTION()
	void OnMainMenuClicked();
};
