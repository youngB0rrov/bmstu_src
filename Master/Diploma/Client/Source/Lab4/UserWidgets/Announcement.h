// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* StartMatchText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* StartMatchTimer;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* StartMatchInfo;
};
