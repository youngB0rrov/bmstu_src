#include "Lab4GameInstance.h"

#include <eos_auth.h>
#include <string>

#include "EOSSettings.h"
#include "eos_init.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Lab4/Lab4Character.h"
#include "Lab4/Actors/MainMenuInitializer.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "Lab4/Lab4GameMode.h"
#include "Lab4/UserWidgets/GameOverMenu.h"
#include "Lab4/UserWidgets/WinnerWiddget.h"

void ULab4GameInstance::Init()
{
	Super::Init();

	bIsLanGame = false;

	// Получить адрес подсистемы
	OnlineSubsystem = IOnlineSubsystem::Get();
	
	if (OnlineSubsystem == nullptr) return;

	// Получить адрес API сессий
	SessionPtr = OnlineSubsystem->GetSessionInterface();
	
	if(!SessionPtr.IsValid()) return;
	
	SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnCreateSessionComplete);
	SessionPtr->OnDestroySessionCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnDestroySessionComplete);
	SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnJoinSessionComplete);
	SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnFindSessionsComplete);

	if (GEngine != nullptr)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &ULab4GameInstance::OnNetworkFailure);
		UE_LOG(LogTemp, Error, TEXT("Ну удалось подключиться к удаленной игровой сессии"));
		
	}

	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ULab4GameInstance::Tick), 0.1f);
}

void ULab4GameInstance::Shutdown()
{
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
	Super::Shutdown();
}

bool ULab4GameInstance::Tick(float DeltaSeconds)
{
	if (PlatformInterface != nullptr)
	{
		EOS_Platform_Tick(PlatformInterface);
	}
	return true;
}


void ULab4GameInstance::ConnectMainMenuInitializer(AMainMenuInitializer* const pInitializer)
{
	m_pMainMenu = pInitializer;
}

FString ULab4GameInstance::GetPlayerName()
{
	return m_PlayerName;
}

ULab4GameInstance::ULab4GameInstance()
{
	bWasLoggedIn = false;
	bShouldBePaused = false;
	
	static ConstructorHelpers::FClassFinder<UUserWidget> BPInGameMenu(TEXT("/Game/MainMenu/WBP_ToMainMenu"));
	BPInGameMenuClass = BPInGameMenu.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> BPGameOver(TEXT("/Game/MainMenu/WBP_GameOverMenu"));
	if (BPGameOver.Succeeded())
	{
		BPGameOverMenu = BPGameOver.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> BPHealthBar(TEXT("/Game/GameUI/WBP_HealthBar"));

	if (BPHealthBar.Succeeded())
	{
		BPHealthBarClass = BPHealthBar.Class;
	}
	
	static ConstructorHelpers::FClassFinder<UUserWidget> BPWinnerWidgetClassFinder(TEXT("/Game/MainMenu/WBP_WinnerWidget"));

	if (BPWinnerWidgetClassFinder.Succeeded())
	{
		BPWinnerWidgetClass = BPWinnerWidgetClassFinder.Class;
	}
	
}

void ULab4GameInstance::SetupWinnerWidget(FString WinnerName)
{
	if (BPWinnerWidgetClass == nullptr) return;
	
	UWinnerWiddget* p_WinnerWidget = CreateWidget<UWinnerWiddget>(this, BPWinnerWidgetClass);
	p_WinnerWidget->SetupWinnerWidget();
	p_WinnerWidget->SetupWinnerName(WinnerName);
}

void ULab4GameInstance::SetPlayerName(const FString& Name)
{
	m_PlayerName = Name;
}

void ULab4GameInstance::SetServerName(const FString& SessionName)
{
	m_ServerName = FName(SessionName);
}

void ULab4GameInstance::SetJoinIndex(const uint32 Index)
{
	m_JoinIndex = Index;
}

void ULab4GameInstance::AddCharacter(ALab4Character* const Character, const FString& Name)
{
	m_PlayerNames.Add(Name);
	m_Characters.Add(Character);

	for (int i = 0; i < m_Characters.Num(); ++i)
	{
		m_Characters[i]->SetPlayerNames(m_PlayerNames);
	}
}

void ULab4GameInstance::RemoveCharacter(ALab4Character* const Character)
{
	const int32 Index = m_Characters.Find(Character);

	if (Index == INDEX_NONE) return;

	m_Characters.RemoveAt(Index);
	m_PlayerNames.RemoveAt(Index);

	for (int i = 0; i < m_Characters.Num(); ++i)
	{
		m_Characters[i]->SetPlayerNames(m_PlayerNames);
	}
}

void ULab4GameInstance::Host() const
{
	if (!SessionPtr.IsValid()) return;

	if (!bIsLanGame)
	{
		if (!bWasLoggedIn) return;
	}
	
	const FNamedOnlineSession *pOnlineSession = SessionPtr->GetNamedSession(SessionNameConst);

	if (pOnlineSession != nullptr)
	{
		SessionPtr->DestroySession(SessionNameConst);
	}
	else
	{
		CreateSession();
	}
}

void ULab4GameInstance::Join() const
{
	if (m_pMainMenu != nullptr)
	{
		m_pMainMenu->TeardownAll();
	}
	
	if (bIsLanGame)
	{
		if (!SessionPtr.IsValid()) return;
	
		if (!m_pSessionSearch.IsValid()) return;

		SessionPtr->JoinSession(0, SessionNameConst, m_pSessionSearch->SearchResults[m_JoinIndex]);
		return;
	}
	
	if (!SessionPtr.IsValid()) return;
	
	if (!SearchSettings.IsValid()) return;

	SessionPtr->JoinSession(0, SessionNameConst, SearchSettings->SearchResults[m_JoinIndex]);
	UE_LOG(LogTemp, Warning, TEXT("Joining session..."));
}

void ULab4GameInstance::RefreshServersList()
{
	if (bIsLanGame)
	{
		m_pSessionSearch = MakeShareable(new FOnlineSessionSearch());

		if (m_pSessionSearch == nullptr) return;
			
		m_pSessionSearch->bIsLanQuery = true;
		m_pSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SessionPtr->FindSessions(0, m_pSessionSearch.ToSharedRef());
		return;
	}
	
	SearchSettings = MakeShareable(new FOnlineSessionSearch());
	
	if (SearchSettings.IsValid())
	{
		SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("Test session"), EOnlineComparisonOp::Equals);
		SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);		
		SessionPtr->FindSessions(0,SearchSettings.ToSharedRef());
	}
}

