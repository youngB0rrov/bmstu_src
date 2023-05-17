// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusControll.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Lab4/EmptyLobbyClasses/EmptyLobbyGameMode.h"

bool UStatusControll::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}

	EmptyLobbyOwningController = Cast<AEmptyLobbyPlayerController>(GetOwningPlayer());

	if (EmptyLobbyOwningController && EmptyLobbyOwningController->GetLocalRole() == ENetRole::ROLE_AutonomousProxy
		&& EmptyLobbyOwningController->GetRemoteRole() == ENetRole::ROLE_Authority)
	{
		StartMatchButton->SetVisibility(ESlateVisibility::Hidden);	
	}
	StartTimerText->SetVisibility(ESlateVisibility::Hidden);
	StartTimerInfoText->SetVisibility(ESlateVisibility::Hidden);

	StartMatchButton->OnClicked.AddDynamic(this, &UStatusControll::OnStartButtonMatchedClicked);
	ExitButton->OnClicked.AddDynamic(this, &UStatusControll::OnExitButtonClicked);
	
	return true;
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
