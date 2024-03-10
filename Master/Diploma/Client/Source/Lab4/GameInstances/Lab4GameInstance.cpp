#include "Lab4GameInstance.h"

#include <eos_auth.h>
#include <eos_leaderboards.h>
#include <eos_sessions.h>
#include <eos_stats.h>
#include <string>

#include "EOSSettings.h"
#include "eos_init.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Lab4/Lab4Character.h"
#include "Lab4/Actors/MainMenuInitializer.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerState.h"
#include "WebSocketsModule.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Lab4/UserWidgets/GameOverMenu.h"
#include "Lab4/UserWidgets/LobbyHostWidget.h"
#include "Lab4/UserWidgets/PlayerTableRow.h"
#include "Lab4/UserWidgets/RankedLeaderboardRow.h"

#define EOS_ID_SEPARATOR TEXT("|")
#define WITH_SDK 0
#define USE_EMPTY_LOBBY 1

EOS_EpicAccountId ULab4GameInstance::LoggedInUserID = nullptr;
EOS_ProductUserId ULab4GameInstance::Eos_ProductUserId = nullptr;
EOS_HAuth ULab4GameInstance::AuthInterface = nullptr;
EOS_HConnect ULab4GameInstance::ConnectHandle = nullptr;
const char* ULab4GameInstance::CurrentSessionId = "";
const FString ULab4GameInstance::TravelGamePath = TEXT("/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap?listen");
AMainMenuInitializer* ULab4GameInstance::m_pMainMenu = m_pMainMenu != nullptr ? m_pMainMenu : nullptr;
constexpr const char* BucketId = "SessionSample:Region";

void ULab4GameInstance::Init()
{
	Super::Init();

	bIsLanGame = false;

	#if	USE_EMPTY_LOBBY == 1
		TravelLobbyPath = TEXT("/Game/Maps/LobbyMap?listen");
	#else
		TravelLobbyPath = TEXT("/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap?listen");
	#endif
	
	// Получить адрес подсистемы
	OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem == nullptr) return;

	// Получить адрес API сессий
	SessionPtr = OnlineSubsystem->GetSessionInterface();
	LeaderboardsPtr = OnlineSubsystem->GetLeaderboardsInterface();
	StatsPtr = OnlineSubsystem->GetStatsInterface();
	IdentityPtr = OnlineSubsystem->GetIdentityInterface();
	
	if(!SessionPtr.IsValid()) return;
	
	SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnFindSessionsComplete);
	SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnJoinSessionComplete);

	if (GEngine != nullptr)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &ULab4GameInstance::OnNetworkFailure);
	}

	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ULab4GameInstance::Tick), 0.1f);
}

