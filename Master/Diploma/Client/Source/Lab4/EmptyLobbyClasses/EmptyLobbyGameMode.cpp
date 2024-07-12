// Fill out your copyright notice in the Description page of Project Settings.


#include "EmptyLobbyGameMode.h"

#include "EmptyLobbyPlayerController.h"
#include "EmptyLobbyPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Networking.h"

AEmptyLobbyGameMode::AEmptyLobbyGameMode()
{
	int32 ServerPort;
	FString ServerAddress, DaemonAddressString;

	if (GConfig)
	{
		GConfig->GetInt(TEXT("ServerManager"), TEXT("Port"), ServerPort, GEngineIni);
		GConfig->GetString(TEXT("ServerManager"), TEXT("Address"), ServerAddress, GEngineIni);
		GConfig->GetString(TEXT("Daemon"), TEXT("Address"), DaemonAddressString, GEngineIni);
		ServerManagerAddress = ServerAddress;
		ServerManagerPort = ServerPort;
		DaemonAddress = DaemonAddressString;
	}
}

void AEmptyLobbyGameMode::ServerTravelToGameMap()
{
	
	UWorld* World = GetWorld();
	
	if (World)
	{
		bUseSeamlessTravel = true;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(*It);

			if (EmptyLobbyPlayerController)
			{
				EmptyLobbyPlayerController->DeleteEmptyLobbyHUD();
			}
		}
		
		World->ServerTravel(FString("/Game/Maps/GamePlayMap?listen"));
	}
}

void AEmptyLobbyGameMode::StartServerTravel()
{
	bIsReadyToStart = true;
	CountdownStartTime = GetWorld()->GetTimeSeconds();
	UWorld* World = GetWorld();
	
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(*It);

			if (EmptyLobbyPlayerController)
			{
				EmptyLobbyPlayerController->ClientAddCountdownTimer(CountdownStartTime);
			}
		}
	}
	
	TravelTimerDelegate.BindUFunction(this, FName(TEXT("ServerTravelToGameMap")));
	GetWorldTimerManager().SetTimer(
		TravelTimer,
		TravelTimerDelegate,
		5.5f,
		false
	);
}

void AEmptyLobbyGameMode::CheckPlayersStatuses()
{
	TArray<APlayerState*> PlayerStates = GameState->PlayerArray;

	for (APlayerState* PlayerState : PlayerStates)
	{
		AEmptyLobbyPlayerState* EmptyLobbyPlayerState = Cast<AEmptyLobbyPlayerState>(PlayerState);

		if (EmptyLobbyPlayerState)
		{
			if (EmptyLobbyPlayerState->bIsReady == false)
			{
				UE_LOG(LogTemp, Warning, TEXT("Not all users are ready, return;"));
				if (bIsReadyToStart)
				{
					ClearServerTravelTimer();
				}
				
				return;
			}
		}
	}
	
	StartServerTravel();
}

void AEmptyLobbyGameMode::ClearServerTravelTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("Interrupting sertver travel"));
	bIsReadyToStart = false;
	GetWorldTimerManager().ClearTimer(TravelTimer);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(*It);

		if (EmptyLobbyPlayerController)
		{
			EmptyLobbyPlayerController->ClearCountdownTimer();
			EmptyLobbyPlayerController->ClientAddCancellationMessage();
		}
	}
}

void AEmptyLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	// if (bIsReadyToStart)
	// {
	// 	ClearServerTravelTimer();
	// }
	
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AEmptyLobbyPlayerController* EmptyLobbyPlayerController = Cast<AEmptyLobbyPlayerController>(*It);

		if (EmptyLobbyPlayerController)
		{
			EmptyLobbyPlayerController->ClientRefreshPlayersGrid();
		}
	}
}

void AEmptyLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World is null"));
		return;
	}

	UNetDriver* NetDriver = World->GetNetDriver();
	if (!NetDriver)
	{
		UE_LOG(LogTemp, Error, TEXT("NetDriver is null"));
		return;
	}

	FString LocalNetworkAddress = NetDriver->LowLevelGetNetworkNumber();
	if (LocalNetworkAddress.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("LocalNetworkAddressString is empty"));
		return;
	}

	FString AdressString;
	FString PortString;

	if (!LocalNetworkAddress.Split(TEXT(":"), &AdressString, &PortString))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get port from LocalNetworkAdressString"));
		return;
	}
	int32 Port = FCString::Atoi(*PortString);
	UE_LOG(LogTemp, Log, TEXT("Server is listening on port: %d"), Port);
	SendUriToServerManager(Port);
}

void AEmptyLobbyGameMode::SendUriToServerManager(const int32 Port)
{
	// Создание сокета ServerManager
	FIPv4Address outAddress;
	bool addressParseResult = FIPv4Address::Parse(ServerManagerAddress, outAddress);
	if (!addressParseResult)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse FStringAddress %s to IPv4Address"), *ServerManagerAddress);
		return;
	}

	FIPv4Endpoint Endpoint(outAddress, ServerManagerPort);
	FSocket* ConnectionSocket = FTcpSocketBuilder("ClientSocket");
	if (!ConnectionSocket->Connect(*Endpoint.ToInternetAddr()))
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while creating socket to host: [address=%s, port=%d]"), *outAddress.ToString(), ServerManagerPort)
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Successfully created socket to host: [address=%s, port=%d]"), *outAddress.ToString(), ServerManagerPort)

	TArray<uint8> payload;
	FString Uri = FString::Printf(TEXT("%s:%d"), *DaemonAddress, Port);
	FromStringToBinaryArray(Uri, payload);

	FBufferArchive ArchiveBuffer;
	int32 bytesSent = 0;

	ArchiveBuffer.Append(payload);
	ConnectionSocket->Send(ArchiveBuffer.GetData(), ArchiveBuffer.Num(), bytesSent);
	if (bytesSent != ArchiveBuffer.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Error while submiting URI to ServerManager: sent:%d, length:%d"), bytesSent, ArchiveBuffer.Num())
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Successfully sent URI '%s' to ServerManager"), *Uri)
}
void AEmptyLobbyGameMode::FromStringToBinaryArray(const FString& Message, TArray<uint8>& OutBinaryArray)
{
	FTCHARToUTF8 Convert(*Message);
	OutBinaryArray.Empty();
	OutBinaryArray.Append((UTF8CHAR*)Convert.Get(), Convert.Length());
}
