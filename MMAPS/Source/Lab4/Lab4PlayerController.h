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
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void BroadcastAnnouncement(APlayerState* AttackerPlayerState, APlayerState* VictimPlayerState);

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* AttackerPlayerState, APlayerState* VictimPlayerState);
	
	FORCEINLINE ALab4HUD* GetLab4HUD () const { return Lab4HUD; }
};
