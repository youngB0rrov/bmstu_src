// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "Lab4GameMode.generated.h"

UCLASS(minimalapi)
class ALab4GameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ALab4GameMode();
	
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	virtual void PlayerEliminated(
		class ALab4Character* ElimmedCharacter,
		class ALab4PlayerController* VictimController,
		ALab4PlayerController* AttackerController
		);
	
protected:
	virtual void BeginPlay() override;
	
private:
	int32 TotalFrags;
};



