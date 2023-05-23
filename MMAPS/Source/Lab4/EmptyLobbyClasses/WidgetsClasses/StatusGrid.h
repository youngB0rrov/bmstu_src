// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatusGrid.generated.h"

class UCircularThrobber;
/**
 * 
 */
UCLASS()
class LAB4_API UStatusGrid : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta=(BindWidget))
	UPanelWidget* PlayersTableList;

	UPROPERTY(meta=(BindWidget))
	UCircularThrobber* PlayerGridSpinner;
	
	TSubclassOf<UUserWidget> PlayerRowClass;

	bool bIsGridLoading = true;

public:
	UStatusGrid(const FObjectInitializer& ObjectInitializer);
	void SetPlayersList();
	
};
