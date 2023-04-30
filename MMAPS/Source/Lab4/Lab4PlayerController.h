// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Lab4PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API ALab4PlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	class ALab4HUD* Lab4HUD;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void ReceivedPlayer() override;
	void PollInit();

	/**
	 * Возвращает время сервера
	 */
	virtual float GetServerTime();
	
	/**
	 * @param float TimeOfClientRequest - текущее время клиента
	 */
	// Запрашивает время сервера, принимая в качестве параметра время, когда бьл отправлен запрос с клиента
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Посылает текущее время сервера клиенту в ответ на вызов RPC ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	UFUNCTION(Server, Reliable)
	void ServerGetGameModeSettings();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float LevelTime, float WarmupTime);
	
public:
	void SetRestartCountdownTime(int32 CountdownTime);
	void SetRestartCountdownTimer();
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetStartMatchCountdownTime(float CountdownTime);
	void SetHUDTime();
	void BroadcastAnnouncement(APlayerState* AttackerPlayerState, APlayerState* VictimPlayerState);
	void BroadcastGameOverAnnouncement(class ALab4PlayerState* WinnerPlayerState, float NormalizedPlayerScores);
	void OnMatchStateSet(FName State);

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* AttackerPlayerState, APlayerState* VictimPlayerState);

	UFUNCTION(Client, Reliable)
	void ClientGameOverToggle(ALab4PlayerState* WinnerPlayerState, float NormalizedPlayerScores);
	
	FORCEINLINE ALab4HUD* GetLab4HUD () const { return Lab4HUD; }
	
private:
	int32 ResetCountdownTime;
	FTimerHandle CountdownTimer;

	// Разница между временем сервера и клиента
	float ClientServerDelta = .0f;

	// Каждые 5 секунд буду отправляться запросы, чтобы синхронизировать игровое время с сервером
	UPROPERTY(EditAnywhere)
	float TimeSyncFrequency = 5.f;

	// Количество секунд, которое прошло с последней синхронизации времени с сервером
	float TimeSyncRunningTime = .0f;
	
	UFUNCTION()
	void DecreaseCountdownTime();

	void CheckTimeSync(float DeltaSeconds);

	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UPlayerHealthBar* CharacterOverlay;

	bool bInitializeCharacterOverlay = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;

	float MatchStartTime = 0.f;
	float LevelStartTime = 0.f;
	uint32 CountdownInt = 0;
};
