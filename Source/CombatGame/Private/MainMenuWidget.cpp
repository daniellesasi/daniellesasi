#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayButton) PlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClicked);
	if (OptionsButton) OptionsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);
	if (QuitButton) QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
}

void UMainMenuWidget::OnPlayClicked()
{
	UGameplayStatics::OpenLevel(this, FName("/Game/Maps/CharacterSelectMap"));
}

void UMainMenuWidget::OnOptionsClicked()
{
	// Override in Blueprint to show options panel
}

void UMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}
