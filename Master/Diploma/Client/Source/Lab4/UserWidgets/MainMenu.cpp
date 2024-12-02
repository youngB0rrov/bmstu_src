#include "MainMenu.h"
#include "UObject/ConstructorHelpers.h"
#include "ServerRow.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Lab4/Actors/MainMenuInitializer.h"
#include "Lab4/GameInstances/Lab4GameInstance.h"
#include "Kismet/GameplayStatics.h"

#define WITH_SDK 0

UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> ServerRowWBP(
		TEXT("/Game/MainMenu/WBP_ServerRow"));

	m_ServerRowWidget = ServerRowWBP.Class;

	bWasLoggedIn = false;

	if (UserPassword != nullptr)
	{
		UserPassword->SetIsPassword(true);
	}
}

bool UMainMenu::Initialize()
{
	if (!Super::Initialize()) return false;

	CreateGameButton->OnClicked.AddDynamic(this, &UMainMenu::OnClickedCreate);
	JoinGameButton->OnClicked.AddDynamic(this, &UMainMenu::OnClickedJoin);
	CancelJoinConfirmButton->OnClicked.AddDynamic(this, &UMainMenu::OnClickedMain);
	JoinGameConfirmButton->OnClicked.AddDynamic(this, &UMainMenu::OnClickedPlayerNameAtJoining);
	CancelSessionNameConfirmButton->OnClicked.AddDynamic(this, &UMainMenu::OnClickedMain);
	SessionNameConfirmButton->OnClicked.AddDynamic(this, &UMainMenu::OnClickedPlayerNameAtCreation);
	CancelNameConfirmButton->OnClicked.AddDynamic(this, &UMainMenu::OnClickedMain);
	NameConfirmButton->OnClicked.AddDynamic(this, &UMainMenu::OnClickedStartGame);
	InternetButton->OnClicked.AddDynamic(this, &UMainMenu::UMainMenu::OnInternetButtonClicked);
	CancelInternetGame->OnClicked.AddDynamic(this, &UMainMenu::UMainMenu::OnToSelectModeButtonClicked);
	LogInButton->OnClicked.AddDynamic(this, &UMainMenu::OnLogInButtonClicked);
	LoginAccountButton->OnClicked.AddDynamic(this, &UMainMenu::OnLoginViaAccountButtonClicked);
	CancelAuthButton->OnClicked.AddDynamic(this, &UMainMenu::OnCancelAuthButtonClicked);
	LANButton->OnClicked.AddDynamic(this, &UMainMenu::OnLanButtonClicked);
	MainMenuExitButton->OnClicked.AddDynamic(this, &UMainMenu::OnMainMenuExitButtonClicked);
	CancelCredentialsButton->OnClicked.AddDynamic(this, &UMainMenu::UMainMenu::OnCancelCredentialsClicked);
	ConfirmCredentials->OnClicked.AddDynamic(this, &UMainMenu::UMainMenu::OnConfirmCredentialsClicked);
	TogglePassword->OnPressed.AddDynamic(this, &UMainMenu::UMainMenu::OnTogglePasswordButtonPressed);
	TogglePassword->OnReleased.AddDynamic(this, &UMainMenu::UMainMenu::OnTogglePasswordButtonReleased);
	LeaderboardsButton->OnClicked.AddDynamic(this, &UMainMenu::OnLeaderboardsButtonClicked);
	LeaderboardBackButton->OnClicked.AddDynamic(this, &UMainMenu::OnLeaderboardBackButtonClicked);
	RecruitsLeagueButton->OnClicked.AddDynamic(this, &UMainMenu::UMainMenu::OnRecruitsLeagueButtonClicked);
	GuardiansLeagueButton->OnClicked.AddDynamic(this, &UMainMenu::UMainMenu::OnGuardiansLeagueButtonClicked);
	CrusaidersLeagueButton->OnClicked.AddDynamic(this, &UMainMenu::UMainMenu::OnCrusaidersLeagueButtonClicked);
	LegendsLeagueButton->OnClicked.AddDynamic(this, &UMainMenu::UMainMenu::OnLegendsLeagueButtonClicked);
	MainMenuMatchmakingButton->OnClicked.AddDynamic(this, &UMainMenu::OnMatchmakingButtonClicked);
	MatchmakingCreateButton->OnClicked.AddDynamic(this, &UMainMenu::OnMatchmakingCreateButtonClicked);
	MatchmakingFindButton->OnClicked.AddDynamic(this, &UMainMenu::OnMatchmakingFindButtonClicked);
	MatchmakingBackButton->OnClicked.AddDynamic(this, &UMainMenu::OnMatchmakingBackButtonClicked);

	StatusSizeBox->SetVisibility(ESlateVisibility::Hidden);
	MainMenuMatchmakingButton->SetIsEnabled(false);
	LeaderboardsButton->SetIsEnabled(false);
	return true;
}

