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
	~AEmptyLobbyGameMode();

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
	uint32 ServerManagerServersPort;
	bool bIsReadyToStart = false;
	FSocket* ConnectionSocket;

	void ClearServerTravelTimer();
	void SendUriToServerManager(const int32 Port);
	void RegisterPlayersInfoByteForServerManager(const int32 Port);
	void UpdatePlayersInfoForServerManager();
	void UpdatePlayersInfoByteForServerManager();
	void FromStringToBinaryArray(const FString& Message, TArray<uint8>& OutBinaryArray);
	void SendMessageWithSocket(const FString& Message);
	bool InitializeSocketToServerManager();
	bool ReconnectToServerManager();
	FGuid GetServerInstanceUuid();

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void BeginPlay() override;
};
