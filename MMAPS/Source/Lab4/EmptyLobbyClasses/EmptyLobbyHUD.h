// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EmptyLobbyHUD.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API AEmptyLobbyHUD : public AHUD
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> StatusControlWidgetClass;
	
public:
	AEmptyLobbyHUD();
	void AddStatusControllOverlay();
	void RemoveControllOverlay();
	void SetTimerVisability();

	UPROPERTY()
	class UStatusControll* StatusControllWidget;
};
