#include "MainMenuInitializer.h"

#include "InputSaver.h"
#include "JoinGameUpdater.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Lab4/Lab4PlayerController.h"
#include "Lab4/Lab4PlayerState.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"
#include "Lab4/UserWidgets/MainMenu.h"

AMainMenuInitializer::AMainMenuInitializer()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuWBP(TEXT("/Game/MainMenu/WBP_MainMenu"));
	m_MainMenuWidget = MainMenuWBP.Class;
}

void AMainMenuInitializer::BeginPlay()
{
	Super::BeginPlay();

	m_pGameInstance = GetGameInstance<ULab4GameInstance>();
	m_pGameInstance->ConnectMainMenuInitializer(this);
	
	m_pMainMenu = CreateWidget<UMainMenu>(m_pGameInstance, m_MainMenuWidget);
	m_pMainMenu->SetMainMenu(this);
	m_pMainMenu->Setup();

	UWorld* const pWorld = GetWorld();

	m_pInputSaver = pWorld->SpawnActor<AInputSaver>(AInputSaver::StaticClass());

	const FString ServerName = m_pInputSaver->LoadServerName();
	const FString PlayerName = m_pInputSaver->LoadPlayerName();

	GamerName = PlayerName;
	m_pMainMenu->SetServerName(ServerName);
	m_pMainMenu->SetPlayerName(PlayerName);
	
	m_pGameInstance->SetServerName(ServerName);
	m_pGameInstance->SetPlayerName(PlayerName);

	m_pServersListUpdater = pWorld->SpawnActor<AJoinGameUpdater>(AJoinGameUpdater::StaticClass());
	m_pServersListUpdater->SetMainMenu(this);
}

void AMainMenuInitializer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// PollInit();
}

void AMainMenuInitializer::OnWidgetToJoin() const
{
	
	bool isLan = m_pGameInstance->GetIsLanGame();

	if (isLan)
	{
		m_pGameInstance->RefreshServersList();
		return;
	}
	
	m_pGameInstance->FindSessions();
}

void AMainMenuInitializer::OnWidgetToCreate() const
{
	
}

void AMainMenuInitializer::OnWidgetToMain(bool bIsFromCreate) const
{
	if (!bIsFromCreate)
	{
		m_pServersListUpdater->StopTimer();
	}
}

void AMainMenuInitializer::OnWidgetToPlayerNameAtJoining(uint32 Index) const
{
	m_pServersListUpdater->StopTimer();
	m_pGameInstance->SetJoinIndex(Index);
}

void AMainMenuInitializer::OnWidgetToPlayerNameAtCreation(const FString& ServerName) const
{
	m_pGameInstance->SetServerName(ServerName);
	m_pInputSaver->SaveServerName(ServerName);
}

void AMainMenuInitializer::OnWidgetToStartGame(const FString& PlayerName, bool bIsFromCreate) const
{
	m_pInputSaver->SavePlayerName(PlayerName);
	m_pGameInstance->SetPlayerName(PlayerName);

	if (bIsFromCreate)
	{
		m_pGameInstance->Host();
	}
	else
	{
		m_pGameInstance->Join();
	}
}

void AMainMenuInitializer::OnInstanceFoundServers(const TArray<FString>& ServerNames) const
{
	m_pMainMenu->SetServersList(ServerNames);

	if (ServerNames.Num() == 0)
	{
		m_pGameInstance->RefreshServersList();
	}
	else
	{
		m_pServersListUpdater->BeginTimer(m_UpdateServersListTime);
	}
}

void AMainMenuInitializer::SetWidgetOnLoginComplete()
{
	if (m_pMainMenu == nullptr) return;
	m_pMainMenu->SetWidgetOnLoginComplete();
}

TArray<FText> AMainMenuInitializer::GetCredentials()
{
	return m_pMainMenu->GetCredentials();
}

void AMainMenuInitializer::OnUpdaterTimerRunOut() const
{
	m_pGameInstance->RefreshServersList();
}

void AMainMenuInitializer::PollInit()
{
	if (Lab4PlayerState == nullptr)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
		
		if (PlayerController && PlayerController->PlayerState)
		{
			Lab4PlayerState = Cast<ALab4PlayerState>(PlayerController->PlayerState);
			if (Lab4PlayerState)
			{
				Lab4PlayerState->SetPlayerName(GamerName);
			}
		}
	}
}

void AMainMenuInitializer::TeardownAll() const
{
	m_pMainMenu->Teardown();
	m_pServersListUpdater->StopTimer();
}
