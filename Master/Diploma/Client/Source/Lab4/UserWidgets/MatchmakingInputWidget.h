// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Blueprint/UserWidget.h"
#include "MatchmakingInputWidget.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API UMatchmakingInputWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	UEditableText* MatchmakingInput;

	UPROPERTY(meta = (BindWidget))
	UButton* MatchmakingInputConfirmButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MatchmakingInputCancelButton;

public:
	UFUNCTION()
	void OnMatchmakingInputCofirmButtonClicked();

	UFUNCTION()
	void OnMatchmakingInputCancelButtonClicked();

protected:
	virtual bool Initialize() override;
};
