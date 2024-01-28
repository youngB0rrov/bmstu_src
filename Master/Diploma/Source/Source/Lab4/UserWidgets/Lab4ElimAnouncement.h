// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Lab4ElimAnouncement.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API ULab4ElimAnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	class UHorizontalBox* ElimBlock;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ElimText;

	void SetElimAnnouncement(const FString Attacker, const FString Victim);
};

