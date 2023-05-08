// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "Lab4Character.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "UserWidgets/LobbyHostWidget.h"

ALobbyGameMode::ALobbyGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ALab4Character* Character = Cast<ALab4Character>(NewPlayer->GetCharacter());
	ULab4GameInstance* GameInstance = GetGameInstance<ULab4GameInstance>();
	APlayerState* LoggedInPlayerState = NewPlayer->GetPlayerState<APlayerState>();
	uint32 NumberOfPlayers = GameState->PlayerArray.Num();
	uint32 MaxPlayersNum = 10;

	if (Character)
	{
		Character->bDisableFire = true;	
	}
	
	if (GameInstance)
	{
		const IOnlineSubsystem* OnlineSubsystemInGameMode = IOnlineSubsystem::Get();

		if (OnlineSubsystemInGameMode)
		{
			const IOnlineSessionPtr SessionPtrInLobby = OnlineSubsystemInGameMode->GetSessionInterface();

			if (SessionPtrInLobby)
			{
				MaxPlayersNum = SessionPtrInLobby->GetSessionSettings(GameInstance->GetSessionName())->NumPublicConnections;
			}
		}
	}

	if (LoggedInPlayerState)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* CharacterPlayerController = Cast<APlayerController>(*It);
			ALab4Character* CurrentCharacter = Cast<ALab4Character>(CharacterPlayerController->GetCharacter());

			if (CurrentCharacter)
			{
				CurrentCharacter->BroadcastAddLoginMessageToHUD(LoggedInPlayerState->GetPlayerName(), NumberOfPlayers, MaxPlayersNum);
			}
		}
	}
	
	if (NumberOfPlayers == MaxPlayersNum)
	{
		FTimerHandle MatchStartTimerHandle;
		FTimerDelegate MatchStartDelegate;
		MatchStartDelegate.BindUFunction(this, FName("ServerTravelToGameMap"));
		GetWorldTimerManager().SetTimer(
			MatchStartTimerHandle,
			MatchStartDelegate,
			1.0f,
			false
			);
	}

}

void ALobbyGameMode::TeardownAll()
{
	ULab4GameInstance* ServerGameInstance = GetGameInstance<ULab4GameInstance>();

	if (ServerGameInstance && ServerGameInstance->InGameMenu != nullptr)
	{
		// hide game menu
		ServerGameInstance->ShowInGameMenu();
	}
	if (ServerGameInstance && ServerGameInstance->LobbyHostWidget)
	{
		ServerGameInstance->LobbyHostWidget->RemoveFromViewport();
	}
}

void ALobbyGameMode::ServerTravelToGameMap()
{
	UWorld* World = GetWorld();
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* CharacterPlayerController = Cast<APlayerController>(*It);
		ALab4Character* CurrentCharacter = Cast<ALab4Character>(CharacterPlayerController->GetCharacter());

		if (CurrentCharacter)
		{
			CurrentCharacter->BroadcastRemoveLoginMessage();
		}
	}

	if (World)
	{
		TeardownAll();
		bUseSeamlessTravel = true;
		GEngine->RemoveOnScreenDebugMessage(1);
		World->ServerTravel(FString("/Game/Maps/GamePlayMap?listen"));
	}
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* ServerPlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (ServerPlayerController != nullptr)
	{
		ALab4Character* ServerCharacter = Cast<ALab4Character>(ServerPlayerController->GetCharacter());

		if (ServerCharacter)
		{
			ServerCharacter->bDisableFire = true;
			ULab4GameInstance* Lab4GameInstance = ServerCharacter->GetGameInstance<ULab4GameInstance>();
			
			if (Lab4GameInstance != nullptr)
			{
				Lab4GameInstance->AddLobbyHostWidget();
			}
		}
	}
}
