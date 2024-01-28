// Fill out your copyright notice in the Description page of Project Settings.


#include "EmptyLobbyPlayerState.h"

#include "EmptyLobbyGameMode.h"
#include "EmptyLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AEmptyLobbyPlayerState::AEmptyLobbyPlayerState()
{
	bIsReady = false;
}

void AEmptyLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEmptyLobbyPlayerState, bIsReady);
}

void AEmptyLobbyPlayerState::OnRep_bIsReady()
{
	UE_LOG(LogTemp, Warning, TEXT("Status changed to: %d"), bIsReady);
	AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(GetPawn()->Controller);

	if (EmptyLobbyPlayerController)
	{
		AEmptyLobbyHUD* EmptyLobbyHUD = Cast<AEmptyLobbyHUD>(EmptyLobbyPlayerController->GetHUD());

		if (EmptyLobbyHUD)
		{
			UE_LOG(LogTemp, Warning, TEXT("Refresh player table in OnRep_funciton"));
			EmptyLobbyHUD->RefreshGrid();
		}
	}
}

void AEmptyLobbyPlayerState::SetStatusToReady_Implementation(const uint16& ButtonFlag)
{
	bIsReady = ButtonFlag == 1 ? true : ButtonFlag == 2 ? false : false;

	AEmptyLobbyGameMode* EmptyLobbyGameMode = Cast<AEmptyLobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (EmptyLobbyGameMode)
	{
		EmptyLobbyGameMode->CheckPlayersStatuses();
	}
}