void UMainMenu::SetMainMenu(AMainMenuInitializer* const pMainMenu)
{
	m_pMainMenu = pMainMenu;
}

void UMainMenu::SetServersList(const TArray<FString>& List)
{
	if (ServersList->HasAnyChildren())
	{
		ServersList->ClearChildren();
	}
	m_SelectedIndex.Reset();
	
	UWorld *pWorld = GetWorld();

	if (List.Num() == 0)
	{
		UServerRow *pServerRow = CreateWidget<UServerRow>(GetWorld(), m_ServerRowWidget);
		pServerRow->SetServerName(Nothing);
		ServersList->AddChild(pServerRow);
	}
	else
	{
		uint32 i = 0;
		for (const FString& Row : List)
		{
			UServerRow *pServerRow = CreateWidget<UServerRow>(pWorld, m_ServerRowWidget);
			pServerRow->SetServerName(Row);
			pServerRow->Setup(this, i);
			ServersList->AddChild(pServerRow);

			++i;
		}
	}
}

void UMainMenu::SetServerName(const FString& SessionName) const
{
	SessionNameEnterText->SetText(FText::FromString(SessionName));
}

void UMainMenu::SetPlayerName(const FString& Name) const
{
	NameEnterText->SetText(FText::FromString(Name));
}

