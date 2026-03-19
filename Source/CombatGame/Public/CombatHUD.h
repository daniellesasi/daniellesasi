// CombatHUD.h - Main fighting game HUD
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CombatHUD.generated.h"

UCLASS()
class COMBATGAME_API ACombatHUD : public AHUD
{
	GENERATED_BODY()

public:
	ACombatHUD();
	virtual void BeginPlay() override;

	// Widget class to spawn (set to WBP_FightHUD in BP)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UFightHUDWidget> FightHUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	class UFightHUDWidget* FightHUDWidget;
};