void ULab4GameInstance::Shutdown()
{
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
	if (WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
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

void ULab4GameInstance::InitWebSocketConnection()
{
	// Создание веб-сокета
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	WebSocket = FWebSocketsModule::Get().CreateWebSocket("ws://localhost:8080");

	UE_LOG(LogTemp, Warning, TEXT("WebSocket connected: %d"), WebSocket->IsConnected())
		WebSocket->Connect();
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

	static ConstructorHelpers::FClassFinder<UUserWidget> BPRankedLeaderboardRow(TEXT("/Game/MainMenu/WBP_RankedPlayerRow"));

	if (BPRankedLeaderboardRow.Succeeded())
	{
		BPRankedLeaderboardRowClass = BPRankedLeaderboardRow.Class;
	}
	
	static ConstructorHelpers::FClassFinder<UUserWidget> BPWinnerWidgetClassFinder(TEXT("/Game/MainMenu/WBP_WinnerWidget"));

	if (BPWinnerWidgetClassFinder.Succeeded())
	{
		BPWinnerWidgetClass = BPWinnerWidgetClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> BPLobbyHostClass(TEXT("/Game/GameUI/WBP_HostStartButton"));
	if (BPLobbyHostClass.Succeeded())
	{
		BPLobbyHostButtonClass = BPLobbyHostClass.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> BPLoadingWidgetClassFinder(TEXT("/Game/MainMenu/WBP_LoadWidget"));
	if (BPLoadingWidgetClassFinder.Succeeded())
	{
		BPLoadingWidgetClass = BPLoadingWidgetClassFinder.Class;
	}
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

void ULab4GameInstance::Host()
{
#if WITH_SDK != 1
	if (!SessionPtr.IsValid()) return;
	
	const FNamedOnlineSession *pOnlineSession = SessionPtr->GetNamedSession(SessionNameConst);
	
	if (pOnlineSession != nullptr)
	{
		SessionPtr->DestroySession(SessionNameConst);
	}
	else
	{
		CreateSession();
	}
#else
	CreateSessionViaSDK();
#endif
}

void ULab4GameInstance::CreateSession()
{
	if (!SessionPtr.IsValid()) return;
	
	FOnlineSessionSettings SessionSettings;
	
	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" || bIsLanGame)
	{
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
	SessionSettings.bUseLobbiesVoiceChatIfAvailable = true;

	FOnlineSessionSetting setting;
	setting.Data = m_ServerName.ToString();
	setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineServiceAndPing;
	
	SessionSettings.Set(ServerNameKey, setting);
	SessionSettings.Set(SEARCH_KEYWORDS, FString("Test session"), EOnlineDataAdvertisementType::ViaOnlineService);
	
	const FUniqueNetIdPtr NetID = GetFirstGamePlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
	GetFirstGamePlayer()->SetCachedUniqueNetId(NetID);
	
	SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnCreateSessionComplete);
	SessionPtr->CreateSession(0, SessionNameConst, SessionSettings);
}

void ULab4GameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	UE_LOG(LogTemp, Warning, TEXT("Session creation status: %d"), Success);
	if (!Success) return;
	
	if (m_pMainMenu != nullptr)
	{
		m_pMainMenu->TeardownAll();
	}

	GetWorld()->ServerTravel(TravelLobbyPath);
	SessionPtr->ClearOnCreateSessionCompleteDelegates(this);
}

void ULab4GameInstance::DestroySession()
{
	if (OnlineSubsystem == nullptr) return;

	SessionPtr = OnlineSubsystem->GetSessionInterface();
	if (SessionPtr == nullptr) return;

	SessionPtr->OnDestroySessionCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnDestroySessionComplete);
	SessionPtr->DestroySession(SessionNameConst);
}

void ULab4GameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	UE_LOG(LogTemp, Warning, TEXT("Destroying Session Success %d"), Success);
	
	if (!Success) return;
	
	if (OnlineSubsystem == nullptr) return;

	SessionPtr = OnlineSubsystem->GetSessionInterface();
	SessionPtr->ClearOnDestroySessionCompleteDelegates(this);
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

		// Для поиска лобби по ключу на devportal
		SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("Test session"), EOnlineComparisonOp::Equals);
		SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnFindOnlineSessionsComplete);
		SessionPtr->FindSessions(0, SearchSettings.ToSharedRef());
	}
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

		SessionPtr->ClearOnFindSessionsCompleteDelegates(this);
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

void ULab4GameInstance::Join()
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
}

void ULab4GameInstance::OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Result)
{
	FString Address;

	if (Result == EOnJoinSessionCompleteResult::Type::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Join Session status: %d"), Result);
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while joining online session"));
	}
	
	if (OnlineSubsystem == nullptr) return;
	SessionPtr = OnlineSubsystem->GetSessionInterface();

	if (SessionPtr == nullptr) return;
	SessionPtr->GetResolvedConnectString(Name, Address);

	if (Address.IsEmpty()) return;
	
	if (APlayerController *PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
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

	if (LoadingWidget == nullptr && BPLoadingWidgetClass)
	{
		LoadingWidget = CreateWidget(this, BPLoadingWidgetClass);
		LoadingWidget->AddToViewport();
	}
}

void ULab4GameInstance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	UE_LOG(LogTemp, Warning, TEXT("Logged In: %d"), bWasSuccessful);
	LoadingWidget->RemoveFromViewport();
	LoadingWidget = nullptr;
	
	bWasLoggedIn = bWasSuccessful;
	
	if (OnlineSubsystem == nullptr) return;
	
	IOnlineIdentityPtr IdentyPtr = OnlineSubsystem->GetIdentityInterface();

	if (IdentyPtr == nullptr) return;

	IdentyPtr->ClearOnLoginCompleteDelegates(0, this);

	if (bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.5f, FColor::Green, FString::Printf(TEXT("Logged In Successfuly!")));
		m_pMainMenu->SetWidgetOnLoginComplete();
		EOSVoiceChatLogin();
	} else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.5f, FColor::Red, FString::Printf(TEXT("Connetcion Error!")));
	}
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
		SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
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

