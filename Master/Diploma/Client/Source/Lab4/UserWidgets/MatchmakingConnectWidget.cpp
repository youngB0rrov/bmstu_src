// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchmakingConnectWidget.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Kismet/GameplayStatics.h"

bool UMatchmakingConnectWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	MatchmakingConnectionSpinner->SetVisibility(ESlateVisibility::Hidden);

	MatchmakingConnectButton->OnClicked.AddDynamic(this, &UMatchmakingConnectWidget::OnMatchmakingConnectButtonClicked);
	MatchmakingCancelConnectButton->OnClicked.AddDynamic(this, &UMatchmakingConnectWidget::OnMatchmakingCancelConnectButtonClicked);
	return true;
}

void UMatchmakingConnectWidget::OnMatchmakingConnectButtonClicked()
{
	ULab4GameInstance* gameInstance = GetGameInstance<ULab4GameInstance>();
	if (!gameInstance) return;

	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FString connectionString = gameInstance->GetConnectionAddress();
	SetMatchmakingSpinnerVisibility(true);
	playerController->ClientTravel(connectionString, ETravelType::TRAVEL_Absolute);
}

void UMatchmakingConnectWidget::OnMatchmakingCancelConnectButtonClicked()
{
	RemoveFromViewport();
}

void UMatchmakingConnectWidget::SetMatchmakingSpinnerVisibility(bool bIsVisible)
{
	if (!MatchmakingConnectionSpinner) return;
	MatchmakingConnectionSpinner->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
