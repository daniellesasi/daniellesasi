// CombatPlayerController.h - Handles input mapping context setup for fighters
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CombatPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class COMBATGAME_API ACombatPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACombatPlayerController();

	virtual void BeginPlay() override;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupInputComponent() override;

	// The mapping context to add when possessing a fighter
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* FighterMappingContext;

private:
	void AddFighterMappingContext();
	void RemoveFighterMappingContext();
	bool bMappingContextAdded = false;
};
