// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusRow.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UStatusRow::SetPlayerRowText(AEmptyLobbyPlayerState* CurrentPlayerState)
{
	if (CurrentPlayerState)
	{
		if (PlayerNameText)
		{
			PlayerNameText->SetText(FText::FromString(CurrentPlayerState->GetPlayerName()));
		}

		if (PlayerPingText)
		{
			PlayerPingText->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentPlayerState->GetPing())));
		}

		if (PlayerStatusText)
		{
			FString PlayerStatus = FString::Printf(TEXT("%s"), CurrentPlayerState->bIsReady == true ? TEXT("Yes") : TEXT("No"));
			PlayerStatusText->SetText(FText::FromString(PlayerStatus));
		}
	}
}
