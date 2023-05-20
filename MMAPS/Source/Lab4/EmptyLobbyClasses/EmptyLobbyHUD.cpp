// Fill out your copyright notice in the Description page of Project Settings.


#include "EmptyLobbyHUD.h"

#include "EmptyLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "WidgetsClasses/StatusControll.h"
#include "WidgetsClasses/StatusGrid.h"

AEmptyLobbyHUD::AEmptyLobbyHUD()
{
	static  ConstructorHelpers::FClassFinder<UUserWidget> BP_StatusControllClass(TEXT("/Game/EmptyLobby/WBP_StatusControllWidget"));
	if (BP_StatusControllClass.Succeeded())
	{
		StatusControlWidgetClass = BP_StatusControllClass.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> BP_StatusGridClass(TEXT("/Game/EmptyLobby/WBP_PlayersStatusGridWidget"));
	if (BP_StatusGridClass.Succeeded())
	{
		StatusGridWidgetClass = BP_StatusGridClass.Class;
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

void AEmptyLobbyHUD::AddStatusGrid()
{
	AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(GetOwningPlayerController());

	if (EmptyLobbyPlayerController && StatusGridWidgetClass)
	{
		StatusGridWidget = CreateWidget<UStatusGrid>(EmptyLobbyPlayerController, StatusGridWidgetClass);
		StatusGridWidget->AddToViewport();
	}
}

void AEmptyLobbyHUD::RemoveControllOverlay()
{
	AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(GetOwningPlayerController());

	if (StatusGridWidget)
	{
		StatusGridWidget->RemoveFromViewport();	
	}
	
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

void AEmptyLobbyHUD::RefreshGrid()
{
	if (StatusGridWidget)
	{
		StatusGridWidget->SetPlayersList();
	}
}
