// Fill out your copyright notice in the Description page of Project Settings.


#include "EmptyLobbyGameMode.h"
#include <cstdint>
#include "EmptyLobbyPlayerController.h"
#include "EmptyLobbyPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Sockets.h"
#include "FNetProtocolStructures.h"
#include "SocketSubsystem.h"
#include "Networking.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"

AEmptyLobbyGameMode::AEmptyLobbyGameMode()
{
	int32 ServerPort, ServersListenPort;
	FString ServerAddress, DaemonAddressString;

	if (GConfig)
	{
		GConfig->GetInt(TEXT("ServerManager"), TEXT("Port"), ServerPort, GEngineIni);
		GConfig->GetString(TEXT("ServerManager"), TEXT("Address"), ServerAddress, GEngineIni);
		GConfig->GetString(TEXT("Daemon"), TEXT("Address"), DaemonAddressString, GEngineIni);
		GConfig->GetInt(TEXT("ServerManager"), TEXT("ServersPort"), ServersListenPort, GEngineIni);

		ServerManagerAddress = ServerAddress;
		ServerManagerPort = ServerPort;
		DaemonAddress = DaemonAddressString;
		ServerManagerServersPort = ServersListenPort;
	}
}

AEmptyLobbyGameMode::~AEmptyLobbyGameMode()
{
	if (ConnectionSocket != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Closing connection socket for server manager..."))
		ConnectionSocket->Shutdown(ESocketShutdownMode::ReadWrite);
		ConnectionSocket->Close();
		ConnectionSocket = nullptr;
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

	//UpdatePlayersInfoForServerManager();
	UpdatePlayersInfoByteForServerManager();
}

void AEmptyLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!InitializeSocketToServerManager())
	{
		UE_LOG(LogTemp, Error, TEXT("An error occured, while initializing socket to ServerManager. Cancel sending any commands"))
		return;
	}

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
	//SendUriToServerManager(Port);
	RegisterPlayersInfoByteForServerManager(Port);
}

bool AEmptyLobbyGameMode::InitializeSocketToServerManager()
{
	FIPv4Address outAddress;
	bool addressParseResult = FIPv4Address::Parse(ServerManagerAddress, outAddress);
	if (!addressParseResult)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse FStringAddress %s to IPv4Address"), *ServerManagerAddress);
		return false;
	}

	FIPv4Endpoint Endpoint(outAddress, ServerManagerServersPort);
	ConnectionSocket = ConnectionSocket == nullptr ? FTcpSocketBuilder("ClientSocket").AsReusable() : ConnectionSocket;
	ConnectionSocket->SetNoDelay(true); // Отключение накопления данных в буфере
	if (!ConnectionSocket->Connect(*Endpoint.ToInternetAddr()))
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while creating socket to host: [address=%s, port=%d]"), *outAddress.ToString(), ServerManagerServersPort)
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("Successfully created socket to host: [address=%s, port=%d]"), *outAddress.ToString(), ServerManagerServersPort)
	
	return true;
}

bool AEmptyLobbyGameMode::ReconnectToServerManager()
{
	if (ConnectionSocket)
	{
		ConnectionSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
		ConnectionSocket = nullptr;
	}

	return InitializeSocketToServerManager();
}

void AEmptyLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	//UpdatePlayersInfoForServerManager();
	UpdatePlayersInfoByteForServerManager();
}

void AEmptyLobbyGameMode::SendMessageWithSocket(const FString& Message)
{
	if (ConnectionSocket == nullptr || ConnectionSocket->GetConnectionState() != SCS_Connected)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send message to ServerMessage: Invalid socket connection, trying to reconnect..."))
		return;
	}

	TArray<uint8> payload;
	FromStringToBinaryArray(Message, payload);

	FBufferArchive ArchiveBuffer;
	int32 bytesSent = 0;

	ArchiveBuffer.Append(payload);
	bool sendResult = ConnectionSocket->Send(ArchiveBuffer.GetData(), ArchiveBuffer.Num(), bytesSent);
	if (bytesSent != ArchiveBuffer.Num() || !sendResult)
	{
		ESocketErrors lastSocketError = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
		FString socketErrorDescription = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError(lastSocketError);

		UE_LOG(LogTemp, Error, TEXT("Error while submiting message to ServerManager: sent:%d, length:%d, sendResult: %d, error: %s"), bytesSent, ArchiveBuffer.Num(), sendResult, *socketErrorDescription)
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Successfully sent message to ServerManager: '%s'"), *Message)
}

