// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusGrid.h"

#include "StatusRow.h"
#include "Components/CircularThrobber.h"
#include "Components/PanelWidget.h"
#include "Components/SplineComponent.h"
#include "GameFramework/GameState.h"
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
	
	AGameStateBase* WorldGameState = UGameplayStatics::GetGameState(World);
	if (WorldGameState == nullptr) return;
	TArray<APlayerState*> PlayerStates = WorldGameState->PlayerArray;
	if (PlayerStates.Num() == 0) return;

	if (bIsGridLoading)
	{
		bIsGridLoading = false;

		if (PlayerGridSpinner)
		{
			PlayerGridSpinner->RemoveFromParent();
		}
	}
	
	TArray<AEmptyLobbyPlayerState*> EmptyLobbyPlayerStates;

	for (auto It = PlayerStates.CreateConstIterator(); It; ++It)
	{
		AEmptyLobbyPlayerState* EmptyLobbyPlayerState = Cast<AEmptyLobbyPlayerState>(*It);

		if (EmptyLobbyPlayerState && EmptyLobbyPlayerState->bIsVerified)
		{
			EmptyLobbyPlayerStates.Push(EmptyLobbyPlayerState);
		}
	}

	EmptyLobbyPlayerStates.Sort([](const AEmptyLobbyPlayerState& A, const AEmptyLobbyPlayerState& B)
	{	
		return A.bIsReady > B.bIsReady;	
	});
	
	AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(GetOwningPlayer());
	if (!EmptyLobbyPlayerController) return;

	if (PlayersTableList->HasAnyChildren())
	{
		PlayersTableList->ClearChildren();
	}

	for (auto EmptyPlayerState : EmptyLobbyPlayerStates)
	{
		UStatusRow* StatusRow = CreateWidget<UStatusRow>(EmptyLobbyPlayerController, PlayerRowClass);
		StatusRow->SetPlayerRowText(EmptyPlayerState);
		PlayersTableList->AddChild(StatusRow);
	}
	
}
