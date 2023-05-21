// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Lab4/EmptyLobbyClasses/EmptyLobbyPlayerController.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"
#include "StatusControll.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class LAB4_API UStatusControll : public UUserWidget, public FTickableGameObject
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	
public:
	UFUNCTION()
	void OnStartButtonMatchedClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UFUNCTION()
	void OnPlayerButtonDelegates();

	UFUNCTION()
	void OnPlayerReadyButtonPressed();

	UFUNCTION()
	void OnPlayerButtonReleased();

	UFUNCTION()
	void OnPlayerCancelButtonPressed();
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* StartTimerInfoText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* StartTimerText;

	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* CancelationMessageBox;
	
private:
	UPROPERTY(meta=(BindWidget))
	UButton* PlayerReadyButton;

	UPROPERTY(meta=(BindWidget))
	UButton* PlayerCancelButton;

	UPROPERTY(meta=(BindWidget))
	UButton* StartMatchButton;

	UPROPERTY(meta=(BindWidget))
	UButton* ExitButton;

	UPROPERTY(meta=(BindWidget))
	UProgressBar* PressButtonProgressbar;

	UPROPERTY()
	AEmptyLobbyPlayerController* EmptyLobbyOwningController;

	const FString TravelMainMenuPath = TEXT("/Game/MainMenu/MainMenuMap");

	FTimerHandle PressButtonHandle;
	FTimerDelegate PressButtonDelegate;

	UPROPERTY()
	ULab4GameInstance* GameInstance;

	float PressButtonDuration = 2.f;
	bool bIsProgressbarVisible = false;

	void SetPorgressbarPercantage();

	uint16 PressedButton;
};
