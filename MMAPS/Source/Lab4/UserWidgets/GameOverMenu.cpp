// Fill out your copyright notice in the Description page of Project Settings.


#include "GameOverMenu.h"


#include "Lab4/Lab4Character.h"

void UGameOverMenu::TeardownGameOverMenu()
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

void UGameOverMenu::OnRetryButtonClicked()
{
	//TODO: respawn
	TeardownGameOverMenu();
	UWorld* World = GetWorld();
	
	const APlayerController* PlayerController = World->GetFirstPlayerController();
	ALab4Character* PlayerCharacter = static_cast<ALab4Character*>(PlayerController->GetPawn());

	//PlayerCharacter->CallRestartPlayer();
	PlayerCharacter->SetCurrentHealth(PlayerCharacter->GetMaxHealth());

	GetGameInstance<ULab4GameInstance>()->ChangeHealthBarState(1.0f);
}

void UGameOverMenu::OnExitButtonClicked()
{
	const UWorld *World = GetWorld();

	if (World == nullptr) return;

	APlayerController *PlayerController = World->GetFirstPlayerController();

	if (PlayerController == nullptr) return;

	PlayerController->ClientTravel(TravelMainMenuPath, ETravelType::TRAVEL_Absolute);
}

bool UGameOverMenu::Initialize()
{
	Super::Initialize();

	if (GameOverMenuRetryButton == nullptr) return false;

	GameOverMenuRetryButton->OnClicked.AddDynamic(this, &UGameOverMenu::OnRetryButtonClicked);

	if (GameOverMenuExitButton == nullptr) return false;

	GameOverMenuExitButton->OnClicked.AddDynamic(this, &UGameOverMenu::OnExitButtonClicked);
	return true;
}

void UGameOverMenu::SetupGameOverMenu()
{
	const UWorld* World = GetWorld();

	if (World == nullptr) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();

	if (PlayerController == nullptr) return;

	this->AddToViewport();
	
	FInputModeUIOnly InputMode;

	InputMode.SetWidgetToFocus(this->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}