void UMainMenu::OnClickedCreate()
{
	UE_LOG(LogTemp, Warning, TEXT("Create match button clicked"))
	ULab4GameInstance* gameInstance = GetGameInstance<ULab4GameInstance>();
	if (gameInstance == nullptr) return;

	if (gameInstance->GetIfCanStartDedicated())
	{
		m_bIsCreateGame = true;
		MenuSwitcher->SetActiveWidget(SessionNameMenu);
		m_pMainMenu->OnWidgetToCreate();
		return;
	}
	if (gameInstance->GetIsLanGame() && !gameInstance->GetIfCanStartDedicated())
	{
		gameInstance->MatchmakingInputWidget->AddToViewport();
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Starting purchasing process from main menu"))
	gameInstance->StartPurchase();
}

void UMainMenu::OnClickedJoin()
{
	m_bIsCreateGame = false;
	m_SelectedIndex.Reset();
	MenuSwitcher->SetActiveWidget(JoinMenu);

	ServersList->ClearChildren();
	UServerRow *pServerRow = CreateWidget<UServerRow>(GetWorld(), m_ServerRowWidget);
	pServerRow->SetServerName(Waiting);
	ServersList->AddChild(pServerRow);

	m_pMainMenu->OnWidgetToJoin();
}

void UMainMenu::OnClickedMain()
{
	MenuSwitcher->SetActiveWidget(MainMenu);

	m_pMainMenu->OnWidgetToMain(m_bIsCreateGame);
}

void UMainMenu::OnClickedPlayerNameAtJoining()
{
	if (!m_SelectedIndex.IsSet()) return;
	
	if (GetGameInstance<ULab4GameInstance>()->GetIsLanGame())
	{
		MenuSwitcher->SetActiveWidget(NameMenu);
		
		// запомнить выбранный индекс выбираемой комнаты
		m_pMainMenu->OnWidgetToPlayerNameAtJoining(m_SelectedIndex.GetValue());
	}
	else
	{
		m_pMainMenu->OnWidgetToPlayerNameAtJoining(m_SelectedIndex.GetValue());
		m_pMainMenu->OnWidgetToStartGame(FString(""), m_bIsCreateGame);
	}

}

void UMainMenu::OnClickedPlayerNameAtCreation()
{
	UE_LOG(LogTemp, Warning, TEXT("Clicked Accept"));
	if (SessionNameEnterText->GetText().IsEmpty()) return;
	
	if (GetGameInstance<ULab4GameInstance>()->GetIsLanGame())
	{
		MenuSwitcher->SetActiveWidget(NameMenu);
		
		// Запомнить введенное имя комнаты
		m_pMainMenu->OnWidgetToPlayerNameAtCreation(SessionNameEnterText->GetText().ToString());
	}
	else
	{
		// Запомнить введенное имя комнаты
		UE_LOG(LogTemp, Warning, TEXT("Clicked online"));
		m_pMainMenu->OnWidgetToPlayerNameAtCreation(SessionNameEnterText->GetText().ToString());
		m_pMainMenu->OnWidgetToStartGame(FString(""), m_bIsCreateGame);
	}
}

void UMainMenu::OnClickedStartGame()
{
	const FString EnteredPlayerName = NameEnterText->GetText().ToString();
	
	if (EnteredPlayerName.IsEmpty()) return;

	m_pMainMenu->OnWidgetToStartGame(EnteredPlayerName, m_bIsCreateGame);
}

void UMainMenu::OnLanButtonClicked()
{
	GetGameInstance<ULab4GameInstance>()->SetIsLanGame(true);

	if (MenuSwitcher == nullptr || MainMenu == nullptr) return;
	
	MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::OnInternetButtonClicked()
{
	ULab4GameInstance* Lab4GameInstance = GetGameInstance<ULab4GameInstance>();

	if (Lab4GameInstance != nullptr)
	{
		Lab4GameInstance->SetIsOnlineGame(false);
	}
	
	if (MainMenu == nullptr || AuthMenu == nullptr) return;

	if (Lab4GameInstance && !Lab4GameInstance->GetLoginStatus())
	{
		MenuSwitcher->SetActiveWidget(AuthMenu);
		return;
	}
	
	MenuSwitcher->SetActiveWidget(MainMenu);	
	
}

void UMainMenu::OnToSelectModeButtonClicked()
{
	if (ModeSelect == nullptr) return;
	
	MenuSwitcher->SetActiveWidget(ModeSelect);
}

void UMainMenu::OnTogglePasswordButtonPressed()
{
	if (UserPassword != nullptr)
	{
		UserPassword->SetIsPassword(false);
	}
}

void UMainMenu::OnTogglePasswordButtonReleased()
{
	if (UserPassword != nullptr)
	{
		UserPassword->SetIsPassword(true);
	}
}

void UMainMenu::OnLogInButtonClicked()
{
	GetGameInstance<ULab4GameInstance>()->LogIn();
	bWasLoggedIn = true;
}

void UMainMenu::OnCancelAuthButtonClicked()
{
	if (ModeSelect == nullptr) return;
	
	MenuSwitcher->SetActiveWidget(ModeSelect);
}

void UMainMenu::OnMainMenuExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void UMainMenu::OnLoginViaAccountButtonClicked()
{
	if (Credentials != nullptr)
	{
		MenuSwitcher->SetActiveWidget(Credentials);
	}
}

void UMainMenu::OnCancelCredentialsClicked()
{
	if (AuthMenu != nullptr)
	{
		MenuSwitcher->SetActiveWidget(AuthMenu);
	}
}

void UMainMenu::OnConfirmCredentialsClicked()
{
	ULab4GameInstance* GameInstance = GetGameInstance<ULab4GameInstance>();

	if (GameInstance != nullptr)
	{
		if (UserEmail->GetText().ToString().IsEmpty() || UserPassword->GetText().ToString().IsEmpty())
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("User and password are required"), true,
				FVector2D(2.f));
			return;
		}
		GameInstance->LoginViaCredentials();
	}
}

void UMainMenu::OnLeaderboardsButtonClicked()
{
	if (Leaderboard != nullptr && !GetGameInstance<ULab4GameInstance>()->GetIsLanGame())
	{
		MenuSwitcher->SetActiveWidget(Leaderboard);
		ULab4GameInstance* GameInstance = GetGameInstance<ULab4GameInstance>();

		if (GameInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("Calling query global ranks"));
			GameInstance->QueryGlobalRanks(-2000, -1000);
		}
	}
}

