// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstances/ServerManagerGameInstance.h"
#include <string>

void UServerManagerGameInstance::Init()
{
	Super::Init();
	
	if (ConnectionSocket)
	{
		ConnectionSocket->Close();
		ConnectionSocket = nullptr;
	}
	if (TcpListener.IsValid())
	{
		TcpListener->Stop();
	}

	FIPv4Address outAddress;
	bool parseAddress = FIPv4Address::Parse(Adress, outAddress);
	if (!parseAddress)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse FStringAddress %s to IPv4Address"), *Adress);
		return;
	}

	FIPv4Endpoint Endpoint(outAddress, Port);
	ListenerSocket = FTcpSocketBuilder("ServerListeningSocket")
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.Listening(8);

	int32 receiveBufferSize = 2 * 1024 * 1024;
	int32 newSize = 0;
	ListenerSocket->SetReceiveBufferSize(receiveBufferSize, newSize);

	TcpListener = MakeShareable(new FTcpListener(*ListenerSocket));
	TcpListener->OnConnectionAccepted().BindUObject(this, &UServerManagerGameInstance::OnConnected);
	if (TcpListener->Init())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Start listening on %s:%d"), *Adress, Port));
		Thread = FRunnableThread::Create(TcpListener.Get(), TEXT("TCP_Subsystem_Thread"), 0, TPri_BelowNormal);
	}
}

void UServerManagerGameInstance::Shutdown()
{
	if (TcpListener.IsValid())
	{
		TcpListener->Stop();
	}
	Super::Shutdown();
}

bool UServerManagerGameInstance::OnConnected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	bool ServiceStatus = false;
	ServiceStatus = StartMessageService(ClientSocket, ClientEndpoint);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("New client connected from %s"), *ClientEndpoint.ToString()));
	return ServiceStatus;
}

bool UServerManagerGameInstance::StartMessageService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	if (!ConnectionSocket)
	{
		ConnectionSocket = ClientSocket;
		UE_LOG(LogTemp, Log, TEXT("New client connected from %s"), *ClientEndpoint.ToString())
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("New client connected from %s"), *ClientEndpoint.ToString()));
		FString confirmMessage = FString::Printf(TEXT("Connected to %s"), *(FApp::GetName()));

		bool sendResult = SendMessageToClient(confirmMessage);
		if (sendResult)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Message %s was successfully sent to client"), *confirmMessage));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Error, while sending hello message to client")));
		}
		bool receiveResult = ReceivePayload();
	}
	return true;
}

bool UServerManagerGameInstance::SendMessageToClient(const FString Message)
{
	bool sendResult = false;
	if (ConnectionSocket)
	{
		TArray<uint8> Payload;
		FromStringToBinaryArray(Message, Payload);
		sendResult = SendPayload(Payload, ConnectionSocket);
	}
	return sendResult;
}

void UServerManagerGameInstance::FromStringToBinaryArray(const FString& Message, TArray<uint8>& OutBinaryArray)
{
	FTCHARToUTF8 Convert(*Message);
	OutBinaryArray.Empty();
	OutBinaryArray.Append((UTF8CHAR*)Convert.Get(), Convert.Length());
}

FString UServerManagerGameInstance::FromBinaryArrayToString(const TArray<uint8>& InBinaryArray)
{
	std::string cstr(reinterpret_cast<const char*>(InBinaryArray.GetData()), InBinaryArray.Num());
	return FString(cstr.c_str());
}

bool UServerManagerGameInstance::SendPayload(const TArray<uint8>& Payload, FSocket* Socket)
{
	FBufferArchive Ar;
	Ar.Append(Payload);
	
	int32 sentAmount;
	Socket->Send(Ar.GetData(), Ar.Num(), sentAmount);
	if (sentAmount != Ar.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Error while submiting data to client: sent:%d, length:%d"), sentAmount, Ar.Num())
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Error while submiting data to client")));
		return false;
	}
	return true;
}

bool UServerManagerGameInstance::ReceivePayload()
{
	bool receiveResult = false;
	TArray<uint8> receivedData;
	uint8 element = 0;

	if (!ConnectionSocket)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error, while receiving data. Client connection pointer is nullptr"));
		return false;
	}
	while (ConnectionSocket)
	{
		receivedData.Init(element, 1024u);
		int32 bytesRead = 0;

		ConnectionSocket->Recv(receivedData.GetData(), receivedData.Num(), bytesRead);
		const FString receivedStringData = FromBinaryArrayToString(receivedData);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Received data from client: %s"), *receivedStringData));
		FPlatformProcess::Sleep(0.01f);
	}
	if (receivedData.Num() != 0)
	{
		const FString receivedStringData = FromBinaryArrayToString(receivedData);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Received data from client: %s"), *receivedStringData));
		receiveResult = true;
	}
	ConnectionSocket = nullptr;
	return receiveResult;
}
