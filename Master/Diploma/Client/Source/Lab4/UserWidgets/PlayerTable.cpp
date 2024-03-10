// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTable.h"

#include "PlayerTableRow.h"
#include "Components/PanelWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

UPlayerTable::UPlayerTable(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> BPPlayerRowClass(TEXT("/Game/GameUI/WBP_Player_Table_Row"));

	if (BPPlayerRowClass.Succeeded())
	{
		PlayerRowClass = BPPlayerRowClass.Class;
	}
}

void UPlayerTable::SetPlayersList()
{
	UWorld* World = GetWorld();
	uint32 PlayerIndex = 1;
	TArray<APlayerState*> PlayerStates = UGameplayStatics::GetGameState(World)->PlayerArray;
	APlayerController* PlayerController = GetOwningPlayer();
	
	PlayersTableList->ClearChildren();
	PlayerStates.Sort([](const APlayerState& A, const APlayerState& B)
	{
		return A.GetScore() > B.GetScore();
	});
	
	for (auto PlayerStateItem : PlayerStates)
	{
		ALab4PlayerState* Lab4PlayerState = Cast<ALab4PlayerState>(PlayerStateItem);
		
		if (PlayerRowClass && PlayerController)
		{
			UPlayerTableRow* PlayerTableRow = CreateWidget<UPlayerTableRow>(PlayerController, PlayerRowClass);

			if (PlayerTableRow && Lab4PlayerState)
			{
				PlayerTableRow->SetPlayerRowText(Lab4PlayerState, PlayerIndex);
			}

			PlayersTableList->AddChild(PlayerTableRow);
		}
		PlayerIndex++;
	}
	
}
