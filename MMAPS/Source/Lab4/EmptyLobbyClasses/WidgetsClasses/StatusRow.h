// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Lab4/EmptyLobbyClasses/EmptyLobbyPlayerState.h"
#include "StatusRow.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class LAB4_API UStatusRow : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPlayerRowText(AEmptyLobbyPlayerState* CurrentPlayerState);

private:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* PlayerPingText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* PlayerStatusText;
	
};
