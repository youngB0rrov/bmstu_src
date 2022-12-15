// Fill out your copyright notice in the Description page of Project Settings.


#include "CurrentPlayerState.h"

ACurrentPlayerState::ACurrentPlayerState()
{
	SetScore(0.0f);
}

void ACurrentPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	OnScoreUpdate();
}

void ACurrentPlayerState::OnScoreUpdate()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
		FString::Printf(TEXT("You now have %f frags!"), GetScore()));
}

void ACurrentPlayerState::SetCurrentScore()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		const float PlayerRepScore = GetScore();
		SetScore(PlayerRepScore + 1.0f);
		OnScoreUpdate();
	}
}