void ULab4GameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("Ошибка запуска сессии/поключения к сессии"))
	LoadMainMenu();
}

void ULab4GameInstance::AddLobbyHostWidget()
{
	APlayerController* LobbyHostPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	
	if (LobbyHostPlayerController)
	{
		LobbyHostWidget = CreateWidget<ULobbyHostWidget>(this, BPLobbyHostButtonClass);

		if (LobbyHostWidget)
		{
			LobbyHostWidget->AddToViewport();
		}
	}
}

void ULab4GameInstance::EOSVoiceChatLogin()
{
	OnlineSubsystem = OnlineSubsystem == nullptr ? IOnlineSubsystem::Get() : OnlineSubsystem;

	if (OnlineSubsystem)
	{
		IdentityPtr = IdentityPtr == nullptr ? OnlineSubsystem->GetIdentityInterface() : IdentityPtr;

		if (IdentityPtr)
		{
			if (bWasLoggedIn)
			{
				IVoiceChat* VoiceChatRef = IVoiceChat::Get();
				if (VoiceChatRef)
				{
					Lab4VoiceChatUser = VoiceChatRef->CreateUser();

					if (Lab4VoiceChatUser)
					{
						TSharedPtr<const FUniqueNetId> NetId = IdentityPtr->GetUniquePlayerId(0);
						FPlatformUserId PlatformUserId = IdentityPtr->GetPlatformUserIdFromUniqueNetId(*NetId);
						Lab4VoiceChatUser->Login(PlatformUserId, NetId->ToString(), TEXT(""), FOnVoiceChatLoginCompleteDelegate::CreateUObject(this, &ULab4GameInstance::OnVoiceLoginComplete));
					}
				}
			}
		}
	}
}

void ULab4GameInstance::OnVoiceLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result)
{
	if (Result == EVoiceChatResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("User was connected to voice chat successfully"));
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Error, while conntecting user to the voice chat"));
}

void ULab4GameInstance::InitializeSDKCredentials()
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
		InitializeStatsHandler();
		InitializeLeaderboardsHandler();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error while getting Platform Interface"));
	}
}

void ULab4GameInstance::InitializeAuthInterfaceViaCredentials()
{
	AuthInterface =  AuthInterface == nullptr ? EOS_Platform_GetAuthInterface(PlatformInterface) : AuthInterface;
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

	//EOS_Auth_Login(AuthInterface, &LoginOptions, nullptr, &CompletionDelegate);
}

void ULab4GameInstance::InitializeAuthInterfaceViaExchangeCode()
{
	AuthInterface =  AuthInterface == nullptr ? EOS_Platform_GetAuthInterface(PlatformInterface) : AuthInterface;
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
}

void ULab4GameInstance::InitializeAuthInterfaceViaAccountPortal()
{
	AuthInterface =  AuthInterface == nullptr ? EOS_Platform_GetAuthInterface(PlatformInterface) : AuthInterface;
	
	if (AuthInterface == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AuthInterface has not been initialized"));
		return;
	}
	
	EOS_Auth_LoginOptions LoginOptions;
	EOS_Auth_Credentials AuthCredentials;
	
	LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
	AuthCredentials.Id = "";
	AuthCredentials.Token = "";
	AuthCredentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
	AuthCredentials.Type = EOS_ELoginCredentialType::EOS_LCT_AccountPortal;
	AuthCredentials.SystemAuthCredentialsOptions = nullptr;
	LoginOptions.Credentials = &AuthCredentials;
}

void ULab4GameInstance::InitializeStatsHandler()
{
	if (PlatformInterface == nullptr)
		return;
	
	StatsInterfaceHandle = EOS_Platform_GetStatsInterface(PlatformInterface);
	UE_LOG(LogTemp, Warning, TEXT("Stats Interface has been created"));
}

void ULab4GameInstance::InitializeLeaderboardsHandler()
{
	if (PlatformInterface == nullptr)
		return;

	LeaderboardsHandle = EOS_Platform_GetLeaderboardsInterface(PlatformInterface);

	if (LeaderboardsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while creaating LeaderboardsHandle:"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Leaderboards Interface has been created"));
}

