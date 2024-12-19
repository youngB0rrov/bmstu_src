// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchmakingInputWidget.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"

bool UMatchmakingInputWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	MatchmakingInputConfirmButton->OnClicked.AddDynamic(this, &UMatchmakingInputWidget::OnMatchmakingInputCofirmButtonClicked);
	MatchmakingInputCancelButton->OnClicked.AddDynamic(this, &UMatchmakingInputWidget::OnMatchmakingInputCancelButtonClicked);

	return true;
}

void UMatchmakingInputWidget::OnMatchmakingInputCofirmButtonClicked()
{
	ULab4GameInstance* gameInstance = GetGameInstance<ULab4GameInstance>();
	if (gameInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("GameInstance ptr is invalid"))
		return;
	}
	if (MatchmakingInput == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Matchmaking ptr is invalid"))
		return;
	}
	if (MatchmakingInput->GetText().ToString().IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("User login is required"), true);
		return;
	}

	FString userLogin = MatchmakingInput->GetText().ToString();
	gameInstance->SetPlayerName(userLogin);

	if (gameInstance->CheckIfOfferAcquired())
	{
		MatchmakingInput->SetText(FText::GetEmpty());
		RemoveFromViewport();
		gameInstance->GiveAccessToCreateMatchSection();
		gameInstance->SetCreateGameHintTextVisibility(false);
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Access to create-match section is forbidden!"), true, FVector2D(2.f));	
}

void UMatchmakingInputWidget::OnMatchmakingInputCancelButtonClicked()
{
	ULab4GameInstance* gameInstance = GetGameInstance<ULab4GameInstance>();
	if (gameInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("GameInstance ptr is invalid"))
		return;
	}
	gameInstance->ShowMatchmakingInputWidget(false);
}
