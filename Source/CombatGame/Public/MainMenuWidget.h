// MainMenuWidget.h - Main menu with play, options, quit
// Create Widget Blueprint: WBP_MainMenu based on this class
// Required widget names:
//   - PlayButton (Button)
//   - OptionsButton (Button)
//   - QuitButton (Button)
//   - TitleText (TextBlock)
//   - VersionText (TextBlock) [optional]
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class COMBATGAME_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* PlayButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* OptionsButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* QuitButton;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* TitleText;

private:
	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnOptionsClicked();

	UFUNCTION()
	void OnQuitClicked();
};
