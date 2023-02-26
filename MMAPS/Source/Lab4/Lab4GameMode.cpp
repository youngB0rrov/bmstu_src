// Copyright Epic Games, Inc. All Rights Reserved.

#include "Lab4GameMode.h"
#include "Lab4Character.h"
#include "Lab4HUD.h"
#include "Lab4PlayerController.h"
#include "Lab4PlayerState.h"
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

	TotalFrags = 3;
}

void ALab4GameMode::Pause(const FString PlayerName)
{
	UWorld* World = GetWorld();

	if (World == nullptr) return;

	if (!IsPaused())
	{
		UGameplayStatics::SetGamePaused(World, true);
		GetGameInstance<ULab4GameInstance>()->SetupWinnerWidget(PlayerName);
	}
	else
	{
		UGameplayStatics::SetGamePaused(World, false);
	}
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

					if (CharacterPlayerController)
					{
						CharacterPlayerController->BroadcastGameOverAnnouncement(Lab4PlayerController->GetPlayerState<ALab4PlayerState>());
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
}