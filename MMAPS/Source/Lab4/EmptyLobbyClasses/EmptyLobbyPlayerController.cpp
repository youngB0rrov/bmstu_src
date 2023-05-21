// Fill out your copyright notice in the Description page of Project Settings.


#include "EmptyLobbyPlayerController.h"

#include "EmptyLobbyGameMode.h"
#include "EmptyLobbyPlayerState.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"
#include "Net/UnrealNetwork.h"
#include "WidgetsClasses/StatusControll.h"

void AEmptyLobbyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	FInputModeUIOnly InputModeUIOnly;
	InputModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputModeUIOnly);
	bShowMouseCursor = true;

	RequestServerTime();
}

void AEmptyLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ServerGetEmptyLobbySettings();

	ULab4GameInstance* GameInstance = GetGameInstance<ULab4GameInstance>();

	if (GameInstance)
	{
		SetLobbyPlayerName(GameInstance->GetPlayerName());
	}
}

void AEmptyLobbyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PollInit();

	if (bStartCountdownTimer)
	{
		SetEmptyLobbyHUDTime();
	}

	CheckPlayersSync(DeltaSeconds);
}

void AEmptyLobbyPlayerController::PollInit()
{
	if (EmptyLobbyHUD == nullptr)
	{
		EmptyLobbyHUD = Cast<AEmptyLobbyHUD>(GetHUD());

		if (EmptyLobbyHUD)
		{
			EmptyLobbyHUD->AddStatusControllOverlay();
			EmptyLobbyHUD->AddStatusGrid();

			if (bHUDIsBeingInitialized)
			{
				EmptyLobbyHUD->RefreshGrid();
				bHUDIsBeingInitialized = false;
			}
		}
	}
}

void AEmptyLobbyPlayerController::SetEmptyLobbyHUDTime()
{
	float TimeLeft = CountdownTime - GetClientServerDelta() + CountdownStartTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (CountdownInt != SecondsLeft)
	{
		SetEmptyLobbyCountdownTimer(SecondsLeft);
	}
	CountdownInt = SecondsLeft;
}

void AEmptyLobbyPlayerController::SetEmptyLobbyCountdownTimer(uint32 TimeLeft)
{
	if (EmptyLobbyHUD
		&& EmptyLobbyHUD->StatusControllWidget
		&& EmptyLobbyHUD->StatusControllWidget->StartTimerText)
	{
		const int32 Minutes = FMath::FloorToInt(TimeLeft / 60.f);
		const int32 Seconds = TimeLeft - Minutes * 60;
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		EmptyLobbyHUD->StatusControllWidget->StartTimerText->SetText(FText::FromString(CountdownText));
	}
}

void AEmptyLobbyPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float ServerReceitTime)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = ServerReceitTime + (.5 * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AEmptyLobbyPlayerController::ServerRequestServerTime_Implementation(float ClientRequestTime)
{
	// Текущее количество секунд, которое прошло на сервере на данный момент;
	float ServerTimeOfReceit = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(ClientRequestTime, ServerTimeOfReceit);
}

void AEmptyLobbyPlayerController::ClientAddCountdownTimer_Implementation(float CountdownServerStartTime)
{
	CountdownStartTime = CountdownServerStartTime;
	EmptyLobbyHUD->SetTimerVisability(true);
	bStartCountdownTimer = true;
}

void AEmptyLobbyPlayerController::ClientGetEmptyLobbySettings_Implementation(float CountdownServerTime)
{
	CountdownTime = CountdownServerTime;
	UE_LOG(LogTemp, Warning, TEXT("Got CountdownTime = %f"), CountdownTime);
}

void AEmptyLobbyPlayerController::ServerGetEmptyLobbySettings_Implementation()
{
	AEmptyLobbyGameMode* EmptyLobbyGameMode = Cast<AEmptyLobbyGameMode>(UGameplayStatics::GetGameMode(this));

	if (EmptyLobbyGameMode)
	{
		ClientGetEmptyLobbySettings(EmptyLobbyGameMode->CountdownTime);
	}
	
}

void AEmptyLobbyPlayerController::DeleteEmptyLobbyHUD_Implementation()
{
	EmptyLobbyHUD->RemoveControllOverlay();
}

float AEmptyLobbyPlayerController::GetClientServerDelta()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}

	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AEmptyLobbyPlayerController::CheckPlayersSync(float DeltaSeconds)
{
	
	TimeSyncRunning += DeltaSeconds;

	if (TimeSyncRunning > TimeSyncFrequency)
	{
		if (EmptyLobbyHUD)
		{
			EmptyLobbyHUD->RefreshGrid();
		}
		TimeSyncRunning = 0.f;
	}
}

void AEmptyLobbyPlayerController::ClearCountdownTimer_Implementation()
{
	EmptyLobbyHUD->SetTimerVisability(false);
	bStartCountdownTimer = false;
	CountdownInt = 0;

	// Дополнительная синхронизациия во времени клиента и сервера
	RequestServerTime();
}

void AEmptyLobbyPlayerController::ClientRefreshPlayersGrid_Implementation()
{
	if (EmptyLobbyHUD)
	{
		EmptyLobbyHUD->RefreshGrid();
	}
	else
	{
		bHUDIsBeingInitialized = true;
	}
}

void AEmptyLobbyPlayerController::SetLobbyPlayerName_Implementation(const FString& PlayerName)
{
	ULab4GameInstance* GameInstance = GetGameInstance<ULab4GameInstance>();

	if (GameInstance)
	{
		if (GameInstance->GetIsLanGame())
		{
			PlayerState->SetPlayerName(PlayerName);
		}
	}
}

void AEmptyLobbyPlayerController::RequestServerTime()
{
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AEmptyLobbyPlayerController::ClientAddCancellationMessage_Implementation()
{
	if (EmptyLobbyHUD)
	{
		EmptyLobbyHUD->ShowCancellationMessage();
	}
}
