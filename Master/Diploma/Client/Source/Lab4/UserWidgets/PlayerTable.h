// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerTable.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API UPlayerTable : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta=(BindWidget))
	UPanelWidget* PlayersTableList;

	TSubclassOf<UUserWidget> PlayerRowClass;
	
public:
	UPlayerTable(const FObjectInitializer& ObjectInitializer);
	void SetPlayersList();
};
