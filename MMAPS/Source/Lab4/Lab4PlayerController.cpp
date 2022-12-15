// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab4PlayerController.h"

#include "Lab4HUD.h"
#include "Lab4PlayerState.h"
#include "UserWidgets/PlayerHealthBar.h"

void ALab4PlayerController::BeginPlay()
{
	Super::BeginPlay();
	Lab4HUD = Cast<ALab4HUD>(GetHUD());
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
