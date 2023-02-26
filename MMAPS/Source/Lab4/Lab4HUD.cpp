// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab4HUD.h"

#include "Lab4Character.h"
#include "Lab4PlayerController.h"
#include "Lab4PlayerState.h"
#include "Actors/GameInitializer.h"
#include "Actors/MainMenuInitializer.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "UserWidgets/PlayerHealthBar.h"
#include "UserWidgets/Lab4ElimAnouncement.h"
#include "Components/HorizontalBox.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UserWidgets/MyUserWidget.h"
#include "UserWidgets/PlayerTable.h"

ALab4HUD::ALab4HUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> BPCharacterOverlay(TEXT("/Game/GameUI/WBP_HealthBar"));
	if (BPCharacterOverlay.Succeeded())
	{
		CharacterOverlayClass = BPCharacterOverlay.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> BPElimText(TEXT("/Game/GameUI/WBP_ElimAnouncment"));
	if (BPElimText.Succeeded())
	{
		ElimAnnouncementClass = BPElimText.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> BPPlayerTable(TEXT("/Game/GameUI/WBP_Player_Table"));
	if (BPPlayerTable.Succeeded())
	{
		PlayerTableClass = BPPlayerTable.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> BPGameOverWidget(TEXT("/Game/MainMenu/WBP_GameOverWidget"));
	if (BPGameOverWidget.Succeeded())
	{
		GameOverWidgetClass = BPGameOverWidget.Class;
	}

	ElimAnnouncementDuration = 3.f;

	bIsSet = false;
}

void ALab4HUD::DrawHUD()
{
	Super::DrawHUD();
	FVector2D ViewportSize;

	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
}

void ALab4HUD::BeginPlay()
{
	Super::BeginPlay();
}

void ALab4HUD::ElimAnnouncementTimerFinished(ULab4ElimAnouncement* MessageToRemove)
{
	if (MessageToRemove)
	{
		MessageToRemove->RemoveFromParent();
	}
}

void ALab4HUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && CharacterOverlayClass)
	{
		PlayerHealthBar = CreateWidget<UPlayerHealthBar>(PlayerController, CharacterOverlayClass);
		PlayerHealthBar->AddToViewport();
		bIsSet = true;
	}
}

void ALab4HUD::AddElimOverlay(FString Attacker, FString Victim)
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && ElimAnnouncementClass)
	{
		ULab4ElimAnouncement* ElimAnnouncementWidget = CreateWidget<ULab4ElimAnouncement>(
			PlayerController, ElimAnnouncementClass);
		if (ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncement(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();

			// for (ULab4ElimAnouncement* ElimMsg : ElimMessages)
			// {
			// 	if (ElimMsg && ElimMsg->ElimBlock)
			// 	{
			// 		// Необходимо получить канвас виджета, чтобы перемещать его
			// 		UCanvasPanelSlot* CanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ElimMsg->ElimBlock);
			// 		if (CanvasPanelSlot)
			// 		{
			// 			FVector2D Position = CanvasPanelSlot->GetPosition();
			// 			FVector2D NewPosition = FVector2D(
			// 				Position.X,
			// 				Position.Y - CanvasPanelSlot->GetSize().Y
			// 				);
			// 			CanvasPanelSlot->SetPosition(NewPosition);
			// 		}
			// 	}
			// }

			ElimMessages.Add(ElimAnnouncementWidget);
			FTimerHandle ElimAnnouncementTimer;
			FTimerDelegate ElimMessageDelegate;
			ElimMessageDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(
				ElimAnnouncementTimer,
				ElimMessageDelegate,
				ElimAnnouncementDuration,
				false
			);
		}
	}
}

void ALab4HUD::ShowGameOverWidget(const ALab4PlayerState* WinnerPlayerState)
{
	APlayerController* 	PlayerController = GetOwningPlayerController();

	if (PlayerController && GameOverWidgetClass)
	{
		GameOverWidget = CreateWidget<UMyUserWidget>(PlayerController, GameOverWidgetClass);
	}

	if (GameOverWidget && WinnerPlayerState)
	{
		GameOverWidget->SetWinnerText(WinnerPlayerState);
		GameOverWidget->AddToViewport();
	}
}

void ALab4HUD::AddPlayerList()
{
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && PlayerTableClass)
	{
		PlayerTableWidget = CreateWidget<UPlayerTable>(PlayerController, PlayerTableClass);
		
		if (PlayerTableWidget)
		{
			PlayerTableWidget->SetPlayersList();
			PlayerTableWidget->AddToViewport();
		}
	}
}

void ALab4HUD::RemovePlayerList()
{
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && PlayerTableWidget)
	{
		PlayerTableWidget->RemoveFromParent();
	}
}