bool ULab4GameInstance::ChangeConfigToOnline()
{
	return true;
}

bool ULab4GameInstance::ChangeConfigToLan()
{	

	return true;
}

void ULab4GameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	UE_LOG(LogTemp, Warning, TEXT("Success: %d"), Success);
	GetWorld()->ServerTravel(TravelGamePath);
	
	if (!Success) return;
	
	if (m_pMainMenu != nullptr)
	{
		m_pMainMenu->TeardownAll();
	}

	SessionPtr->ClearOnCreateSessionCompleteDelegates(this);
}

void ULab4GameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	UE_LOG(LogTemp, Warning, TEXT("Destroying Session Success %d"), Success);
	
	if (!Success) return;

	if (OnlineSubsystem == nullptr) return;

	SessionPtr = OnlineSubsystem->GetSessionInterface();

	SessionPtr->ClearOnDestroySessionCompleteDelegates(this);
	//CreateSession();
}

void ULab4GameInstance::OnFindSessionsComplete(bool Success)
{
	if (Success && m_pSessionSearch.IsValid() && m_pMainMenu != nullptr)
	{
		TArray<FString> ServersList;
		
		for (const FOnlineSessionSearchResult& Session : m_pSessionSearch->SearchResults)
		{
			FString ServerName;
			Session.Session.SessionSettings.Get(ServerNameKey, ServerName);
			ServersList.Add(ServerName);
		}

		m_pMainMenu->OnInstanceFoundServers(ServersList);
	}
}

void ULab4GameInstance::OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Result)
{
	FString Address;

	if (Result == EOnJoinSessionCompleteResult::Type::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Join Session Success %d"), Result);
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while joining online session"));
	}
	
	if (OnlineSubsystem == nullptr) return;

	SessionPtr = OnlineSubsystem->GetSessionInterface();

	if (SessionPtr == nullptr) return;
	
	SessionPtr->GetResolvedConnectString(Name, Address);

	if (Address.IsEmpty()) return;
	
	//GetFirstLocalPlayerController()->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	if (APlayerController *PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
}

void ULab4GameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("Ошибка запуска сессии/поключения к сессии"))
	LoadMainMenu();
}

void ULab4GameInstance::LogIn()
{
	if (OnlineSubsystem == nullptr) return;
	IOnlineIdentityPtr IdentyPtr = OnlineSubsystem->GetIdentityInterface();

	if (IdentyPtr == nullptr) return;

	FOnlineAccountCredentials AccountCredentials;
	AccountCredentials.Id = FString("");
	AccountCredentials.Token = FString("");
	AccountCredentials.Type = FString("accountportal");

	IdentyPtr->OnLoginCompleteDelegates->AddUObject(this, &ULab4GameInstance::OnLoginComplete);
	IdentyPtr->Login(0, AccountCredentials);
}

