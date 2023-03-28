// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTableRow.h"

#include "Components/TextBlock.h"

void UPlayerTableRow::SetPlayerRowText(ALab4PlayerState* PlayerState, uint32 PlayerIndex)
{
	FString Rank = FString::Printf(TEXT("%d"), PlayerIndex);
	FString Name;
	FString Score;
	
	if (PlayerState)
	{
		Name = FString::Printf(TEXT("%s"), *PlayerState->GetPlayerName());
		Score = FString::Printf(TEXT("%d"), FMath::FloorToInt(PlayerState->GetPlayerScore()));
		UE_LOG(LogTemp, Error, TEXT("PlayerName: %s, PlayerScore: %s"), *Name, *Score);
	}

	if (PlayerRankText)
	{
		PlayerRankText->SetText(FText::FromString(Rank));
	}

	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(Name));
	}

	if (PlayerScoreText)
	{
		PlayerScoreText->SetText(FText::FromString(Score));
	}
}

void UPlayerTableRow::SetRankedRowText(int32 Rank, FString NickName, int32 Score)
{
	FString RankString = FString::Printf(TEXT("%d"), Rank);
	FString ScoreString = FString::Printf(TEXT("%d"), Score);

	if (PlayerRankText)
	{
		PlayerRankText->SetText(FText::FromString(RankString));
	}

	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(NickName));
	}

	if (PlayerScoreText)
	{
		PlayerScoreText->SetText(FText::FromString(ScoreString));
	}
}
