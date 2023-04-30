// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ALobbyGameMode();
	
private:
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	UFUNCTION()
	void ServerTravelToGameMap();
};
