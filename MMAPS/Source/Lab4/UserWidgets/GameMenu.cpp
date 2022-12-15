// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMenu.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"

void UGameMenu::SetupInGameMenu()
{
	const UWorld *World = GetWorld();

	if (World == nullptr) return;

	this->AddToViewport();

	APlayerController *PlayerController = World->GetFirstPlayerController();

	if (PlayerController == nullptr) return;

	FInputModeUIOnly InputMode;

	InputMode.SetWidgetToFocus(this->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

void UGameMenu::TearDownInGameMenu()
{
	this->RemoveFromViewport();

	UWorld *World = GetWorld();

	if (World == nullptr) return;

	APlayerController *PlayerController = World->GetFirstPlayerController();

	if (PlayerController == nullptr) return;

	FInputModeGameOnly InputMode;

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = false;
}

void UGameMenu::OnToMainMenuClicked()
{
	UWorld *World = GetWorld();

	if (World == nullptr) return;

	APlayerController *PlayerController = World->GetFirstPlayerController();

	if (PlayerController == nullptr) return;

	PlayerController->ClientTravel(TravelMainMenuPath, ETravelType::TRAVEL_Absolute);

	
	GetGameInstance<ULab4GameInstance>()->DestroySession();
}

void UGameMenu::OnExitButtonClicked()
{
	GetGameInstance<ULab4GameInstance>()->DestroySession();
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void UGameMenu::OnInGameCancelButtonClicked()
{
	TearDownInGameMenu();
}

bool UGameMenu::Initialize()
{
	// вся та логика, которая была написана для базового класса UUserWidget прописывается здесь, чтобы не переписывать ее
	Super::Initialize();

	if (ToMainMenuButton == nullptr) return false;
	
	ToMainMenuButton->OnClicked.AddDynamic(this, &UGameMenu::OnToMainMenuClicked);

	if (ExitGameButton == nullptr) return false;
	
	ExitGameButton->OnClicked.AddDynamic(this, &UGameMenu::OnExitButtonClicked);

	if (InGameCancelButton == nullptr) return false;

	InGameCancelButton->OnClicked.AddDynamic(this, &UGameMenu::OnInGameCancelButtonClicked);

	return true;
}
