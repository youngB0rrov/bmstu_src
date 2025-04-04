// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lab4/UserWidgets/MatchmakingConnectWidget.h"
#include "Networking.h"

/**
 * 
 */
class LAB4_API FReceiveThread: public FRunnable
{
public:
	FReceiveThread(FSocket* hostSocket, ULab4GameInstance* gameInstance)
	{
		HostSocket = hostSocket;
		GameInstance = gameInstance;
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
		if (HostSocket == nullptr) return 1;

		TArray<uint8> receivedData;
		uint8 element = 0;

		while (!bIsStopped)
		{
			receivedData.Init(element, 1024u);
			int32 bytesRead = 0;

			HostSocket->Recv(receivedData.GetData(), receivedData.Num(), bytesRead);
			if (bytesRead != 0)
			{
				const FString receivedStringData = FromBinaryArrayToString(receivedData);
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Received data from host: %s"), *receivedStringData));
				
				GameInstance->SetConnectAddress(receivedStringData);
				AMainMenuInitializer* mainMenu = GameInstance->GetMainMenuPtr();
				AsyncTask(ENamedThreads::GameThread, [mainMenu]()
				{
					mainMenu->SetMatchmakingStatusAndConnect();
				});
			}
			FPlatformProcess::Sleep(0.01f);
		}

		return 0;
	}
	virtual void Stop() override
	{
		bIsStopped = true;
		UE_LOG(LogTemp, Log, TEXT("Stopped running receive socket task"))
	}
	virtual void Exit() override
	{
		HostSocket = nullptr;
	}

private:
	FSocket* HostSocket;
	ULab4GameInstance* GameInstance;
	bool bIsStopped;

	FString FromBinaryArrayToString(const TArray<uint8>& receivedData)
	{
		return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(receivedData.GetData())));
	}
};
