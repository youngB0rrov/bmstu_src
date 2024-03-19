// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Networking.h"
#include "Common/TcpListener.h"
#include "ServerManagerGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SERVERMANAGER_API UServerManagerGameInstance : public UGameInstance
{
	GENERATED_BODY()
	 
public:
	TSharedPtr<FTcpListener> TcpListener;
	FRunnableThread* Thread;
	FRunnableThread* ReceiveThread;
	FSocket* ListenerSocket;
	// Как идея: Клиентский сокет можно хранить в основном потоке для отправки ему сообщений
	// Для чтения необходимо создавать дочерний поток, в котором также создается клиентский сокет и происходит чтение бинарных данных
	FSocket* ConnectionSocket;
	
protected:
	virtual void Init() override;
	virtual void Shutdown() override;

private:
	bool OnConnected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	bool StartMessageService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	bool SendMessageToClient(const FString Message);
	void FromStringToBinaryArray(const FString& Message, TArray<uint8>& OutBinaryArray);
	FString FromBinaryArrayToString(const TArray<uint8>& InBinaryArray);
	bool SendPayload(const TArray<uint8>& Payload, FSocket* Socket);
	bool ReceivePayload();

	const FString Adress = TEXT("127.0.0.1");
	const int32 Port = 8870;
};
