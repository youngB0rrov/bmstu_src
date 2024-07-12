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
	AEmptyLobbyGameMode();

	UFUNCTION()
	void ServerTravelToGameMap();

	UFUNCTION()
	void StartServerTravel();

	void CheckPlayersStatuses();

	float CountdownStartTime = 0.f;
	float CountdownTime = 5.f;

private:
	FTimerHandle TravelTimer;
	FTimerDelegate TravelTimerDelegate;
	FString ServerManagerAddress;
	FString DaemonAddress;
	uint32 ServerManagerPort;
	bool bIsReadyToStart = false;

	void ClearServerTravelTimer();
	void SendUriToServerManager(const int32 Port);
	void FromStringToBinaryArray(const FString& Message, TArray<uint8>& OutBinaryArray);

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void BeginPlay() override;
};
