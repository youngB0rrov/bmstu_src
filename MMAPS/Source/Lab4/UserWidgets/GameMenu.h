// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GameMenu.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API UGameMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetupInGameMenu();

	void TearDownInGameMenu();
	
	UFUNCTION()
	void OnToMainMenuClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UFUNCTION()
	void OnInGameCancelButtonClicked();

	virtual bool Initialize() override;
	
protected:
	UPROPERTY(meta=(BindWidget))
	UButton *ToMainMenuButton;

	UPROPERTY(meta=(BindWidget))
	UButton *ExitGameButton;

	UPROPERTY(meta=(BindWidget))
	UButton *InGameCancelButton;

	const FString TravelMainMenuPath = TEXT("/Game/MainMenu/MainMenuMap");
};
