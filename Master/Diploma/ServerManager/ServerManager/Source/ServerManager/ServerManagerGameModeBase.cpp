// Copyright Epic Games, Inc. All Rights Reserved.


#include "ServerManagerGameModeBase.h"
#include "ServerManager/Public/PlayerControllers/ServerManagerPlayerController.h"

AServerManagerGameModeBase::AServerManagerGameModeBase()
{
	PlayerControllerClass = AServerManagerPlayerController::StaticClass();
}

void AServerManagerGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	
}
