// Copyright Epic Games, Inc. All Rights Reserved.

#include "Lab4GameMode.h"
#include "Lab4Character.h"
#include "Lab4HUD.h"
#include "Lab4PlayerController.h"
#include "Lab4PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "UserWidgets/WinnerWiddget.h"

ALab4GameMode::ALab4GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
		TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PlayerStateClass = ALab4PlayerState::StaticClass();
	static ConstructorHelpers::FClassFinder<ALab4HUD> Lab4HUDClass(TEXT("/Game/ClassesBlueprints/Lab4HUD/BP_Lab4HUD"));

	if (Lab4HUDClass.Succeeded())
	{
		HUDClass = Lab4HUDClass.Class;
	}

	PlayerControllerClass = ALab4PlayerController::StaticClass();

	TotalFrags = 10;
	bDelayedStart = true;
}

void ALab4GameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		// Отсоединить актера от текущего контроллера
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}

	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 RandomStartIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[RandomStartIndex]);
	}
}

void ALab4GameMode::PlayerEliminated(ALab4Character* ElimmedCharacter,
		ALab4PlayerController* VictimController,
		ALab4PlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if(VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	ALab4PlayerState* AttackerPlayerState = AttackerController ? Cast<ALab4PlayerState>(AttackerController->PlayerState) : nullptr;
	ALab4PlayerState* VictimPlayerState = VictimController ? Cast<ALab4PlayerState>(VictimController->PlayerState) : nullptr;

	UE_LOG(LogTemp, Warning, TEXT("In GameMode: Attacker %s \n Victim: %s \n"), *AttackerPlayerState->GetPlayerName(), *VictimPlayerState->GetPlayerName());
	
	if (AttackerPlayerState)
	{
		AttackerPlayerState->AddToScore(1.0f);	
	}
	
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}

	const UWorld* World = GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			ALab4PlayerController* Lab4PlayerController = Cast<ALab4PlayerController>(*It);
			
			if (VictimPlayerState && AttackerPlayerState && Lab4PlayerController)
			{
				Lab4PlayerController->BroadcastAnnouncement(AttackerPlayerState, VictimPlayerState);
			}

			if (FMath::FloorToInt(Cast<ALab4PlayerState>(Lab4PlayerController->PlayerState)->GetScore()) >= TotalFrags)
			{
				
				for (FConstPlayerControllerIterator It2 = World->GetPlayerControllerIterator(); It2; ++It2)
				{
					ALab4PlayerController* CharacterPlayerController = Cast<ALab4PlayerController>(*It2);
					ALab4Character* Character = Cast<ALab4Character>(CharacterPlayerController->GetCharacter());
					float PlayerNormalizedScores;
					
					if (Character)
					{
						Character->IngestMatchData(PlayerNormalizedScores);
						Character->BroadcastSubmitPlayerRankedScores(PlayerNormalizedScores);
						Character->bDisableGameplay = true;
					}
					
					if (CharacterPlayerController)
					{
						CharacterPlayerController->BroadcastGameOverAnnouncement(Lab4PlayerController->GetPlayerState<ALab4PlayerState>(), PlayerNormalizedScores);
					}
				}
				RestartGame();
				break;
			}
		}
	}
}

void ALab4GameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartTime = GetWorld()->GetTimeSeconds();
}

void ALab4GameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = StartMatchTime - GetWorld()->GetTimeSeconds() + LevelStartTime;

		if (CountDownTime <= 0.f)
		{
			StartMatch();
		}
	}
}

void ALab4GameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ALab4PlayerController* PC = Cast<ALab4PlayerController>(*It);

		if (PC)
		{
			PC->OnMatchStateSet(MatchState);
		}

		if (MatchState == MatchState::InProgress)
		{
			ALab4Character* CurrentCharacter = Cast<ALab4Character>(PC->GetCharacter());

			if (CurrentCharacter)
			{
				CurrentCharacter->bDisableFire = false;
			}
		}
	}
}
