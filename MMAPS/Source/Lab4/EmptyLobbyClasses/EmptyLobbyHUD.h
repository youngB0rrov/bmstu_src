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

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> StatusGridWidgetClass;
	
public:
	AEmptyLobbyHUD();
	void AddStatusControllOverlay();
	void AddStatusGrid();
	void RemoveControllOverlay();
	void SetTimerVisability();
	void RefreshGrid();

	UPROPERTY()
	class UStatusControll* StatusControllWidget;

	UPROPERTY()
	class UStatusGrid* StatusGridWidget;
};
