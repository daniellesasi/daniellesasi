#include "CombatHUD.h"
#include "FightHUDWidget.h"
#include "Blueprint/UserWidget.h"

ACombatHUD::ACombatHUD()
{
}

void ACombatHUD::BeginPlay()
{
	Super::BeginPlay();

	if (FightHUDWidgetClass)
	{
		FightHUDWidget = CreateWidget<UFightHUDWidget>(GetOwningPlayerController(), FightHUDWidgetClass);
		if (FightHUDWidget)
		{
			FightHUDWidget->AddToViewport();
		}
	}
}
