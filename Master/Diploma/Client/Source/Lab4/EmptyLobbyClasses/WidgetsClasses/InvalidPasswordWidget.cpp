// Fill out your copyright notice in the Description page of Project Settings.

#include "InvalidPasswordWidget.h"
#include "Components/Button.h"

bool UInvalidPasswordWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	
	InvalidPasswordConfirmationButton->OnClicked.AddDynamic(this, &UInvalidPasswordWidget::OnInvalidPasswordConfirmationButtonClicked);

	return true;
}

void UInvalidPasswordWidget::OnInvalidPasswordConfirmationButtonClicked()
{
	APlayerController* pc = GetOwningPlayer();
	if (pc != nullptr)
	{
		pc->ClientTravel(TEXT("/Game/MainMenu/MainMenuMap"), ETravelType::TRAVEL_Absolute);
	}
	
	this->RemoveFromViewport();
}
