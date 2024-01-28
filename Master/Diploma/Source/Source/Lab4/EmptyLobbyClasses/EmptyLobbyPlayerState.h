// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "EmptyLobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API AEmptyLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AEmptyLobbyPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_bIsReady)
	bool bIsReady;

	FORCEINLINE bool GetPlayerStatus() const { return bIsReady; }

	UFUNCTION()
	void OnRep_bIsReady();

	UFUNCTION(Server, Reliable)
	void SetStatusToReady(const uint16& ButtonFlag);
};
