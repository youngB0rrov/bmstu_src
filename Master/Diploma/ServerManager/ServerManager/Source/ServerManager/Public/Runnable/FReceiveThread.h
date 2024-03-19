// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
/**
 * 
 */
class SERVERMANAGER_API FReceiveThread: public FRunnable
{
public:
	FReceiveThread(FSocket* clientSocket)
	{
		ClientSocket = clientSocket;
		bIsStopped = false;
	}
	~FReceiveThread()
	{
		bIsStopped = true;
	}

	virtual bool Init() override
	{
		bIsStopped = false;
		return true;
	}
	virtual uint32 Run() override
	{
		if (ClientSocket == nullptr) return 1;

		TArray<uint8> receivedData;
		uint8 element = 0;

		while (!bIsStopped)
		{
			receivedData.Init(element, 1024u);
			int32 bytesRead = 0;

			ClientSocket->Recv(receivedData.GetData(), receivedData.Num(), bytesRead);
			if (bytesRead != 0)
			{
				const FString receivedStringData = FromBinaryArrayToString(receivedData);
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Received data from client: %s"), *receivedStringData));
				bIsStopped = true;
			}
			FPlatformProcess::Sleep(0.01f);
		}
		
		return 0;
	}
	virtual void Stop() override
	{
		bIsStopped = true;
		UE_LOG(LogTemp, Log, TEXT("Stopped running thread"))
	}
	virtual void Exit() override
	{
		ClientSocket = nullptr;
	}

private:
	FSocket* ClientSocket;
	bool bIsStopped;

	FString FromBinaryArrayToString(const TArray<uint8>& receivedData)
	{
		return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(receivedData.GetData())));
	}
};
