// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "Lab4GameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDiedSignature, ACharacter*, Character);

UCLASS(minimalapi)
class ALab4GameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ALab4GameMode();

	const FOnPlayerDiedSignature& GetOnPlayerDied() const { return OnPlayerDied; }

	UFUNCTION(Exec)
	void Pause(const FString PlayerName);

	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	virtual void PlayerEliminated(
		class ALab4Character* ElimmedCharacter,
		class ALab4PlayerController* VictimController,
		ALab4PlayerController* AttackerController
		);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	FOnPlayerDiedSignature OnPlayerDied;
	
private:
	int32 TotalFrags;
};



