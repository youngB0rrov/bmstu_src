// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GameOverMenu.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API UGameOverMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetupGameOverMenu();
	void TeardownGameOverMenu();
	
	UPROPERTY(meta = (BindWidget))
	UButton* GameOverMenuRetryButton;

	UPROPERTY(meta = (BindWidget))
	UButton* GameOverMenuExitButton;
	
protected:
	UFUNCTION()
	void OnRetryButtonClicked();
	
	UFUNCTION()
	void OnExitButtonClicked();

	virtual bool Initialize() override;
private:
	const FString TravelMainMenuPath = TEXT("/Game/MainMenu/MainMenuMap");
};