void UMainMenu::OnLeaderboardBackButtonClicked()
{
	if (MenuSwitcher != nullptr)
	{
		MenuSwitcher->SetActiveWidget(MainMenu);
		ClearRankedLeaderboardList();

		if (LeagueNameText != nullptr)
		{
			LeagueNameText->SetText(FText::FromString(FString::Printf(TEXT("Recruits"))));
		}
	}
}

void UMainMenu::OnRecruitsLeagueButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("League button clicked"));
	
	if (LeagueNameText == nullptr)
	{
		return;
	}
	
	LeagueNameText->SetText(FText::FromString(FString::Printf(TEXT("Recruits"))));
	ULab4GameInstance* GameInstance = GetGameInstance<ULab4GameInstance>();

	if (GameInstance == nullptr)
	{
		return;
	}

	GameInstance->QueryGlobalRanks(-2000, -1000);
}

void UMainMenu::OnGuardiansLeagueButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("League button clicked"));
	
	if (LeagueNameText == nullptr)
	{
		return;
	}

	LeagueNameText->SetText(FText::FromString(FString::Printf(TEXT("Guardians"))));
	ULab4GameInstance* GameInstance = GetGameInstance<ULab4GameInstance>();

	if (GameInstance == nullptr)
	{
		return;
	}

	GameInstance->QueryGlobalRanks(-999, 0);
}

void UMainMenu::OnCrusaidersLeagueButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("League button clicked"));
	
	if (LeagueNameText == nullptr)
	{
		return;
	}

	LeagueNameText->SetText(FText::FromString(FString::Printf(TEXT("Crusaiders"))));
	ULab4GameInstance* GameInstance = GetGameInstance<ULab4GameInstance>();

	if (GameInstance == nullptr)
	{
		return;
	}

	GameInstance->QueryGlobalRanks(1, 1000);
}

void UMainMenu::OnLegendsLeagueButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("League button clicked"));
	
	if (LeagueNameText == nullptr)
	{
		return;
	}

	LeagueNameText->SetText(FText::FromString(FString::Printf(TEXT("Legends"))));
	ULab4GameInstance* GameInstance = GetGameInstance<ULab4GameInstance>();
	
	if (GameInstance == nullptr)
	{
		return;
	}

	GameInstance->QueryGlobalRanks(1001, 2000);
}

void UMainMenu::OnMatchmakingButtonClicked()
{
	if (MenuSwitcher == nullptr || Matchmaking == nullptr) return;
	MenuSwitcher->SetActiveWidget(Matchmaking);
}

void UMainMenu::OnMatchmakingCreateButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Create matchmaking button clicked"))
	ULab4GameInstance* gameInstance = GetGameInstance<ULab4GameInstance>();
	if (gameInstance == nullptr) return;

	if (gameInstance->GetIfCanStartDedicated())
	{
		gameInstance->CreateSocketConnection();
		gameInstance->InitializeReceiveSocketThread();
		gameInstance->SendMessageToHostSocket(FString::Printf(TEXT("CREATE")));

		return;
	}
	if (!gameInstance->GetIsLanGame() && !gameInstance->GetIfCanStartDedicated())
	{
		UE_LOG(LogTemp, Log, TEXT("Starting purchasing process from main menu"))
		gameInstance->StartPurchase();

		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Matchmaking is available in online mode only"));
}

void UMainMenu::OnMatchmakingFindButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Find matchmaking button clicked"))
	ULab4GameInstance* gameInstance = GetGameInstance<ULab4GameInstance>();
	if (gameInstance == nullptr) return;
	if (gameInstance->GetIsFindingMatchInProgress()) return;

	gameInstance->CreateSocketConnection();
	gameInstance->InitializeReceiveSocketThread();
	gameInstance->SendMessageToHostSocket(FString::Printf(TEXT("JOIN")));
}

void UMainMenu::OnMatchmakingBackButtonClicked()
{
	if (MenuSwitcher == nullptr || MainMenu == nullptr) return;

	SetFindingMatchStatusWidgetVisibility(false);
	MenuSwitcher->SetActiveWidget(MainMenu);

	ULab4GameInstance* gameInstance = GetGameInstance<ULab4GameInstance>();
	if (!gameInstance) return;
	gameInstance->SetFindingMatchProgress(false);
	gameInstance->DisposeReceiveSocketThread();
}