void ULab4GameInstance::InitializeSDK()
{
	EOS_InitializeOptions SDKOptions;
	SDKOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
	SDKOptions.AllocateMemoryFunction = nullptr;
	SDKOptions.ReallocateMemoryFunction = nullptr;
	SDKOptions.ReleaseMemoryFunction = nullptr;
	SDKOptions.ProductName = "OnlineMultiplayerShooter";
	SDKOptions.ProductVersion = "1.0";
	SDKOptions.Reserved = nullptr;
	SDKOptions.SystemInitializeOptions = nullptr;
	EOS_EResult SDKInitializeResult = EOS_Initialize(&SDKOptions);

	if (SDKInitializeResult == EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("SDK success"));
	} else if (SDKInitializeResult == EOS_EResult::EOS_AlreadyConfigured)
	{
		UE_LOG(LogTemp, Warning, TEXT("SDK already configured"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SDK another error"));
	}
}

void ULab4GameInstance::InitializePlatformInterface()
{
	EOS_Platform_Options PlatformOptions;
	PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
	PlatformOptions.Reserved = nullptr;
	PlatformOptions.bIsServer = EOS_FALSE;
	PlatformOptions.EncryptionKey = "1111111111111111111111111111111111111111111111111111111111111111";
	PlatformOptions.OverrideCountryCode = nullptr;
	PlatformOptions.OverrideLocaleCode = nullptr;
	PlatformOptions.Flags = 0;
	PlatformOptions.CacheDirectory = nullptr;
	PlatformOptions.ProductId = "8bd04340aae74d73b3ea092484309e0d";
	PlatformOptions.SandboxId = "080960589ec84e44aaf746e6c31bd352";
	PlatformOptions.DeploymentId = "0a33e5dadf2c410281355e84fe717854";
	PlatformOptions.ClientCredentials.ClientId = "xyza7891o7wcWqcrWKfjyH216I7wCdi8";
	PlatformOptions.ClientCredentials.ClientSecret = "xXOokYk3IXQztbURw2G2EcD+bf84hrwHOkq+aRcj+VU";
	PlatformOptions.RTCOptions = nullptr;
	PlatformInterface = EOS_Platform_Create(&PlatformOptions);

	if (PlatformInterface != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlatformInterface's been created"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error while getting Platform Interface"));
	}
}

void ULab4GameInstance::InitializeAuthInterface()
{
	AuthInterface = EOS_Platform_GetAuthInterface(PlatformInterface);
	TArray<FText> UserCredentials = m_pMainMenu->GetCredentials();
	const FString UserEmail = UserCredentials[0].ToString();
	const FString UserPassword = UserCredentials[1].ToString();

	const char* EmailStr = TCHAR_TO_ANSI(*UserEmail);
	const char* PasswordStr = TCHAR_TO_ANSI(*UserPassword);

	UE_LOG(LogTemp, Warning, TEXT("User str credentials: %hs, %hs"), EmailStr, PasswordStr);
	
	if (AuthInterface == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AuthInterface has not been initialized"));
		return;
	}
	
	EOS_Auth_LoginOptions LoginOptions;
	EOS_Auth_Credentials AuthCredentials;
	
	LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
	AuthCredentials.Id = EmailStr;
	AuthCredentials.Token = PasswordStr;
	AuthCredentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
	AuthCredentials.Type = EOS_ELoginCredentialType::EOS_LCT_Password;
	AuthCredentials.SystemAuthCredentialsOptions = nullptr;
	LoginOptions.Credentials = &AuthCredentials;

	EOS_Auth_Login(AuthInterface, &LoginOptions, nullptr, &CompletionDelegate);
}

void ULab4GameInstance::InitializeAuthInterfaceViaExchangeCode()
{
	AuthInterface = EOS_Platform_GetAuthInterface(PlatformInterface);
	FString Token;
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_PASSWORD"), Token);
	const char* TokenStr = TCHAR_TO_ANSI(*Token);
	
	if (AuthInterface == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AuthInterface has not been initialized"));
		return;
	}

	if (Token.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Authentication token is invalid"));
		return;
	}
	
	EOS_Auth_LoginOptions LoginOptions;
	EOS_Auth_Credentials AuthCredentials;
	
	LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
	AuthCredentials.Id = "";
	AuthCredentials.Token = TokenStr;
	AuthCredentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
	AuthCredentials.Type = EOS_ELoginCredentialType::EOS_LCT_ExchangeCode;
	AuthCredentials.SystemAuthCredentialsOptions = nullptr;
	LoginOptions.Credentials = &AuthCredentials;

	EOS_Auth_Login(AuthInterface, &LoginOptions, nullptr, &CompletionDelegate);
}

void ULab4GameInstance::LoginViaSDK()
{
	InitializeSDK();
	InitializePlatformInterface();
	InitializeAuthInterface();
}

void EOS_CALL ULab4GameInstance::CompletionDelegate(const EOS_Auth_LoginCallbackInfo* Data)
{
	if (Data->ResultCode == EOS_EResult::EOS_InvalidCredentials) {
		UE_LOG(LogTemp, Warning, TEXT("Authentication error"));
		GEngine->AddOnScreenDebugMessage(-1,
			3.f,
			FColor::Red,
			FString::Printf(TEXT("Incorrect email or password")),
			true,
			FVector2D(3.f)
			);
		return;
	}
	if (Data->ResultCode == EOS_EResult::EOS_Success) {
		UE_LOG(LogTemp, Warning, TEXT("User authenticated successfully"));
		GEngine->AddOnScreenDebugMessage(-1,
		3.f,
		FColor::Green,
		FString::Printf(TEXT("Logged in successfully")),
		true,
		FVector2D(3.f)
		);
		// m_pMainMenu->SetWidgetOnLoginComplete();
		// EOS_ProductUserId(Data->LocalUserId);
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Authentication error"));
	GEngine->AddOnScreenDebugMessage(-1,
		3.f,
		FColor::Red,
		FString::Printf(TEXT("Authentication error")),
		true,
		FVector2D(3.f)
		);	
}


void ULab4GameInstance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
                                        const FString& Error)
{
	
	UE_LOG(LogTemp, Warning, TEXT("Logged In: %d"), bWasSuccessful);
	
	bWasLoggedIn = bWasSuccessful;
	
	if (OnlineSubsystem == nullptr) return;

	IOnlineIdentityPtr IdentyPtr = OnlineSubsystem->GetIdentityInterface();

	if (IdentyPtr == nullptr) return;

	IdentyPtr->ClearOnLoginCompleteDelegates(0, this);

	if (bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.5f, FColor::Green, FString::Printf(TEXT("Logged In Successfuly!")));
		m_pMainMenu->SetWidgetOnLoginComplete();
	} else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.5f, FColor::Red, FString::Printf(TEXT("Connetcion Error!")));
	}
}

