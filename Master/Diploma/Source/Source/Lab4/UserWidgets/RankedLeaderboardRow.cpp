// Fill out your copyright notice in the Description page of Project Settings.


#include "RankedLeaderboardRow.h"

#include "Components/TextBlock.h"

void URankedLeaderboardRow::SetRankedRowText(const uint32 Rank, const FString Name, int32 Score, uint32 GlobalRank)
{
	if (RankText != nullptr)
	{
		RankText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Rank)));
	}

	if (NameText != nullptr)
	{
		NameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *Name)));
	}

	if (ScoreText != nullptr)
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Score)));
	}
	
	if (GlobalRankText != nullptr)
	{
		GlobalRankText->SetText(FText::FromString(FString::Printf(TEXT("%d"), GlobalRank)));
	}
}
