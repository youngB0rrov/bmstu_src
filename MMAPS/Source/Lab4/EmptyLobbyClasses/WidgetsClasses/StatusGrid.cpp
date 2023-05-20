// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusGrid.h"

#include "StatusRow.h"
#include "Components/PanelWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Lab4/EmptyLobbyClasses/EmptyLobbyPlayerController.h"
#include "Lab4/EmptyLobbyClasses/EmptyLobbyPlayerState.h"

UStatusGrid::UStatusGrid(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> BP_PlayerRowClass(TEXT("/Game/EmptyLobby/WBP_PlayersStatusRowWidget"));
	if (BP_PlayerRowClass.Succeeded())
	{
		PlayerRowClass = BP_PlayerRowClass.Class;
	}
}

void UStatusGrid::SetPlayersList()
{
	UWorld* World = GetWorld();
	TArray<APlayerState*> PlayerStates = UGameplayStatics::GetGameState(World)->PlayerArray;
	AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(GetOwningPlayer());

	if (!EmptyLobbyPlayerController) return;

	PlayersTableList->ClearChildren();

	for (auto PlayerState : PlayerStates)
	{
		UStatusRow* StatusRow = CreateWidget<UStatusRow>(EmptyLobbyPlayerController, PlayerRowClass);
		AEmptyLobbyPlayerState* EmptyLobbyPlayerState = Cast<AEmptyLobbyPlayerState>(PlayerState);
		StatusRow->SetPlayerRowText(EmptyLobbyPlayerState);
		PlayersTableList->AddChild(StatusRow);
	}
	
}
