#include "FightHUDWidget.h"
#include "CombatGameMode.h"
#include "CombatCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UFightHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind to game mode events
	if (ACombatGameMode* GM = Cast<ACombatGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->OnTimerUpdated.AddDynamic(this, &UFightHUDWidget::UpdateTimer);
		GM->OnFighterDamaged.AddDynamic(this, &UFightHUDWidget::UpdateHealthBar);

		GM->OnRoundStarted.AddLambda([this](int32 RoundNumber)
		{
			ShowAnnouncement(FText::Format(
				FTextFormat::FromString(TEXT("ROUND {0}")), FText::AsNumber(RoundNumber)), 2.0f);
		});

		GM->OnMatchStateChanged.AddLambda([this](EMatchState NewState)
		{
			if (NewState == EMatchState::Fighting)
			{
				ShowAnnouncement(FText::FromString(TEXT("FIGHT!")), 1.5f);
			}
			else if (NewState == EMatchState::RoundEnd)
			{
				ShowAnnouncement(FText::FromString(TEXT("K.O.")), 2.0f);
			}
		});

		GM->OnRoundEnded.AddLambda([this](int32 RoundNumber, ERoundResult Result)
		{
			if (ACombatGameMode* GM2 = Cast<ACombatGameMode>(UGameplayStatics::GetGameMode(this)))
			{
				UpdateRoundIndicators(GM2->Player1RoundWins, GM2->Player2RoundWins);
			}
		});

		GM->OnMatchEnded.AddLambda([this](ERoundResult Result)
		{
			FText WinText;
			if (Result == ERoundResult::Player1Win)
				WinText = FText::FromString(TEXT("PLAYER 1 WINS"));
			else if (Result == ERoundResult::Player2Win)
				WinText = FText::FromString(TEXT("PLAYER 2 WINS"));
			else
				WinText = FText::FromString(TEXT("DRAW"));
			ShowAnnouncement(WinText, 5.0f);
		});
	}

	// Initialize
	if (ComboCounterText) ComboCounterText->SetVisibility(ESlateVisibility::Collapsed);
	if (AnnouncerText) AnnouncerText->SetVisibility(ESlateVisibility::Collapsed);
}

void UFightHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Smooth health bar interpolation
	P1DisplayHealth = FMath::FInterpTo(P1DisplayHealth, P1TargetHealth, InDeltaTime, HealthBarLerpSpeed);
	P2DisplayHealth = FMath::FInterpTo(P2DisplayHealth, P2TargetHealth, InDeltaTime, HealthBarLerpSpeed);

	if (P1HealthBar) P1HealthBar->SetPercent(P1DisplayHealth);
	if (P2HealthBar) P2HealthBar->SetPercent(P2DisplayHealth);
}

void UFightHUDWidget::UpdateHealthBar(int32 PlayerIndex, float HealthPercent)
{
	if (PlayerIndex == 0)
	{
		P1TargetHealth = HealthPercent;
	}
	else
	{
		P2TargetHealth = HealthPercent;
	}
}

void UFightHUDWidget::UpdateTimer(float TimeRemaining)
{
	if (TimerText)
	{
		int32 Seconds = FMath::CeilToInt(TimeRemaining);
		TimerText->SetText(FText::AsNumber(Seconds));
	}
}

void UFightHUDWidget::UpdateRoundIndicators(int32 P1Wins, int32 P2Wins)
{
	FLinearColor WinColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
	FLinearColor EmptyColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);

	if (P1RoundIndicator1)
		P1RoundIndicator1->SetColorAndOpacity(P1Wins >= 1 ? WinColor : EmptyColor);
	if (P1RoundIndicator2)
		P1RoundIndicator2->SetColorAndOpacity(P1Wins >= 2 ? WinColor : EmptyColor);
	if (P2RoundIndicator1)
		P2RoundIndicator1->SetColorAndOpacity(P2Wins >= 1 ? WinColor : EmptyColor);
	if (P2RoundIndicator2)
		P2RoundIndicator2->SetColorAndOpacity(P2Wins >= 2 ? WinColor : EmptyColor);
}

void UFightHUDWidget::ShowComboCounter(int32 HitCount)
{
	if (ComboCounterText)
	{
		FText CountText = FText::Format(
			FTextFormat::FromString(TEXT("{0} HITS")), FText::AsNumber(HitCount));
		ComboCounterText->SetText(CountText);
		ComboCounterText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UFightHUDWidget::HideComboCounter()
{
	if (ComboCounterText)
	{
		ComboCounterText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFightHUDWidget::ShowAnnouncement(const FText& Text, float Duration)
{
	if (AnnouncerText)
	{
		AnnouncerText->SetText(Text);
		AnnouncerText->SetVisibility(ESlateVisibility::HitTestInvisible);

		GetWorld()->GetTimerManager().ClearTimer(AnnouncementTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(AnnouncementTimerHandle,
			this, &UFightHUDWidget::HideAnnouncement, Duration, false);
	}
}

void UFightHUDWidget::HideAnnouncement()
{
	if (AnnouncerText)
	{
		AnnouncerText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFightHUDWidget::SetPlayerNames(const FText& P1Name, const FText& P2Name)
{
	if (P1NameText) P1NameText->SetText(P1Name);
	if (P2NameText) P2NameText->SetText(P2Name);
}
