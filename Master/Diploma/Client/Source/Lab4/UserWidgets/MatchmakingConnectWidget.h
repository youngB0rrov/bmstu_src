// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchmakingConnectWidget.generated.h"

class UButton;
class USizeBox;
/**
 * 
 */
UCLASS()
class LAB4_API UMatchmakingConnectWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	UButton* MatchmakingConnectButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MatchmakingCancelConnectButton;

	UPROPERTY(meta = (BindWidget))
	USizeBox* MatchmakingConnectionSpinner;

	void SetMatchmakingSpinnerVisibility(bool bIsVisible);

public:
	UFUNCTION()
	void OnMatchmakingConnectButtonClicked();

	UFUNCTION()
	void OnMatchmakingCancelConnectButtonClicked();

protected:
	virtual bool Initialize() override;
};
