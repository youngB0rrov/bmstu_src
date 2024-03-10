// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyHostWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Lab4/LobbyGameMode.h"

bool ULobbyHostWidget::Initialize()
{
	if (!Super::Initialize()) return false;

	LobbyStartButton->OnClicked.AddDynamic(this, &ULobbyHostWidget::OnLobbyStartButtonClicked);
	return true;
}

void ULobbyHostWidget::OnLobbyStartButtonClicked()
{
	ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));

	if (LobbyGameMode)
	{
		LobbyGameMode->ServerTravelToGameMap();
		UE_LOG(LogTemp, Warning, TEXT("Server Travel button clicked"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LobbyGameMode is invalid"));
	}
}
