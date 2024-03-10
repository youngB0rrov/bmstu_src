// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RankedLeaderboardRow.generated.h"

/**
 * 
 */
class UTextBlock;

UCLASS()
class LAB4_API URankedLeaderboardRow : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* RankText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* NameText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* GlobalRankText;

public:
	void SetRankedRowText(const uint32 Rank, const FString Name, int32 Score, uint32 GlobalRank);
	
};
