// Fill out your copyright notice in the Description page of Project Settings.


#include "EmptyLobbyPlayerController.h"

#include "EmptyLobbyGameMode.h"
#include "EmptyLobbyPlayerState.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"
#include "Net/UnrealNetwork.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
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

	ULab4GameInstance* gameInstance = GetGameInstance<ULab4GameInstance>();

	if (gameInstance == nullptr) return;

	SetLobbyPlayerName(gameInstance->GetPlayerName());
	
	if (IsLocalController() && !IsRunningDedicatedServer() && !HasAuthority())
	{
		FString matchPassword = gameInstance->GetPendingPassword();
		ServerVerifyPassword(matchPassword, this);
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

void AEmptyLobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("PushToTalk", IE_Pressed, this, &AEmptyLobbyPlayerController::PushToTalkPressed);
	InputComponent->BindAction("PushToTalk", IE_Released, this, &AEmptyLobbyPlayerController::PushToTalkReleased);
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

void AEmptyLobbyPlayerController::PushToTalkPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("Player is talking"));
	GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("ToggleSpeaking 1")));
}

void AEmptyLobbyPlayerController::PushToTalkReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("Player isn't talking anymore"));
	GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("ToggleSpeaking 0")));
}

void AEmptyLobbyPlayerController::KickBackToMainMenu()
{
	if (EmptyLobbyHUD)
	{
		EmptyLobbyHUD->HidePasswordPopup();
	}

	ClientTravel(TEXT("/Game/MainMenu/MainMenuMap"), ETravelType::TRAVEL_Absolute);
}

void AEmptyLobbyPlayerController::ServerVerifyPassword_Implementation(const FString& EnteredPassword, APlayerController* PlayerController)
{
	if (IsRunningDedicatedServer())
	{
		UE_LOG(LogTemp, Log, TEXT("Server is running in dedicated mode, skip checking password"))
		return;
	}

	AEmptyLobbyGameMode* gameMode = GetWorld()->GetAuthGameMode<AEmptyLobbyGameMode>();

	if (gameMode == nullptr) return;

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	if (onlineSubsystem == nullptr) return;

	IOnlineSessionPtr sessionPtr = onlineSubsystem->GetSessionInterface();
	if (sessionPtr == nullptr) return;
	
	const FNamedOnlineSession* pOnlineSession = sessionPtr->GetNamedSession(TEXT("Session"));
	if (pOnlineSession == nullptr) return;

	FString serverPassword;
	pOnlineSession->SessionSettings.Get(TEXT("SESSION_PASSWORD"), serverPassword);

	AEmptyLobbyPlayerState* playerState = PlayerController->GetPlayerState<AEmptyLobbyPlayerState>();

	if (serverPassword.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Server password is not set, skiping password checking process"))
		playerState->bIsVerified = true;

		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Attemp of validating password for private match"))

	if (serverPassword.Compare(EnteredPassword) != 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Password validation failed, returning connected player to the main menu"))
		ClientShowPasswordPopupAndStartCounting();

		return;
	}

	playerState->bIsVerified = true;
	UE_LOG(LogTemp, Log, TEXT("Password validation completed successfully"))
}

void AEmptyLobbyPlayerController::ClientAddCancellationMessage_Implementation()
{
	if (EmptyLobbyHUD)
	{
		EmptyLobbyHUD->ShowCancellationMessage();
	}
}

void AEmptyLobbyPlayerController::ClientShowPasswordPopupAndStartCounting_Implementation()
{
	if (EmptyLobbyHUD)
	{
		EmptyLobbyHUD->ShowPasswordPopup();
	}

	FTimerHandle tempHandle;

	GetWorld()->GetTimerManager().SetTimer(
		tempHandle,
		this,
		&AEmptyLobbyPlayerController::KickBackToMainMenu,
		10.0f,
		false
	);
}

void AEmptyLobbyPlayerController::OnPlayerVerified()
{
	if (EmptyLobbyHUD)
	{
		EmptyLobbyHUD->RefreshGrid();
	}
}
