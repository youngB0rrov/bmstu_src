#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JoinGameUpdater.generated.h"

UCLASS()
class LAB4_API AJoinGameUpdater : public AActor
{
	GENERATED_BODY()

public:
	AJoinGameUpdater();

	void SetMainMenu(class AMainMenuInitializer* const pMainMenu);
	
	void BeginTimer(float TimeUpdate);
	void StopTimer();
	void OnTimerRunOut() const;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	class AMainMenuInitializer *m_pMainMenu;
	
	FTimerManager *m_TimerManager;
	FTimerHandle m_TimerHandle;
};
