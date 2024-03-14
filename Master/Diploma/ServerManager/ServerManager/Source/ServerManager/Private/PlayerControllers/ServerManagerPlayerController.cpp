// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/ServerManagerPlayerController.h"

void AServerManagerPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	FInputModeUIOnly inputMode;
	inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(inputMode);
	bShowMouseCursor = true;
}
