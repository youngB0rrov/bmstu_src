// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyHostWidget.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API ULobbyHostWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta=(BindWidget))
	class UButton* LobbyStartButton;

public:
	UFUNCTION()
	void OnLobbyStartButtonClicked();

protected:
	virtual bool Initialize() override;
	
};
