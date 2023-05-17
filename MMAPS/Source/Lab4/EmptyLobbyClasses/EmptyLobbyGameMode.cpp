// Fill out your copyright notice in the Description page of Project Settings.


#include "EmptyLobbyGameMode.h"

#include "EmptyLobbyPlayerController.h"

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
