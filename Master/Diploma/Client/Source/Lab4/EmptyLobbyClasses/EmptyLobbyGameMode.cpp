// Fill out your copyright notice in the Description page of Project Settings.


#include "EmptyLobbyGameMode.h"

#include "EmptyLobbyPlayerController.h"
#include "EmptyLobbyPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void AEmptyLobbyGameMode::ServerTravelToGameMap()
{
	
	UWorld* World = GetWorld();
	
	if (World)
	{
		bUseSeamlessTravel = true;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(*It);

			if (EmptyLobbyPlayerController)
			{
				EmptyLobbyPlayerController->DeleteEmptyLobbyHUD();
			}
		}
		
		World->ServerTravel(FString("/Game/Maps/GamePlayMap?listen"));
	}
}

void AEmptyLobbyGameMode::StartServerTravel()
{
	bIsReadyToStart = true;
	CountdownStartTime = GetWorld()->GetTimeSeconds();
	UWorld* World = GetWorld();
	
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(*It);

			if (EmptyLobbyPlayerController)
			{
				EmptyLobbyPlayerController->ClientAddCountdownTimer(CountdownStartTime);
			}
		}
	}
	
	TravelTimerDelegate.BindUFunction(this, FName(TEXT("ServerTravelToGameMap")));
	GetWorldTimerManager().SetTimer(
		TravelTimer,
		TravelTimerDelegate,
		5.5f,
		false
	);
}

void AEmptyLobbyGameMode::CheckPlayersStatuses()
{
	TArray<APlayerState*> PlayerStates = GameState->PlayerArray;

	for (APlayerState* PlayerState : PlayerStates)
	{
		AEmptyLobbyPlayerState* EmptyLobbyPlayerState = Cast<AEmptyLobbyPlayerState>(PlayerState);

		if (EmptyLobbyPlayerState)
		{
			if (EmptyLobbyPlayerState->bIsReady == false)
			{
				UE_LOG(LogTemp, Warning, TEXT("Not all users are ready, return;"));
				if (bIsReadyToStart)
				{
					ClearServerTravelTimer();
				}
				
				return;
			}
		}
	}
	
	StartServerTravel();
}

void AEmptyLobbyGameMode::ClearServerTravelTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("Interrupting sertver travel"));
	bIsReadyToStart = false;
	GetWorldTimerManager().ClearTimer(TravelTimer);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(*It);

		if (EmptyLobbyPlayerController)
		{
			EmptyLobbyPlayerController->ClearCountdownTimer();
			EmptyLobbyPlayerController->ClientAddCancellationMessage();
		}
	}
}

void AEmptyLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	// if (bIsReadyToStart)
	// {
	// 	ClearServerTravelTimer();
	// }
	
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(*It);

		if (EmptyLobbyPlayerController)
		{
			EmptyLobbyPlayerController->ClientRefreshPlayersGrid();
		}
	}
}
