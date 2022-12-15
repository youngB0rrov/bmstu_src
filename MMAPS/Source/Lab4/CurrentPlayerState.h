// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CurrentPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API ACurrentPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ACurrentPlayerState();
	void OnScoreUpdate();
	void SetCurrentScore();
	
protected:
	virtual void OnRep_Score() override;
};
