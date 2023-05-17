// Fill out your copyright notice in the Description page of Project Settings.


#include "EmptyLobbyHUD.h"

#include "EmptyLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "WidgetsClasses/StatusControll.h"

AEmptyLobbyHUD::AEmptyLobbyHUD()
{
	static  ConstructorHelpers::FClassFinder<UUserWidget> BP_StatusControllClass(TEXT("/Game/EmptyLobby/WBP_StatusControllWidget"));
	if (BP_StatusControllClass.Succeeded())
	{
		StatusControlWidgetClass = BP_StatusControllClass.Class;
	}
}

void AEmptyLobbyHUD::AddStatusControllOverlay()
{
	AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(GetOwningPlayerController());

	if (EmptyLobbyPlayerController && StatusControlWidgetClass)
	{
		StatusControllWidget = CreateWidget<UStatusControll>(EmptyLobbyPlayerController, StatusControlWidgetClass);
		StatusControllWidget->AddToViewport();
	}
}

void AEmptyLobbyHUD::RemoveControllOverlay()
{
	AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(GetOwningPlayerController());

	if (EmptyLobbyPlayerController && StatusControllWidget)
	{
		StatusControllWidget->RemoveFromViewport();
		const FInputModeGameOnly InputModeGameOnly;
		EmptyLobbyPlayerController->SetInputMode(InputModeGameOnly);
		EmptyLobbyPlayerController->bShowMouseCursor = false;
	}
}

void AEmptyLobbyHUD::SetTimerVisability()
{
	if (StatusControllWidget &&
		StatusControllWidget->StartTimerText &&
		StatusControllWidget->StartTimerInfoText)
	{
		StatusControllWidget->StartTimerText->SetVisibility(ESlateVisibility::Visible);
		StatusControllWidget->StartTimerInfoText->SetVisibility(ESlateVisibility::Visible);
	}
}
