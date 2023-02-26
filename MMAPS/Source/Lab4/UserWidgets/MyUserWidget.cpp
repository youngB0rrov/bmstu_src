// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"

#include "Components/TextBlock.h"

void UMyUserWidget::SetWinnerText(const ALab4PlayerState* WinnerPlayerState)
{
	if (WinnerPlayerState &&
		GameOverTitle &&
		WinnerNameTextBox
	)
	{
		const FString TitleMessage = WinnerPlayerState == GetOwningPlayerState<ALab4PlayerState>()
			 ? FString(TEXT("Win"))
			 : FString(TEXT("Defeat"));
		
		GameOverTitle->SetText(FText::FromString(TitleMessage));
		WinnerNameTextBox->SetText(FText::FromString(WinnerPlayerState->GetPlayerName()));
	}
}

void UMyUserWidget::SetRestartTimer(int32 CountdownTime)
{
	if (RestartDigitsTextBlock)
	{
		int32 Seconds = CountdownTime;
		FString CountdownText = FString::Printf(TEXT("%d..."), Seconds);
		
		RestartDigitsTextBlock->SetText(FText::FromString(CountdownText));
	}
}

void UMyUserWidget::HideWinnerWidget()
{
	RemoveFromViewport();
}
