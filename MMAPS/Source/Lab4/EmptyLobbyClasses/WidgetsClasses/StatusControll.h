// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Lab4/EmptyLobbyClasses/EmptyLobbyPlayerController.h"
#include "StatusControll.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class LAB4_API UStatusControll : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;

public:
	UFUNCTION()
	void OnStartButtonMatchedClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UPROPERTY(meta=(BindWidget))
	UTextBlock* StartTimerInfoText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* StartTimerText;
	
private:
	UPROPERTY(meta=(BindWidget))
	UButton* PlayerReadyButton;

	UPROPERTY(meta=(BindWidget))
	UButton* PlayerCancelButton;

	UPROPERTY(meta=(BindWidget))
	UButton* StartMatchButton;

	UPROPERTY(meta=(BindWidget))
	UButton* ExitButton;

	UPROPERTY()
	AEmptyLobbyPlayerController* EmptyLobbyOwningController;

	const FString TravelMainMenuPath = TEXT("/Game/MainMenu/MainMenuMap");
};
