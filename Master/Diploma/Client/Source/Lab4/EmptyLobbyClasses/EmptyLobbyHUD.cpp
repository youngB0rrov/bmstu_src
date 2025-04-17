// Fill out your copyright notice in the Description page of Project Settings.


#include "EmptyLobbyHUD.h"

#include "EmptyLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "WidgetsClasses/StatusControll.h"
#include "WidgetsClasses/StatusGrid.h"
#include "WidgetsClasses/InvalidPasswordWidget.h"

AEmptyLobbyHUD::AEmptyLobbyHUD()
{
	if (IsRunningDedicatedServer()) return;

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

	static ConstructorHelpers::FClassFinder<UUserWidget> BP_InvalidPasswordWidgetClass(TEXT("/Game/EmptyLobby/WBP_InvalidPasswordWidget"));
	if (BP_InvalidPasswordWidgetClass.Succeeded())
	{
		InvalidPasswordWidgetClass = BP_InvalidPasswordWidgetClass.Class;
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

void AEmptyLobbyHUD::SetTimerVisability(bool bIsVisible)
{
	if (StatusControllWidget &&
		StatusControllWidget->StartTimerText &&
		StatusControllWidget->StartTimerInfoText)
	{
		if (bIsVisible)
		{
			StatusControllWidget->StartTimerText->SetVisibility(ESlateVisibility::Visible);
			StatusControllWidget->StartTimerInfoText->SetVisibility(ESlateVisibility::Visible);

			return;
		}

		StatusControllWidget->StartTimerText->SetVisibility(ESlateVisibility::Hidden);
		StatusControllWidget->StartTimerInfoText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AEmptyLobbyHUD::RefreshGrid()
{
	if (StatusGridWidget)
	{
		StatusGridWidget->SetPlayersList();
	}
}

void AEmptyLobbyHUD::ShowCancellationMessage()
{
	if (StatusControllWidget)
	{
		StatusControllWidget->CancelationMessageBox->SetVisibility(ESlateVisibility::Visible);

		FTimerHandle CancellationMessageTimerHandle;
		FTimerDelegate CancellationMessageTimerDelegate;

		CancellationMessageTimerDelegate.BindUFunction(this, FName("HideCansellationMessage"));
		
		GetWorldTimerManager().SetTimer(
			CancellationMessageTimerHandle,
			CancellationMessageTimerDelegate,
			CancellationMessageDuration,
			false
		);
	}
}

void AEmptyLobbyHUD::ShowPasswordPopup()
{
	AEmptyLobbyPlayerController* emptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(GetOwningPlayerController());

	if (emptyLobbyPlayerController && InvalidPasswordWidgetClass)
	{
		InvalidPasswordPopup = CreateWidget<UInvalidPasswordWidget>(emptyLobbyPlayerController, InvalidPasswordWidgetClass);
		InvalidPasswordPopup->AddToViewport();
	}
}

void AEmptyLobbyHUD::HidePasswordPopup()
{
	if (InvalidPasswordPopup != nullptr)
	{
		InvalidPasswordPopup->RemoveFromViewport();
		InvalidPasswordPopup = nullptr;
	}
}

void AEmptyLobbyHUD::HideCansellationMessage()
{
	StatusControllWidget->CancelationMessageBox->SetVisibility(ESlateVisibility::Hidden);
}
