// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lab4PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Lab4PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API ALab4PlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void OnRep_Score() override;
	void AddToScore(float ScoreAmount);
	void AddToName(FString Name);
	FORCEINLINE float GetPlayerScore() const { return GetScore(); };
	
private:
	class ALab4Character* Character;

	UPROPERTY()
	ALab4PlayerController* Lab4PlayerController;
};
