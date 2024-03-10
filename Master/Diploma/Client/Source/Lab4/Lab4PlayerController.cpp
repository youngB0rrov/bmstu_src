// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab4PlayerController.h"

#include "Lab4Character.h"
#include "Lab4GameMode.h"
#include "Lab4HUD.h"
#include "Lab4PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UserWidgets/Announcement.h"
#include "UserWidgets/MyUserWidget.h"
#include "UserWidgets/PlayerHealthBar.h"
#include "Kismet/GameplayStatics.h"

void ALab4PlayerController::BeginPlay()
{
	Super::BeginPlay();
	Lab4HUD = Cast<ALab4HUD>(GetHUD());
	ResetCountdownTime = 6;
	ServerGetGameModeSettings();
}

void ALab4PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALab4PlayerController, MatchState);
}

void ALab4PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDTime();
	PollInit();
	CheckTimeSync(DeltaSeconds);
}

void ALab4PlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ALab4PlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (Lab4HUD && Lab4HUD->PlayerHealthBar)
		{
			CharacterOverlay = Lab4HUD->PlayerHealthBar;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
			}
		}
	}
}

float ALab4PlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();

	// Если на клиента, то прибавляем сетевые задержки и разницу в запуске приложения
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ALab4PlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceit = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceit);
}

void ALab4PlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                  float TimeServerReceivedClientRequest)
{
	// Вычисление RTT
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ALab4PlayerController::ServerGetGameModeSettings_Implementation()
{
	ALab4GameMode* Lab4GameMode = Cast<ALab4GameMode>(UGameplayStatics::GetGameMode(this));

	if (Lab4GameMode)
	{
		MatchStartTime = Lab4GameMode->StartMatchTime;
		LevelStartTime = Lab4GameMode->LevelStartTime;
		MatchState = Lab4GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, LevelStartTime, MatchStartTime);

		// if (Lab4HUD && MatchState == MatchState::WaitingToStart)
		// {
		// 	Lab4HUD->AddAnnouncement();
		// }
	}
}

void ALab4PlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float LevelTime, float WarmupTime)
{
	LevelStartTime = LevelTime;
	MatchStartTime = WarmupTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	
	if (Lab4HUD && MatchState == MatchState::WaitingToStart)
	{
		Lab4HUD->AddAnnouncement();
	}
}

void ALab4PlayerController::SetRestartCountdownTime(int32 CountdownTime)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;

	if (Lab4HUD &&
		Lab4HUD->GameOverWidget
		)
	{
		Lab4HUD->GameOverWidget->SetRestartTimer(CountdownTime);
	}
}

void ALab4PlayerController::SetRestartCountdownTimer()
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("DecreaseCountdownTime"));
	
	GetWorldTimerManager().SetTimer(
		CountdownTimer,
		TimerDelegate,
		1.0f,
		true,
		0.0f
		);
}

void ALab4PlayerController::DecreaseCountdownTime()
{
	SetRestartCountdownTime(--ResetCountdownTime);

	if (ResetCountdownTime <= 0)
	{
		Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;
		GetWorldTimerManager().ClearTimer(CountdownTimer);
		ResetCountdownTime = 6;

		if (Lab4HUD &&
			Lab4HUD->GameOverWidget
			)
		{
			Lab4HUD->GameOverWidget->HideWinnerWidget();
		}
	}
}

void ALab4PlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;

	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ALab4PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	const ALab4Character* Lab4Character = Cast<ALab4Character>(InPawn);

	if (Lab4Character)
	{
		SetHUDHealth(Lab4Character->GetCurrentHealth(), Lab4Character->GetMaxHealth());
	}
}

void ALab4PlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;

		if (Lab4HUD)
		{
			Lab4HUD->AddCharacterOverlay();
			
			if (Lab4HUD->AnnouncementWidget)
			{
				Lab4HUD->AnnouncementWidget->RemoveFromViewport();
			}
		}
	}
}

