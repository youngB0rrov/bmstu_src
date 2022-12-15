#include "JoinGameUpdater.h"

#include "MainMenuInitializer.h"

AJoinGameUpdater::AJoinGameUpdater()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AJoinGameUpdater::BeginPlay()
{
	Super::BeginPlay();

	m_TimerManager = &GetWorldTimerManager();
}

void AJoinGameUpdater::SetMainMenu(AMainMenuInitializer* const pMainMenu)
{
	m_pMainMenu = pMainMenu;
}

void AJoinGameUpdater::BeginTimer(float TimeUpdate)
{
	m_TimerManager->SetTimer(m_TimerHandle, this, &AJoinGameUpdater::OnTimerRunOut, TimeUpdate);
}

void AJoinGameUpdater::StopTimer()
{
	m_TimerManager->ClearTimer(m_TimerHandle);
}

void AJoinGameUpdater::OnTimerRunOut() const
{
	m_pMainMenu->OnUpdaterTimerRunOut();
}
