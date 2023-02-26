// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Lab4/Lab4PlayerState.h"
#include "MyUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetWinnerText(const ALab4PlayerState* WinnerPlayerState);
	void SetRestartTimer(int32 CountdownTime);
	void HideWinnerWidget();
	
private:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* WinnerNameTextBox;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* GameOverTitle;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* RestartDigitsTextBlock;
};