void ALab4PlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;
	if (Lab4HUD &&
		Lab4HUD->PlayerHealthBar &&
		Lab4HUD->PlayerHealthBar->HealthProgressBar &&
		Lab4HUD->PlayerHealthBar->HealthText
		)
	{
		const float HealthPersantage = Health / MaxHealth;
		Lab4HUD->PlayerHealthBar->HealthProgressBar->SetPercent(HealthPersantage);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::FloorToInt(Health), FMath::FloorToInt(MaxHealth));
		Lab4HUD->PlayerHealthBar->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDMaxHealth = MaxHealth;
		HUDHealth = Health;
	}
}

void ALab4PlayerController::SetHUDScore(float Score)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;
	if (Lab4HUD &&
		Lab4HUD->PlayerHealthBar &&
		Lab4HUD->PlayerHealthBar->ScoreText)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		Lab4HUD->PlayerHealthBar->ScoreText->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void ALab4PlayerController::SetStartMatchCountdownTime(float CountdownTime)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;
	if (Lab4HUD &&
		Lab4HUD->AnnouncementWidget &&
		Lab4HUD->AnnouncementWidget->StartMatchTimer)
	{
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		const int32 Seconds = CountdownTime - Minutes * 60;
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		Lab4HUD->AnnouncementWidget->StartMatchTimer->SetText(FText::FromString(CountdownText));
	}
}

void ALab4PlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = MatchStartTime - GetServerTime() + LevelStartTime;
	}
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetStartMatchCountdownTime(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void ALab4PlayerController::BroadcastAnnouncement(APlayerState* AttackerPlayerState,
                                                  APlayerState* VictimPlayerState)
{
	ClientElimAnnouncement(AttackerPlayerState, VictimPlayerState);
}

void ALab4PlayerController::BroadcastGameOverAnnouncement(ALab4PlayerState* WinnerPlayerState, float NormalizedPlayerScores)
{
	if (WinnerPlayerState)
	{
		ClientGameOverToggle(WinnerPlayerState, NormalizedPlayerScores);
	}
}

void ALab4PlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;

		if (Lab4HUD)
		{
			Lab4HUD->AddCharacterOverlay();

			if (Lab4HUD->AnnouncementWidget)
			{
				Lab4HUD->AnnouncementWidget->RemoveFromViewport();
			}
		}
	}
}

void ALab4PlayerController::ClientGameOverToggle_Implementation(ALab4PlayerState* WinnerPlayerState, float NormalizedPlayerScore)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;

	if (Lab4HUD && WinnerPlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("Show UI winner"))
		Lab4HUD->ShowGameOverWidget(WinnerPlayerState, NormalizedPlayerScore);
		SetRestartCountdownTimer();
	}
}

void ALab4PlayerController::ClientElimAnnouncement_Implementation(APlayerState* AttackerPlayerState,
                                                                  APlayerState* VictimPlayerState)
{
	Lab4HUD = Lab4HUD == nullptr ? Cast<ALab4HUD>(GetHUD()) : Lab4HUD;
	APlayerState* Self = GetPlayerState<APlayerState>();
	
	if (Self && AttackerPlayerState && VictimPlayerState)
	{
		if (Lab4HUD)
		{
			if (AttackerPlayerState == Self && Self != VictimPlayerState)
			{
				// for attacker
				Lab4HUD->AddElimOverlay("You", VictimPlayerState->GetPlayerName());
				return;
			}
			if (VictimPlayerState == Self && Self != AttackerPlayerState)
			{
				// for victim
				Lab4HUD->AddElimOverlay(AttackerPlayerState->GetPlayerName(), "you");
				return;
			}
			if (VictimPlayerState == Self && AttackerPlayerState == Self)
			{
				// suicide
				Lab4HUD->AddElimOverlay("You", "yourself");
			}
			Lab4HUD->AddElimOverlay(AttackerPlayerState->GetPlayerName(), VictimPlayerState->GetPlayerName());
		}
	}
}
