// Fill out your copyright notice in the Description page of Project Settings.


#include "WinnerWiddget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Lab4/Lab4GameMode.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"

void UWinnerWiddget::SetupWinnerWidget()
{
	UWorld* const World = GetWorld();

	if (World == nullptr) return;

	APlayerController* const PlayerController = World->GetFirstPlayerController();

	if (PlayerController == nullptr) return;

	AddToViewport();

	FInputModeUIOnly InputMode;

	InputMode.SetWidgetToFocus(this->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

void UWinnerWiddget::RemoveWinnerWidget()
{
	UWorld* World = GetWorld();

	if (World == nullptr) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();

	if (PlayerController == nullptr) return;

	this->RemoveFromViewport();
	
	const FInputModeGameOnly InputMode;
	
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = false;
}

void UWinnerWiddget::OnNewGameButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("New Game"));
	ALab4GameMode* MyGameMode = static_cast<ALab4GameMode*>(GetWorld()->GetAuthGameMode());
	RemoveWinnerWidget();
	
	if (MyGameMode == nullptr) return;
}

void UWinnerWiddget::OnMainMenuButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("To Main Menu"));
	const UWorld *World = GetWorld();

	if (World == nullptr) return;

	APlayerController *PlayerController = World->GetFirstPlayerController();

	if (PlayerController == nullptr) return;

	PlayerController->ClientTravel(TravelMainMenuPath, ETravelType::TRAVEL_Absolute);
}

void UWinnerWiddget::SetupWinnerName(FString PlayerName)
{
	if (WinnerNameText == nullptr) return;
	WinnerNameText->SetText(FText::FromString(PlayerName));
}

bool UWinnerWiddget::Initialize()
{
	Super::Initialize();

	if (GameOverMenuButton == nullptr) return false;
	
	GameOverNewGameButton->OnClicked.AddDynamic(this, &UWinnerWiddget::OnNewGameButtonClicked);

	if (GameOverNewGameButton == nullptr) return false;
	
	GameOverMenuButton->OnClicked.AddDynamic(this, &UWinnerWiddget::OnMainMenuButtonClicked);

	return true;
}