FString AEmptyLobbyGameMode::GetServerInstanceUuid()
{
	ULab4GameInstance* gameInstance = GetGameInstance<ULab4GameInstance>();
	FString appUuid = TEXT("");
	if (gameInstance != nullptr)
	{
		appUuid = gameInstance->LoadBase64EncodedData(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AppData"), TEXT("AppGUID.dat")));
	}

	return appUuid.IsEmpty() ? FGuid::NewGuid().ToString() : appUuid;
}

void AEmptyLobbyGameMode::SendUriToServerManager(const int32 Port)
{
	FString uuid = GetServerInstanceUuid();
	FString uri = FString::Printf(TEXT("%s:%d"), *DaemonAddress, Port);
	FString currentPlayers = FString::Printf(TEXT("%d"), GetNumPlayers());
	FString maxPlayers = FString::Printf(TEXT("%d"), 10);
	FString serverState = FString::Printf(TEXT("LOBBY"));
	FString stringPayload = FString::Printf(TEXT("REGISTER_SERVER,uuid=%s,uri=%s,current_players=%s,max_players=%s,state=%s"), *uuid, *uri, *currentPlayers, *maxPlayers, *serverState);

	SendMessageWithSocket(stringPayload);
}

void AEmptyLobbyGameMode::UpdatePlayersInfoForServerManager()
{
	FString uuid = GetServerInstanceUuid();
	FString currentPlayers = FString::Printf(TEXT("%d"), GetNumPlayers());
	FString serverState = FString::Printf(TEXT("LOBBY"));
	FString stringPayload = FString::Printf(TEXT("UPDATE_SERVER,uuid=%s,current_players=%s,state=%s"), *uuid, *currentPlayers, *serverState);
	SendMessageWithSocket(stringPayload);
}

void AEmptyLobbyGameMode::RegisterPlayersInfoByteForServerManager(const int32 Port)
{
	FServerRegisterMessage payload;

	FCStringAnsi::Strncpy(payload.Uuid, TCHAR_TO_ANSI(*GetServerInstanceUuid()), sizeof(payload.Uuid));
	FCStringAnsi::Strncpy(payload.Uri, TCHAR_TO_ANSI(*FString::Printf(TEXT("%s:%d"), *DaemonAddress, Port)), sizeof(payload.Uri));
	payload.CurrentPlayers = GetNumPlayers();
	payload.MaxPlayers = 10;
	payload.ServerState = static_cast<uint8_t>(ServerState::LOBBY);

	MessageFrameHeader frameHeader;

	frameHeader.CommandType = static_cast<uint8_t>(ServerCommandType::REGISTER_SERVER);
	frameHeader.PayloadSize = sizeof(payload);

	TArray<uint8> buffer;
	buffer.Append((uint8*)&frameHeader, sizeof(frameHeader));
	buffer.Append((uint8*)&payload, sizeof(payload));

	int32 bytesSent = 0;

	bool sendResult = ConnectionSocket->Send(buffer.GetData(), buffer.Num(), bytesSent);
	if (bytesSent != buffer.Num() || !sendResult)
	{
		ESocketErrors lastSocketError = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
		FString socketErrorDescription = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError(lastSocketError);

		UE_LOG(LogTemp, Error, TEXT("Error while submiting message to ServerManager: sent:%d, length:%d, sendResult: %d, error: %s"), bytesSent, buffer.Num(), sendResult, *socketErrorDescription)
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Successfully sent register message to ServerManager (frameHeader size: %d; payloadSize: %d, bytes sent: %d"), sizeof(frameHeader), sizeof(payload), bytesSent)
}

void AEmptyLobbyGameMode::UpdatePlayersInfoByteForServerManager()
{
	FServerUpdateMessage payload;

	FCStringAnsi::Strncpy(payload.Uuid, TCHAR_TO_ANSI(*GetServerInstanceUuid()), sizeof(payload.Uuid));
	payload.CurrentPlayers = GetNumPlayers();
	payload.ServerState = static_cast<uint8_t>(ServerState::LOBBY);

	MessageFrameHeader frameHeader;

	frameHeader.CommandType = static_cast<uint8_t>(ServerCommandType::UPDATE_SERVER);
	frameHeader.PayloadSize = sizeof(payload);

	TArray<uint8> buffer;
	buffer.Append((uint8*)&frameHeader, sizeof(frameHeader));
	buffer.Append((uint8*)&payload, sizeof(payload));

	int32 bytesSent = 0;

	bool sendResult = ConnectionSocket->Send(buffer.GetData(), buffer.Num(), bytesSent);
	if (bytesSent != buffer.Num() || !sendResult)
	{
		ESocketErrors lastSocketError = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
		FString socketErrorDescription = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError(lastSocketError);

		UE_LOG(LogTemp, Error, TEXT("Error while submiting message to ServerManager: sent:%d, length:%d, sendResult: %d, error: %s"), bytesSent, buffer.Num(), sendResult, *socketErrorDescription)
			return;
	}
	UE_LOG(LogTemp, Log, TEXT("Successfully sent update message to ServerManager"))
}

void AEmptyLobbyGameMode::FromStringToBinaryArray(const FString& Message, TArray<uint8>& OutBinaryArray)
{
	FTCHARToUTF8 Convert(*Message);
	OutBinaryArray.Empty();
	OutBinaryArray.Append((UTF8CHAR*)Convert.Get(), Convert.Length());
}
