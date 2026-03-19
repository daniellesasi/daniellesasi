// FightHUDWidget.h - In-fight HUD with health bars, timer, round indicators, combo counter
// Create a Widget Blueprint: WBP_FightHUD based on this class
// Required widget names (bind in UMG designer):
//   - P1HealthBar (ProgressBar)
//   - P2HealthBar (ProgressBar)
//   - TimerText (TextBlock)
//   - RoundText (TextBlock)
//   - P1RoundIndicator1, P1RoundIndicator2 (Image - filled = round won)
//   - P2RoundIndicator1, P2RoundIndicator2 (Image)
//   - ComboCounterText (TextBlock) - shows combo hit count
//   - AnnouncerText (TextBlock) - "ROUND 1", "FIGHT!", "K.O.", etc.
//   - P1NameText (TextBlock)
//   - P2NameText (TextBlock)
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatTypes.h"
#include "FightHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;

UCLASS()
class COMBATGAME_API UFightHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Health bars
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UProgressBar* P1HealthBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UProgressBar* P2HealthBar;

	// Timer
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UTextBlock* TimerText;

	// Round display
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UTextBlock* RoundText;

	// Round win indicators
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UImage* P1RoundIndicator1;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UImage* P1RoundIndicator2;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UImage* P2RoundIndicator1;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UImage* P2RoundIndicator2;

	// Combo counter
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* ComboCounterText;

	// Center announcer text ("ROUND 1", "FIGHT!", "K.O.")
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* AnnouncerText;

	// Player names
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* P1NameText;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* P2NameText;

	// Update functions
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHealthBar(int32 PlayerIndex, float HealthPercent);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateTimer(float TimeRemaining);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateRoundIndicators(int32 P1Wins, int32 P2Wins);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowComboCounter(int32 HitCount);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideComboCounter();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowAnnouncement(const FText& Text, float Duration = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetPlayerNames(const FText& P1Name, const FText& P2Name);

private:
	FTimerHandle AnnouncementTimerHandle;
	void HideAnnouncement();

	// Smooth health bar interpolation
	float P1TargetHealth = 1.0f;
	float P2TargetHealth = 1.0f;
	float P1DisplayHealth = 1.0f;
	float P2DisplayHealth = 1.0f;
	float HealthBarLerpSpeed = 5.0f;
};