void ULab4GameInstance::FindSessions()
{
	if (OnlineSubsystem == nullptr) return;

	if (!bIsLanGame)
	{
		if (bWasLoggedIn == false) return;

		SessionPtr = OnlineSubsystem->GetSessionInterface();

		if (SessionPtr == nullptr) return;
	
		SearchSettings = MakeShareable(new FOnlineSessionSearch());
		SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("Test session"), EOnlineComparisonOp::Equals);
		SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnFindOnlineSessionsComplete);
		SessionPtr->FindSessions(0, SearchSettings.ToSharedRef());
	}
}

void ULab4GameInstance::OnFindOnlineSessionsComplete(const bool bWasSuccessful)
{
	const int32 LobbiesCount = SearchSettings->SearchResults.Num();
	
	UE_LOG(LogTemp, Warning, TEXT("Finding Lobbies Success: %d"), bWasSuccessful);
	
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found %d lobbies"), LobbiesCount);

		TArray<FString> ServersList;
		
		for (const FOnlineSessionSearchResult& Session : SearchSettings->SearchResults)
		{
			FString ServerName;
			Session.Session.SessionSettings.Get(ServerNameKey, ServerName);
			ServersList.Add(ServerName);
		}

		m_pMainMenu->OnInstanceFoundServers(ServersList);
		
		return;
	}

	if (OnlineSubsystem == nullptr) return;

	SessionPtr = OnlineSubsystem->GetSessionInterface();

	if (SessionPtr == nullptr) return;

	SessionPtr->ClearOnFindSessionsCompleteDelegates(this);
}

void ULab4GameInstance::OnJoinOnlineSessionComplete(const FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	
}

