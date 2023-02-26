// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab4PlayerController.h"

#include "Lab4Character.h"
#include "Lab4GameMode.h"
#include "Lab4HUD.h"
#include "Lab4PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UserWidgets/MyUserWidget.h"
#include "UserWidgets/PlayerHealthBar.h"

void ALab4PlayerController::BeginPlay()
{
	Super::BeginPlay();
	Lab4HUD = Cast<ALab4HUD>(GetHUD());
	ResetCountdownTime = 6;
}

void ALab4PlayerController::SetRestartCountdownTime(int32 CountdownTime)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;

	if (Lab4HUD &&
		Lab4HUD->GameOverWidget
		)
	{
		Lab4HUD->GameOverWidget->SetRestartTimer(CountdownTime);
	}
}

void ALab4PlayerController::SetRestartCountdownTimer()
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("DecreaseCountdownTime"));
	
	GetWorldTimerManager().SetTimer(
		CountdownTimer,
		TimerDelegate,
		1.0f,
		true,
		0.0f
		);
}

void ALab4PlayerController::DecreaseCountdownTime()
{
	SetRestartCountdownTime(--ResetCountdownTime);

	if (ResetCountdownTime <= 0)
	{
		Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;
		GetWorldTimerManager().ClearTimer(CountdownTimer);
		ResetCountdownTime = 6;

		if (Lab4HUD &&
			Lab4HUD->GameOverWidget
			)
		{
			Lab4HUD->GameOverWidget->HideWinnerWidget();
		}
	}
}

void ALab4PlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;
	if (Lab4HUD &&
		Lab4HUD->PlayerHealthBar &&
		Lab4HUD->PlayerHealthBar->HealthProgressBar &&
		Lab4HUD->PlayerHealthBar->HealthText
		)
	{
		const float HealthPersantage = Health / MaxHealth;
		Lab4HUD->PlayerHealthBar->HealthProgressBar->SetPercent(HealthPersantage);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::FloorToInt(Health), FMath::FloorToInt(MaxHealth));
		Lab4HUD->PlayerHealthBar->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ALab4PlayerController::SetHUDScore(float Score)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;
	if (Lab4HUD &&
		Lab4HUD->PlayerHealthBar &&
		Lab4HUD->PlayerHealthBar->ScoreText)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		Lab4HUD->PlayerHealthBar->ScoreText->SetText(FText::FromString(ScoreText));
	}
}

void ALab4PlayerController::BroadcastAnnouncement(APlayerState* AttackerPlayerState,
	APlayerState* VictimPlayerState)
{
	ClientElimAnnouncement(AttackerPlayerState, VictimPlayerState);
}

void ALab4PlayerController::BroadcastGameOverAnnouncement(ALab4PlayerState* WinnerPlayerState)
{
	if (WinnerPlayerState)
	{
		ClientGameOverToggle(WinnerPlayerState);
	}
}

void ALab4PlayerController::ClientGameOverToggle_Implementation(ALab4PlayerState* WinnerPlayerState)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;

	if (Lab4HUD && WinnerPlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("Show UI winner"))
		Lab4HUD->ShowGameOverWidget(WinnerPlayerState);
		SetRestartCountdownTimer();
	}
}

void ALab4PlayerController::ClientElimAnnouncement_Implementation(APlayerState* AttackerPlayerState,
                                                                  APlayerState* VictimPlayerState)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;
	APlayerState* Self = GetPlayerState<APlayerState>();
	
	if (Self && AttackerPlayerState && VictimPlayerState)
	{
		if (Lab4HUD)
		{
			if (AttackerPlayerState == Self && Self != VictimPlayerState)
			{
				// for attacker
				Lab4HUD->AddElimOverlay("You", VictimPlayerState->GetPlayerName());
				return;
			}
			if (VictimPlayerState == Self && Self != AttackerPlayerState)
			{
				// for victim
				Lab4HUD->AddElimOverlay(AttackerPlayerState->GetPlayerName(), "you");
				return;
			}
			if (VictimPlayerState == Self && AttackerPlayerState == Self)
			{
				// suicide
				Lab4HUD->AddElimOverlay("You", "yourself");
			}
			Lab4HUD->AddElimOverlay(AttackerPlayerState->GetPlayerName(), VictimPlayerState->GetPlayerName());
		}
	}
}
