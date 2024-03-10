// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab4PlayerState.h"
#include "Lab4Character.h"

void ALab4PlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	const FVector2D TextMessageLocation = FVector2D(3.0f);
	// Обновление хада для клиентов
	Character = (Character == nullptr) ? Cast<ALab4Character>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Lab4PlayerController = Lab4PlayerController == nullptr ? Cast<ALab4PlayerController>(Character->Controller) : Lab4PlayerController;
	}
	if (Lab4PlayerController)
	{
		Lab4PlayerController->SetHUDScore(GetPlayerScore());
	}
}

void ALab4PlayerState::AddToScore(float ScoreAmount)
{
	const float Lab4PlayerScore = GetScore();
	SetScore(Lab4PlayerScore + ScoreAmount);
	// Обновление хада для клиента
	Character = (Character == nullptr) ? Cast<ALab4Character>(GetPawn()) : Character;
	
	if (Character)
	{
		Lab4PlayerController = Lab4PlayerController == nullptr ? Cast<ALab4PlayerController>(Character->Controller) : Lab4PlayerController;
	}
	
	if (Lab4PlayerController)
	{
		Lab4PlayerController->SetHUDScore(GetPlayerScore());
	}
}

void ALab4PlayerState::AddToName(FString Name)
{
	SetPlayerName(Name);
}