void ULab4GameInstance::DestroySession()
{
	if (OnlineSubsystem == nullptr) return;

	SessionPtr = OnlineSubsystem->GetSessionInterface();

	if (SessionPtr == nullptr) return;

	SessionPtr->OnDestroySessionCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnDestroySessionComplete);
	SessionPtr->DestroySession(SessionNameConst);
}

void ULab4GameInstance::ShowInGameMenu()
{
	if (InGameMenu != nullptr)
	{
		InGameMenu->TearDownInGameMenu();
		InGameMenu = nullptr;
		return;
	}
	
	if (BPInGameMenuClass == nullptr) return;
	
	InGameMenu = CreateWidget<UGameMenu>(this, BPInGameMenuClass);
	
	if (InGameMenu == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ошибка создания указателя на игровой меню"));
		return;
	}
	
	InGameMenu->SetupInGameMenu();
}

void ULab4GameInstance::ShowGameOverMenu()
{
	if (BPGameOverMenu == nullptr) return;

	GameOverWidget = CreateWidget<UGameOverMenu>(this, BPGameOverMenu);

	if (GameOverWidget == nullptr) return;

	GameOverWidget->SetupGameOverMenu();
}

void ULab4GameInstance::CheckGameState()
{
	for (int32 i = 0; i < m_Characters.Num(); ++i)
	{
		ALab4GameMode* MyGameMode = static_cast<ALab4GameMode*>(GetWorld()->GetAuthGameMode());
		if (MyGameMode == nullptr) return;
		
		if (static_cast<int32>(m_Characters[i]->GetPlayerState()->GetScore()) >= FragsToWin)
		{
			bShouldBePaused = true;
			MyGameMode->Pause(m_Characters[i]->GetPlayerName());
		}
	}
}

void ULab4GameInstance::RefreshGameState()
{
	UE_LOG(LogTemp, Warning, TEXT("Refreshing..."));
	bShouldBePaused = false;

	for (int i = 0; i < m_Characters.Num(); ++i)
	{
		m_Characters[i]->GetPlayerState()->SetScore(0.0f);
		m_Characters[i]->SetCurrentHealth(m_Characters[i]->GetPlayerMaxHealth());
	}
}

void ULab4GameInstance::ChangeHealthBarState(float Persantage)
{
	// PlayerHealthBar->SetPersantage(Persantage);
}

void ULab4GameInstance::SetIsLanGame(const bool bIsLan)
{
	bIsLanGame = bIsLan;
	IOnlineSubsystem::Get(TEXT("NULL"));
}

void ULab4GameInstance::SetIsOnlineGame(const bool bIsLan)
{
	bIsLanGame = bIsLan;
	IOnlineSubsystem::Get(TEXT("EOS"));
}

bool ULab4GameInstance::GetIsLanGame() const
{
	return bIsLanGame;
}

void ULab4GameInstance::HideGameOverMenu()
{
	GameOverWidget->TeardownGameOverMenu();
}

void ULab4GameInstance::CreateSession() const
{
	if (!SessionPtr.IsValid()) return;
	
	FOnlineSessionSettings SessionSettings;
	
	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" || bIsLanGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *IOnlineSubsystem::Get()->GetSubsystemName().ToString());
		SessionSettings.bIsLANMatch = true;
		SessionSettings.bUsesPresence = false;
		SessionSettings.bUseLobbiesIfAvailable = false;
	}
	else
	{
		SessionSettings.bIsLANMatch = false;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bUseLobbiesIfAvailable = true;
	}

	SessionSettings.bShouldAdvertise = true;
	SessionSettings.NumPublicConnections = 10;
	SessionSettings.bAllowJoinInProgress = true;
	

	FOnlineSessionSetting setting;
	setting.Data = m_ServerName.ToString();
	setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineServiceAndPing;
	
	SessionSettings.Set(ServerNameKey, setting);
	SessionSettings.Set(SEARCH_KEYWORDS, FString("Test session"), EOnlineDataAdvertisementType::ViaOnlineService);
	
	const FUniqueNetIdPtr NetID = GetFirstGamePlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
	GetFirstGamePlayer()->SetCachedUniqueNetId(NetID);
	
	SessionPtr->CreateSession(0, SessionNameConst, SessionSettings);
}

void ULab4GameInstance::LoadMainMenu() const
{
	UWorld* World = GetWorld();

	if (World == nullptr) return;

	APlayerController* PlayerController = GetFirstLocalPlayerController();

	if (PlayerController == nullptr) return;
	
	PlayerController->ClientTravel(TravelMainMenuPath, ETravelType::TRAVEL_Absolute);
}
