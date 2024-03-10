// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PlayerHealthBar.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API UPlayerHealthBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthProgressBar;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreText;
	
	void SetPersantage(float Persantage);
};
