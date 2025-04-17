// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvalidPasswordWidget.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API UInvalidPasswordWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:

	UPROPERTY(meta=(BindWidget))
	class UButton* InvalidPasswordConfirmationButton;

	UFUNCTION()
	void OnInvalidPasswordConfirmationButtonClicked();

protected:
	virtual bool Initialize() override;
};
