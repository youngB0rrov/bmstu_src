// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WinnerWiddget.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API UWinnerWiddget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SetupWinnerWidget();

	UFUNCTION()
	void RemoveWinnerWidget();

	UFUNCTION()
	void OnNewGameButtonClicked();

	UFUNCTION()
	void OnMainMenuButtonClicked();

	void SetupWinnerName(FString PlayerName);

protected:
	UPROPERTY(meta=(BindWidget))
	class UButton* GameOverNewGameButton;

	UPROPERTY(meta=(BindWidget))
	class UButton* GameOverMenuButton;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* WinnerNameText;
	
	virtual bool Initialize() override;

private:
	const FString TravelMainMenuPath = TEXT("/Game/MainMenu/MainMenuMap");
};
