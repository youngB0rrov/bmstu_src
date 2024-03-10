// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Lab4/Lab4PlayerState.h"
#include "PlayerTableRow.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API UPlayerTableRow : public UUserWidget
{
	GENERATED_BODY()
private:
	UPROPERTY(meta=(BindWidget))
	class UHorizontalBox* PlayerRowContainer;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* PlayerRankText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* PlayerScoreText;

public:
	void SetPlayerRowText(ALab4PlayerState* PlayerState, uint32 PlayerIndex);
	void SetRankedRowText(int32 Rank, FString NickName, int32 Score);
};