void ULab4GameInstance::InitializeConnectHandler()
{
	if (PlatformInterface == nullptr)
	{
		return;
	}

	ConnectHandle = EOS_Platform_GetConnectInterface(PlatformInterface);

	if (ConnectHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while creaating ConnectHandle:"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Connect Interface has been created"));
}

void ULab4GameInstance::InitializeSessionsHandler()
{
	if (PlatformInterface == nullptr)
	{
		return;
	}

	if ((SessionsHandle = EOS_Platform_GetSessionsInterface(PlatformInterface)) == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Eror, while initializing sessions handler"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Sessions interface has been creaated successfully"));
}


void ULab4GameInstance::QueryGlobalRanks(const int32 LeftBoundry, const int32 RightBoundry)
{
	LeftScoreBoundary = LeftBoundry;
	RightScoreBoundary = RightBoundry;
	UE_LOG(LogTemp, Warning, TEXT("Left boudry: %d, right boundry: %d"), LeftScoreBoundary, RightScoreBoundary);
	LeaderboardsPtr = LeaderboardsPtr == nullptr ? OnlineSubsystem->GetLeaderboardsInterface() : LeaderboardsPtr;

	if (!LeaderboardsPtr.IsValid())
	{
		return;
	}

	FOnlineLeaderboardReadRef ReadRef = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
	ReadRef->LeaderboardName = RankedLeaderboardName;
	ReadRef->ColumnMetadata.Add(FColumnMetaData(FName(TEXT("PlayerFragsHighScore")), EOnlineKeyValuePairDataType::Int32));
	ReadRef->SortedColumn = FName(TEXT("Score"));
	QueryGlobalRanksDelegateHandle = LeaderboardsPtr->AddOnLeaderboardReadCompleteDelegate_Handle(
			FOnLeaderboardReadComplete::FDelegate::CreateUObject(
				this,
				&ULab4GameInstance::HandleQueryGlobalRanksResult,
				ReadRef
				)
		);
	
	if (!LeaderboardsPtr->ReadLeaderboardsAroundRank(
		0,
		100,
		ReadRef
		))
	{
		LeaderboardsPtr->ClearOnLeaderboardReadCompleteDelegate_Handle(QueryGlobalRanksDelegateHandle);
		QueryGlobalRanksDelegateHandle.Reset();
	}
}

void ULab4GameInstance::IngestMatchData()
{
	IOnlineIdentityPtr Identity = IdentityPtr == nullptr ? OnlineSubsystem->GetIdentityInterface() : IdentityPtr;
	IOnlineStatsPtr Stats = StatsPtr == nullptr ? OnlineSubsystem->GetStatsInterface() : StatsPtr;

	FOnlineStatsUserUpdatedStats Stat = FOnlineStatsUserUpdatedStats(Identity->GetUniquePlayerId(0).ToSharedRef());

	APlayerController* CurrentPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
}

void ULab4GameInstance::CompletionDelegateIngestPlayerData(const EOS_Stats_IngestStatCompleteCallbackInfo* Data)
{
	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("User data was submitted successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while submitting user data %hs"), EOS_EResult_ToString(Data->ResultCode));
	}
}

void ULab4GameInstance::CompletionDelegateConnect(const EOS_Connect_LoginCallbackInfo* Data)
{
	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("User connected successfully"));
		Eos_ProductUserId = Data->LocalUserId;

		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Error, while connecting user: %hs"), EOS_EResult_ToString(Data->ResultCode));
}

void ULab4GameInstance::CompletionDelegateSessionCreate(const EOS_Sessions_UpdateSessionCallbackInfo* Data)
{
	UWorld* World = static_cast<UWorld*>(Data->ClientData);
	
	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session %hs with session id: %hs was created and updated successfully"), Data->SessionName, Data->SessionId);

		if (World != nullptr)
		{
			World->ServerTravel(TravelGamePath);
		}
		
		if (m_pMainMenu != nullptr)
		{
			m_pMainMenu->TeardownAll();
		}
		
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Error, while creating and updating session: %hs"), EOS_EResult_ToString(Data->ResultCode));
}

void ULab4GameInstance::CompletionDelegateSessionDestroy(const EOS_Sessions_DestroySessionCallbackInfo* Data)
{
	const FString* DestroyedSessionName = static_cast<FString*>(Data->ClientData);
	
	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session %s was destroyed successfully"), **DestroyedSessionName);
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Error, while destroying session %s: %hs"), **DestroyedSessionName, EOS_EResult_ToString(Data->ResultCode));
}

