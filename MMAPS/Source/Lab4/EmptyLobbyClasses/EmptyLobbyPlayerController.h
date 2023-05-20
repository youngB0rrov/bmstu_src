// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EmptyLobbyHUD.h"
#include "GameFramework/PlayerController.h"
#include "EmptyLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API AEmptyLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	virtual void ReceivedPlayer() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
private:
	UPROPERTY()
	AEmptyLobbyHUD* EmptyLobbyHUD;

	void PollInit();
	void SetEmptyLobbyHUDTime();
	void SetEmptyLobbyCountdownTimer(uint32 TimeLeft);
	float GetClientServerDelta();
	void CheckPlayersSync(float DeltaSeconds);

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float ClientRequestTime);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float ServerReceitTime);

	UFUNCTION(Server, Reliable)
	void SetLobbyPlayerName(const FString& PlayerName);
	
	bool bStartCountdownTimer = false;
	uint32 CountdownInt = 0;
	float CountdownTime = 0.f;
	float CountdownStartTime = 0.f;
	float ClientServerDelta = 0.f;
	float TimeSyncRunning = 0.f;
	float TimeSyncFrequency = 3.f;

	bool bHUDIsBeingInitialized = false;
	
public:
	UFUNCTION(Client, Reliable)
	void DeleteEmptyLobbyHUD();

	UFUNCTION(Server, Reliable)
	void ServerGetEmptyLobbySettings();

	UFUNCTION(Client, Reliable)
	void ClientGetEmptyLobbySettings(float CountdownServerTime);

	UFUNCTION(Client, Reliable)
	void ClientAddCountdownTimer(float CountdownServerStartTime);

	UFUNCTION(Client, Reliable)
	void ClientRefreshPlayersGrid();
};
