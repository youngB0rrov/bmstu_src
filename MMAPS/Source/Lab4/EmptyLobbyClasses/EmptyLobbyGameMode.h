// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EmptyLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API AEmptyLobbyGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void ServerTravelToGameMap();

	UFUNCTION()
	void StartServerTravel();

	float CountdownStartTime = 0.f;
	float CountdownTime = 5.f;

private:
	FTimerHandle TravelTimer;
	FTimerDelegate TravelTimerDelegate;
};