void ULab4GameInstance::HandleQueryGlobalRanksResult(const bool bWasSuccessful,
	FOnlineLeaderboardReadRef LeaderboardRef)
{
	LeaderboardsPtr = LeaderboardsPtr == nullptr ? OnlineSubsystem->GetLeaderboardsInterface() : LeaderboardsPtr;
	LeaderboardsPtr->ClearOnLeaderboardReadCompleteDelegate_Handle(QueryGlobalRanksDelegateHandle);
	QueryGlobalRanksDelegateHandle.Reset();
	uint32 LocalRank = 0;
	UE_LOG(LogTemp, Warning, TEXT("QueryGlobalRanksResult: %d"), bWasSuccessful);
	
	if (!bWasSuccessful)
	{
		UE_LOG_ONLINE(Warning, TEXT("Error, while querying glogal ranks leaderboard"));
		return;
	}

	m_pMainMenu->ClearRankedLeaderboardList();
	
	for (auto Row: LeaderboardRef->Rows)
	{
		UE_LOG(LogTemp, Warning, TEXT("Get global leaderboard user entity: PlayerId: %s, Player Nickname: %s, PlayerRank: %d"), *Row.PlayerId->ToString(), *Row.NickName, Row.Rank);
		int32 Score;
		Row.Columns[TEXT("Score")].GetValue(Score);

		if (!(Score > LeftScoreBoundary && Score <= RightScoreBoundary))
		{
			continue;
		}
		
		if (BPRankedLeaderboardRowClass == nullptr)
		{
			return;
		}
		
		URankedLeaderboardRow* RankedLeaderboardRow = CreateWidget<URankedLeaderboardRow>(this, BPRankedLeaderboardRowClass);

		if (RankedLeaderboardRow)
		{
			RankedLeaderboardRow->SetRankedRowText(++LocalRank, Row.NickName, Score, Row.Rank);
			m_pMainMenu->AddRankedLeaderboardRow(RankedLeaderboardRow);
		}
	}
}

void ULab4GameInstance::ConnectViaSDK()
{
	if (AuthInterface == nullptr)
	{
		return;
	}

	EOS_Auth_Token* UserAuthToken;
	EOS_Auth_CopyUserAuthTokenOptions CopyTokenOptions = {0};
	CopyTokenOptions.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;

	if (EOS_Auth_CopyUserAuthToken(AuthInterface, &CopyTokenOptions, LoggedInUserID, &UserAuthToken) == EOS_EResult::EOS_Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Getting user access token"));
		
		EOS_Connect_Credentials ConnectCredentials;
		ConnectCredentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
		ConnectCredentials.Token = UserAuthToken->AccessToken;
		ConnectCredentials.Type = EOS_EExternalCredentialType::EOS_ECT_EPIC;
	
		EOS_Connect_LoginOptions ConnectLoginOptions = {0};
		ConnectLoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
		ConnectLoginOptions.Credentials = &ConnectCredentials;
		ConnectLoginOptions.UserLoginInfo = nullptr;
	
		if (ConnectHandle == nullptr)
		{
			return;
		}
	
		EOS_Connect_Login(ConnectHandle, &ConnectLoginOptions, nullptr, &CompletionDelegateConnect);
		EOS_Auth_Token_Release(UserAuthToken);
	}
}


void ULab4GameInstance::LoginViaCredentials()
{
	InitializeSDKCredentials();
	InitializePlatformInterface();
	InitializeAuthInterfaceViaCredentials();
}

void ULab4GameInstance::GetUserVoiceChatInterface()
{
	if (OnlineSubsystem == nullptr) return;
	
	IdentityPtr = IdentityPtr == nullptr ? OnlineSubsystem->GetIdentityInterface() : IdentityPtr;
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

void ULab4GameInstance::LoadMainMenu() const
{
	UWorld* World = GetWorld();

	if (World == nullptr) return;

	APlayerController* PlayerController = GetFirstLocalPlayerController();

	if (PlayerController == nullptr) return;
	
	PlayerController->ClientTravel(TravelMainMenuPath, ETravelType::TRAVEL_Absolute);
}