TArray<FText> UMainMenu::GetCredentials()
{
	TArray<FText> EnteredCredentials;
	
	if (UserEmail != nullptr)
	{
		EnteredCredentials.Add(UserEmail->GetText());
	}

	if (UserPassword != nullptr)
	{
		EnteredCredentials.Add(UserPassword->GetText());
	}

	return EnteredCredentials;
}

void UMainMenu::Setup()
{
	this->AddToViewport();

	APlayerController *pPlayerController = GetWorld()->GetFirstPlayerController();

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	pPlayerController->SetInputMode(InputModeData);
	pPlayerController->bShowMouseCursor = true;
}

void UMainMenu::Teardown()
{
	this->RemoveFromViewport();

	APlayerController *PlayerController = GetWorld()->GetFirstPlayerController();

	const FInputModeGameOnly InputModeData;

	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = false;
}

void UMainMenu::SetWidgetOnLoginComplete()
{
	if (MenuSwitcher == nullptr || MainMenu == nullptr) return;
	
	MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::AddRankedLeaderBoardRow(URankedLeaderboardRow* PlayerTableRow)
{
	if (RankedPlayersList == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerTableRow pointer is NULL"));
		return;
	}

	RankedPlayersList->AddChild(PlayerTableRow);
}

void UMainMenu::ClearRankedLeaderboardList()
{
	if (RankedPlayersList == nullptr)
	{
		return;
	}

	RankedPlayersList->ClearChildren();
}

void UMainMenu::SetFindingMatchStatusWidgetVisibility(bool bIsVisible)
{
	if (StatusSizeBox == nullptr) return;
	bIsVisible ? StatusSizeBox->SetVisibility(ESlateVisibility::Visible) : StatusSizeBox->SetVisibility(ESlateVisibility::Hidden);
}

void UMainMenu::SelectIndex(uint32 Index)
{
	UpdateChildren(Index);
	m_SelectedIndex = Index;
}

void UMainMenu::UpdateChildren(uint32 NewIndexSelected)
{
	if (m_SelectedIndex.IsSet())
	{
		UServerRow *pRow = Cast<UServerRow>(ServersList->GetChildAt(m_SelectedIndex.GetValue()));
		if (pRow != nullptr)
		{
			pRow->bIsSelected = false;
		}
	}

	UServerRow *pRow = Cast<UServerRow>(ServersList->GetChildAt(NewIndexSelected));
	if (pRow != nullptr)
	{
		pRow->bIsSelected = true;
	}
}

void UMainMenu::SetMatchmakingHintTextVisibility(bool bIsVisible)
{
	if (MatchmakingCreateButtonHintText == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MatchmakingCreateButtonHintTextPtr in invalid"))
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Setting MatchmakingHintText visibility to false"))
	MatchmakingCreateButtonHintText->SetVisibility(ESlateVisibility::Hidden);
}

void UMainMenu::SetCreateGameHintTextVisibility(bool bIsVisible)
{
	if (CreateGameHintText == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MatchmakingCreateButtonHintTextPtr in invalid"))
		return;
	}
	CreateGameHintText->SetVisibility(ESlateVisibility::Hidden);
}

void UMainMenu::HandleMatchmakingStatusAndConnect()
{
	SetMatchmakingStatusText("Connecting...");
	
	ULab4GameInstance* gameInstance = GetGameInstance<ULab4GameInstance>();
	if (!gameInstance) return;

	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FString connectionString = gameInstance->GetConnectionAddress();
	playerController->ClientTravel(connectionString, ETravelType::TRAVEL_Absolute);
}

void UMainMenu::ShowOnlineOnlyButtons()
{
	if (MainMenuMatchmakingButton == nullptr) return;
	if (LeaderboardsButton == nullptr) return;

	LeaderboardsButton->SetIsEnabled(true);
	MainMenuMatchmakingButton->SetIsEnabled(true);
}

void UMainMenu::GiveAccessToCreateMatchSection()
{
	m_bIsCreateGame = true;
	MenuSwitcher->SetActiveWidget(SessionNameMenu);
	m_pMainMenu->OnWidgetToCreate();
}

void UMainMenu::SetMatchmakingStatusText(const FString& StatusText)
{
	if (MatchmakingStatusText == nullptr) return;
	MatchmakingStatusText->SetText(FText::FromString(StatusText));
}
