#include "PauseMenuWidget.h"
#include "CombatGameMode.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton) ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	if (CharSelectButton) CharSelectButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnCharSelectClicked);
	if (MainMenuButton) MainMenuButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
}

void UPauseMenuWidget::OnResumeClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	RemoveFromParent();
}

void UPauseMenuWidget::OnCharSelectClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	if (ACombatGameMode* GM = Cast<ACombatGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->ReturnToCharacterSelect();
	}
}

void UPauseMenuWidget::OnMainMenuClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	if (ACombatGameMode* GM = Cast<ACombatGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->ReturnToMainMenu();
	}
}
