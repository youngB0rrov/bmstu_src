// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusControll.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Lab4/EmptyLobbyClasses/EmptyLobbyGameMode.h"
#include "Lab4/EmptyLobbyClasses/EmptyLobbyPlayerState.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"

bool UStatusControll::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}

	EmptyLobbyOwningController = Cast<AEmptyLobbyPlayerController>(GetOwningPlayer());
	GameInstance = GetGameInstance<ULab4GameInstance>();

	if (EmptyLobbyOwningController && EmptyLobbyOwningController->GetLocalRole() == ENetRole::ROLE_AutonomousProxy
		&& EmptyLobbyOwningController->GetRemoteRole() == ENetRole::ROLE_Authority)
	{
		StartMatchButton->SetVisibility(ESlateVisibility::Hidden);	
	}
	StartTimerText->SetVisibility(ESlateVisibility::Hidden);
	StartTimerInfoText->SetVisibility(ESlateVisibility::Hidden);
	PressButtonProgressbar->SetVisibility(ESlateVisibility::Hidden);
	CancelationMessageBox->SetVisibility(ESlateVisibility::Hidden);

	StartMatchButton->OnClicked.AddDynamic(this, &UStatusControll::OnStartButtonMatchedClicked);
	ExitButton->OnClicked.AddDynamic(this, &UStatusControll::OnExitButtonClicked);
	PlayerReadyButton->OnPressed.AddDynamic(this, &UStatusControll::OnPlayerReadyButtonPressed);
	PlayerReadyButton->OnReleased.AddDynamic(this, &UStatusControll::OnPlayerButtonReleased);
	PlayerCancelButton->OnPressed.AddDynamic(this, &UStatusControll::OnPlayerCancelButtonPressed);
	PlayerCancelButton->OnReleased.AddDynamic(this, &UStatusControll::OnPlayerButtonReleased);
	
	return true;
}

void UStatusControll::Tick(float DeltaTime)
{
	if (bIsProgressbarVisible)
	{
		SetPorgressbarPercantage();
	}
}

bool UStatusControll::IsTickable() const
{
	return true;
}

TStatId UStatusControll::GetStatId() const
{
	return TStatId();
}

void UStatusControll::OnStartButtonMatchedClicked()
{
	AEmptyLobbyGameMode* EmptyLobbyGameMode = Cast<AEmptyLobbyGameMode>(UGameplayStatics::GetGameMode(this));

	if (EmptyLobbyGameMode)
	{
		EmptyLobbyGameMode->StartServerTravel();
		UE_LOG(LogTemp, Warning, TEXT("Server Travel button clicked"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LobbyGameMode is invalid"));
	}
}

void UStatusControll::OnExitButtonClicked()
{
	UWorld* World = GetWorld();

	if (World)
	{
		EmptyLobbyOwningController->ClientTravel(TravelMainMenuPath, ETravelType::TRAVEL_Absolute);
	}
}

void UStatusControll::OnPlayerButtonDelegates()
{
	PressButtonProgressbar->SetVisibility(ESlateVisibility::Hidden);
	bIsProgressbarVisible = false;
	PressButtonProgressbar->SetPercent(0.f);
	
	if (EmptyLobbyOwningController)
	{
		if (EmptyLobbyOwningController->HasAuthority())
		{
			AEmptyLobbyPlayerState* EmptyLobbyPlayerState = Cast<AEmptyLobbyPlayerState>(EmptyLobbyOwningController->PlayerState);
			if (EmptyLobbyPlayerState)
			{
				EmptyLobbyPlayerState->bIsReady = PressedButton == 1 ? true : PressedButton == 2 ? false : false;
			}

			AEmptyLobbyHUD* EmptyLobbyHUD = Cast<AEmptyLobbyHUD>(EmptyLobbyOwningController->GetHUD());
			if (EmptyLobbyHUD)
			{
				EmptyLobbyHUD->RefreshGrid();
			}

			AEmptyLobbyGameMode* EmptyLobbyGameMode = Cast<AEmptyLobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			if (EmptyLobbyGameMode)
			{
				EmptyLobbyGameMode->CheckPlayersStatuses();
			}

			return;
		}

		AEmptyLobbyPlayerState* EmptyLobbyPlayerState = Cast<AEmptyLobbyPlayerState>(EmptyLobbyOwningController->PlayerState);
		if (EmptyLobbyPlayerState)
		{
			EmptyLobbyPlayerState->SetStatusToReady(PressedButton);
		}
	}
}

void UStatusControll::OnPlayerReadyButtonPressed()
{
	PressButtonProgressbar->SetVisibility(ESlateVisibility::Visible);
	bIsProgressbarVisible = true;
	
	// Ready button clicked
	PressedButton = 1;
	GameInstance = GameInstance == nullptr ? GetGameInstance<ULab4GameInstance>() : GameInstance;

	if (GameInstance)
	{
		PressButtonDelegate.BindUFunction(this, FName(TEXT("OnPlayerButtonDelegates")));
		GameInstance->GetTimerManager().SetTimer(
			PressButtonHandle,
			PressButtonDelegate,
			PressButtonDuration,
			false
			);
	}
}
void UStatusControll::OnPlayerCancelButtonPressed()
{
	PressButtonProgressbar->SetVisibility(ESlateVisibility::Visible);
	bIsProgressbarVisible = true;
	
	// Cancel button clicked
	PressedButton = 2;
	GameInstance = GameInstance == nullptr ? GetGameInstance<ULab4GameInstance>() : GameInstance;

	if (GameInstance)
	{
		PressButtonDelegate.BindUFunction(this, FName(TEXT("OnPlayerButtonDelegates")));
		GameInstance->GetTimerManager().SetTimer(
			PressButtonHandle,
			PressButtonDelegate,
			PressButtonDuration,
			false
			);
	}
}

void UStatusControll::OnPlayerButtonReleased()
{
	PressButtonProgressbar->SetVisibility(ESlateVisibility::Hidden);
	bIsProgressbarVisible = false;
	PressButtonProgressbar->SetPercent(0.f);
	
	GameInstance = GameInstance == nullptr ? GetGameInstance<ULab4GameInstance>() : GameInstance;

	if (GameInstance)
	{
		GameInstance->GetTimerManager().ClearTimer(PressButtonHandle);
	}
	
}

void UStatusControll::SetPorgressbarPercantage()
{
	GameInstance = GameInstance == nullptr ? GetGameInstance<ULab4GameInstance>() : GameInstance;

	if (GameInstance)
	{
		float ProgressbarRunningTime = GameInstance->GetTimerManager().GetTimerElapsed(PressButtonHandle);
		PressButtonProgressbar->SetPercent(ProgressbarRunningTime / PressButtonDuration);
	}
}
