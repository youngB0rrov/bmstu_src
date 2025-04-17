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
#include "Lab4/Runnable/FReceiveThread.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#include "Lab4/UserWidgets/GameOverMenu.h"
#include "Lab4/UserWidgets/LobbyHostWidget.h"
#include "Lab4/UserWidgets/PlayerTableRow.h"
#include "Lab4/UserWidgets/RankedLeaderboardRow.h"
#include "Lab4/UserWidgets/MatchmakingConnectWidget.h"
#include "Lab4/UserWidgets/MatchmakingInputWidget.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "Misc/Guid.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

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
	UserCloudPtr = OnlineSubsystem->GetUserCloudInterface();
	UserStoreInterface = OnlineSubsystem->GetStoreV2Interface();
	UserPurchaseInterface = OnlineSubsystem->GetPurchaseInterface();
	LeaderboardsPtr = OnlineSubsystem->GetLeaderboardsInterface();
	StatsPtr = OnlineSubsystem->GetStatsInterface();
	IdentityPtr = OnlineSubsystem->GetIdentityInterface();
	
	if(!SessionPtr.IsValid()) return;

	SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnJoinSessionComplete);

	if (GEngine != nullptr)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &ULab4GameInstance::OnNetworkFailure);
	}

	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ULab4GameInstance::Tick), 0.1f);

	if (!UserCloudPtr.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UserCloudInterface is invalid in Init method"))
		return;
	}
	UserCloudPtr->OnWriteUserFileCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnWriteUserFileCompleteDelegate);
	UserCloudPtr->OnReadUserFileCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnReadUserFileCompleteDelegate);

	if (!CheckIfGuidExists())
	{
		GenerateGuidAndSave();
	}
}

void ULab4GameInstance::Shutdown()
{
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
	if (ConnectionSocket != nullptr)
	{
		ConnectionSocket->Close();
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
	bCanStartDedicatedMatch = false;
	int32 serverPort;
	FString serverAddress;

	GConfig->GetInt(TEXT("ServerManager"), TEXT("Port"), serverPort, GEngineIni);
	GConfig->GetString(TEXT("ServerManager"), TEXT("Address"), serverAddress, GEngineIni);
	HostSocketAddress = serverAddress;
	HostSocketPort = serverPort;

	if (IsRunningDedicatedServer())
	{
		UE_LOG(LogTemp, Log, TEXT("Application is running as dedicated server, skip finding blueprints classes..."))
		return;
	}

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

	static ConstructorHelpers::FClassFinder<UUserWidget> BPMatchmakingInputWidgetFinder(TEXT("/Game/MainMenu/WBP_MatchmakingInputWidget"));
	if (BPMatchmakingInputWidgetFinder.Succeeded())
	{
		BPMatchmakingInputWidgetClass = BPMatchmakingInputWidgetFinder.Class;
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

	if (!PendingPassword.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Private match has password to connect, setting pasword in session settings"))
		SessionSettings.Set(TEXT("SESSION_PASSWORD"), PendingPassword, EOnlineDataAdvertisementType::DontAdvertise);
	}
	
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
	PendingPassword.Empty();

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
	UE_LOG(LogTemp, Log, TEXT("Find session button clicked"))

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
	}

	if (SessionPtr != nullptr)
	{
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
	UE_LOG(LogTemp, Log, TEXT("User FUniqueNetId: %s"), *UserId.ToString())
	LoadingWidget->RemoveFromViewport();
	LoadingWidget = nullptr;
	
	bWasLoggedIn = bWasSuccessful;
	
	if (OnlineSubsystem == nullptr) return;
	
	IOnlineIdentityPtr IdentyPtr = OnlineSubsystem->GetIdentityInterface();

	if (IdentyPtr == nullptr) return;

	IdentyPtr->ClearOnLoginCompleteDelegates(0, this);

	if (bWasSuccessful)
	{
		SetIsLanGame(false);
		GEngine->AddOnScreenDebugMessage(-1, 3.5f, FColor::Green, FString::Printf(TEXT("Logged In Successfuly!")));
		m_pMainMenu->SetWidgetOnLoginComplete();
		// SetInitialPlayerDataForCloudStorage();
		RetrieveOffers(); 
		QueryUserReceipts(false);

		if (!bIsLanGame)
		{
			SetPlayerName(IdentityPtr->GetPlayerNickname(UserId));
			UE_LOG(LogTemp, Log, TEXT("Player name is %s"), *m_PlayerName)
		}
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
		SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &ULab4GameInstance::OnFindSessionsComplete);
		return;
	}
	
	//SearchSettings = MakeShareable(new FOnlineSessionSearch());
	//
	//if (SearchSettings.IsValid())
	//{
	//	SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("Test session"), EOnlineComparisonOp::Equals);
	//	SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	//	SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	//	SessionPtr->FindSessions(0,SearchSettings.ToSharedRef());
	//}
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

void ULab4GameInstance::LoginViaCredentials()
{
	InitializeSDKCredentials();
	InitializePlatformInterface();
	InitializeAuthInterfaceViaCredentials();
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

bool ULab4GameInstance::CreateSocketConnection()
{	
	// Создание клиентского сокета и подключение для отправки сообщений
	FIPv4Address outAddress;
	bool addressParseResult = FIPv4Address::Parse(HostSocketAddress, outAddress);
	if (!addressParseResult)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse FStringAddress %s to IPv4Address"), *HostSocketAddress);
		return false;
	}

	FIPv4Endpoint Endpoint(outAddress, HostSocketPort);
	ConnectionSocket = FTcpSocketBuilder("ClientSocket");
	if (ConnectionSocket->Connect(*Endpoint.ToInternetAddr()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Successfully created socket to host: [address=%s, port=%d]"), *outAddress.ToString(), HostSocketPort));
		return true;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Error, while creating socket to host: [address=%s, port=%d]"), *outAddress.ToString(), HostSocketPort));
	return false;
}

bool ULab4GameInstance::SendMessageToHostSocket(const FString& Message)
{
	TArray<uint8> payload;
	FromStringToBinaryArray(Message, payload);

	FBufferArchive ArchiveBuffer;
	int32 bytesSent = 0;

	if (!ConnectionSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while sending message to server. Client socket in nullptr"))
		return false;
	}
	ArchiveBuffer.Append(payload);
	ConnectionSocket->Send(ArchiveBuffer.GetData(), ArchiveBuffer.Num(), bytesSent);
	if (bytesSent != ArchiveBuffer.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Error while submiting data to client: sent:%d, length:%d"), bytesSent, ArchiveBuffer.Num())
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Error while submiting data to client: sent:%d, length:%d"), bytesSent, ArchiveBuffer.Num()));
		return false;
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Successfully sent data: '%s' to host"), *Message));
	
	// For spinnet widget and status
	SetFindingMatchProgress(true);
	m_pMainMenu->SetFindingMatchStatusWidgetVisibility(true);
	return true;
}

void ULab4GameInstance::FromStringToBinaryArray(const FString& Message, TArray<uint8>& OutBinaryArray)
{
	FTCHARToUTF8 Convert(*Message);
	OutBinaryArray.Empty();
	OutBinaryArray.Append((UTF8CHAR*)Convert.Get(), Convert.Length());
}

void ULab4GameInstance::InitializeReceiveSocketThread()
{
	if (!ConnectionSocket) return;
	if (ReceiveThread != nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Receive thread has already been created"))
		return;
	}
	ReceiveThread = FRunnableThread::Create(new FReceiveThread(ConnectionSocket, this), TEXT("ReceiveThread"), 0, TPri_BelowNormal);
	UE_LOG(LogTemp, Log, TEXT("Initialized receive socket thread: [ThreadId=%d, ThreadName=%s]"), ReceiveThread->GetThreadID(), *(ReceiveThread->GetThreadName()))
}

void ULab4GameInstance::DisposeReceiveSocketThread()
{
	if (!ReceiveThread) return;
	ReceiveThread->Kill(true);
	UE_LOG(LogTemp, Log, TEXT("Disposed receive socket thread: [ThreadId=%d, ThreadName=%s]"), ReceiveThread->GetThreadID(), *(ReceiveThread->GetThreadName()))
	delete ReceiveThread;
	ReceiveThread = nullptr;
}

void ULab4GameInstance::SetFindingMatchProgress(bool bIsFindingMatch)
{
	bIsFindingMatchInProgress = bIsFindingMatch;
}

void ULab4GameInstance::SetConnectAddress(const FString& ConnectAddressFromDedicatedServer)
{
	ConnectAddress = ConnectAddressFromDedicatedServer;
}

void ULab4GameInstance::SetInitialPlayerDataForCloudStorage()
{
	TSharedPtr<FJsonObject> playerData = MakeShareable(new FJsonObject);
	playerData->SetBoolField("CanCreateMatch", false);

	FString plyaerDataString;
	TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&plyaerDataString);
	FJsonSerializer::Serialize(playerData.ToSharedRef(), writer);
	UE_LOG(LogTemp, Log, TEXT("PlayerDataString after serialization: %s"), *plyaerDataString);

	FUniqueNetIdPtr userUniqueId = IdentityPtr->GetUniquePlayerId(0);
	if (!userUniqueId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid UniquePlayerId"))
		return;
	}

	FString fileName = TEXT("PlayerData.json");
	FTCHARToUTF8 Converter(*plyaerDataString);
	TArray<uint8> playerDataBytes;

	playerDataBytes.Append((uint8*)Converter.Get(), Converter.Length());
	playerDataBytes.Add(0);

	if (!UserCloudPtr.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UserCloudPtr is invalid"))
		return;
	}

	UserCloudPtr->WriteUserFile(*userUniqueId, fileName, playerDataBytes, false);
}

void ULab4GameInstance::OnWriteUserFileCompleteDelegate(bool bWasSuccessfull, const FUniqueNetId& userId, const FString& fileName)
{
	if (bWasSuccessfull)
	{
		UE_LOG(LogTemp, Log, TEXT("Player data successfully stored to file: %s"), *fileName);
		UserCloudPtr->ReadUserFile(userId, fileName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while storing player data to file: %s"), *fileName);
	}
	UserCloudPtr->ClearOnWriteUserFileCompleteDelegates(this);
}

void ULab4GameInstance::OnReadUserFileCompleteDelegate(bool bWasSuccessfull, const FUniqueNetId& userId, const FString& fileName)
{
	if (bWasSuccessfull)
	{
		FUniqueNetIdPtr userUniqueId = IdentityPtr->GetUniquePlayerId(0);
		
		if (!userUniqueId.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Error, while reading player data from file. UnserUniqueId is invalid"))
			return;
		}

		TArray<uint8> playerDataBytes;
		if (UserCloudPtr->GetFileContents(*userUniqueId, fileName, playerDataBytes))
		{
			if (playerDataBytes.Num() > 0 && playerDataBytes.Last() == 0)
			{
				playerDataBytes.Pop();
			}
			FString playerDataContentString(UTF8_TO_TCHAR(playerDataBytes.GetData()));
			playerDataContentString.TrimStartAndEndInline();
			UE_LOG(LogTemp, Log, TEXT("Got raw json string: %s"), *playerDataContentString)

			TSharedPtr<TJsonReader<>> reader = TJsonReaderFactory<>::Create(playerDataContentString);
			TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());

			if (FJsonSerializer::Deserialize(reader.ToSharedRef(), jsonObject))
			{
				bool bCanPlayerCreateMatch;
				bool bParseResult = jsonObject->TryGetBoolField("CanCreateMatch", bCanPlayerCreateMatch);

				if (!bParseResult)
				{
					UE_LOG(LogTemp, Error, TEXT("Error, while getting value from json object"))
				}
				UE_LOG(LogTemp, Log, TEXT("CanUserCreateMatch: %s"), bCanPlayerCreateMatch ? TEXT("true") : TEXT("false"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Error, while deserializing raw json string"))
			}
		}
	}
	UserCloudPtr->ClearOnReadUserFileCompleteDelegates(this);
}

void ULab4GameInstance::RetrieveOffers()
{
	FUniqueNetIdPtr userUniqueId = IdentityPtr->GetUniquePlayerId(0);
	if (!userUniqueId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while retrieving offers. UnserUniqueId is invalid"))
		return;
	}

	FOnlineStoreFilter filter;

	UserStoreInterface->QueryOffersByFilter(*userUniqueId, filter, FOnQueryOnlineStoreOffersComplete::CreateLambda([this](bool bWasSuccessfull, const TArray<FUniqueOfferId>& OfferIds, const FString& Error)
	{
		if (bWasSuccessfull)
		{
			UE_LOG(LogTemp, Log, TEXT("Offers retrieved successfully"))
			RetrieveOffersById(OfferIds);

			return;
		}
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve offers: %s"), *Error);
	}));
}

void ULab4GameInstance::RetrieveOffersById(const TArray<FUniqueOfferId>& OfferIds)
{
	FUniqueNetIdPtr userUniqueId = IdentityPtr->GetUniquePlayerId(0);
	if (!userUniqueId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while querying offers by IDs. UnserUniqueId is invalid"))
		return;
	}

	UserStoreInterface->QueryOffersById(*userUniqueId, OfferIds, FOnQueryOnlineStoreOffersComplete::CreateLambda([this](bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& Error)
	{
		FString ExcludeOfferId = FString(TEXT("03098188b46941b69160fe557939008f"));
		if (bWasSuccessful)
		{
			UE_LOG(LogTemp, Log, TEXT("Query offers by IDs succeded"))
			UserStoreInterface->GetOffers(Offers);

			for (const FOnlineStoreOfferRef& Offer : Offers)
			{
				if (Offer->OfferId == ExcludeOfferId) continue;
				UE_LOG(LogTemp, Log, TEXT("Got offer: [ID=%s, Title=%s]"), *(Offer->OfferId), *(Offer->Title.ToString()))
			}

			return;
		}
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve offers by IDs: %s"), *Error);
	}));
}

void ULab4GameInstance::StartPurchase()
{
	FUniqueNetIdPtr userUniqueId = IdentityPtr->GetUniquePlayerId(0);
	if (!userUniqueId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while purchasing item. UnserUniqueId is invalid"))
		return;
	}

	if (!UserPurchaseInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("PurchaseInterface pointer is invalid"))
		return;
	}

	FPurchaseCheckoutRequest checkoutRequest = {};
	checkoutRequest.AddPurchaseOffer(TEXT("DedicatedMatchStart"), OfferId, 1);

	UserPurchaseInterface->Checkout(*userUniqueId, checkoutRequest, FOnPurchaseCheckoutComplete::CreateLambda([this, userUniqueId](const FOnlineError& Result, const TSharedRef<FPurchaseReceipt>& Receipt)
	{
		if (Result.bSucceeded)
		{
			GetUserReceipts(userUniqueId, true);
			return;
		}

		UE_LOG(LogTemp, Error, TEXT("Failed to complete checkout: %s"), *(Result.ErrorRaw));
	}));
}

void ULab4GameInstance::QueryUserReceipts(bool bShouldFinalize)
{
	FUniqueNetIdPtr userUniqueId = IdentityPtr->GetUniquePlayerId(0);
	if (!userUniqueId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Error, while querying user entitlements. UnserUniqueId is invalid"))
		return;
	}

	if (!UserPurchaseInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("PurchaseInterface pointer is invalid"))
		return;
	}

	UserPurchaseInterface->QueryReceipts(*userUniqueId, true, FOnQueryReceiptsComplete::CreateLambda([this, userUniqueId, bShouldFinalize](const FOnlineError& Result)
	{
		if (Result.bSucceeded)
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully queried user receipts"))
			GetUserReceipts(userUniqueId, bShouldFinalize);

			return;
		}

		UE_LOG(LogTemp, Error, TEXT("Error, while querying user receipts: %s"), *(Result.ErrorRaw))
	}));
}

void ULab4GameInstance::GetUserReceipts(FUniqueNetIdPtr userUniqueId, bool bShouldFinalize)
{
	TArray<FPurchaseReceipt> userReceipts;
	UserPurchaseInterface->GetReceipts(*userUniqueId, userReceipts);

	for (const FPurchaseReceipt& userReceipt : userReceipts)
	{
		UE_LOG(LogTemp, Log, TEXT("User receipt transaction Id: %s"), *(userReceipt.TransactionId))
		if (userReceipt.TransactionState != EPurchaseTransactionState::Purchased)
		{
			UE_LOG(LogTemp, Error, TEXT("Transaction with ID: %s is not purchased yet"), *(userReceipt.TransactionId))
			continue;
		}

		for (const FPurchaseReceipt::FReceiptOfferEntry& offerEntry : userReceipt.ReceiptOffers)
		{
			UE_LOG(LogTemp, Log, TEXT("OfferId: %s, quantity: %d"), *(offerEntry.OfferId), offerEntry.Quantity)

			if (!bCanStartDedicatedMatch && offerEntry.OfferId == OfferIdAudience)
			{
				bCanStartDedicatedMatch = true;
				SavePurchaseToFile(offerEntry.OfferId);
				UE_LOG(LogTemp, Log, TEXT("OfferId %s has been found. Set bCanStartDedicatedMatch = true"), *OfferId)
			}
			if (!bShouldFinalize) continue;

			// Finalizing purchased transations
			for (const FPurchaseReceipt::FLineItemInfo& offerLineItem : offerEntry.LineItems)
			{
				UE_LOG(LogTemp, Log, TEXT("OfferLineItem name: %s"), *(offerLineItem.ItemName))
				FString InReceiptValidationInfo = offerLineItem.ValidationInfo;

				if (InReceiptValidationInfo.IsEmpty())
				{
					UE_LOG(LogTemp, Log, TEXT("OfferLineItem name has empty validation info: %s. Skip finalizing"))
					continue;
				}

				UserPurchaseInterface->FinalizeReceiptValidationInfo(*userUniqueId, InReceiptValidationInfo, FOnFinalizeReceiptValidationInfoComplete::CreateLambda([userReceipt, offerEntry](const FOnlineError& Result, const FString& ValidationInfo)
				{
					if (Result.bSucceeded)
					{
						UE_LOG(LogTemp, Log, TEXT("Successfully finilized purchase: TransactionId: %s, OfferId: %s"), *(userReceipt.TransactionId), *(offerEntry.OfferId))
						return;
					}

					UE_LOG(LogTemp, Error, TEXT("Error, while finalizing purchase: %s"), *(Result.ErrorRaw))
				}));
			}
		}
	}

	if (bCanStartDedicatedMatch)
	{
		m_pMainMenu->SetMatchmakingHintTextVisibility(false);
		m_pMainMenu->SetCreateGameHintTextVisibility(false);
	}
}

void ULab4GameInstance::GenerateGuidAndSave()
{
	FString appGuidString = FGuid::NewGuid().ToString();
	UE_LOG(LogTemp, Log, TEXT("Generated app GUID"))

	SaveBase64EncodedData(appGuidString, FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AppData"), TEXT("AppGUID.dat")));
}

void ULab4GameInstance::SaveBase64EncodedData(const FString& Data, const FString& FilePath)
{
	FString encodedData = FBase64::Encode(Data);
	FString filePath = FilePath;

	FFileHelper::SaveStringToFile(encodedData, *filePath);
	UE_LOG(LogTemp, Log, TEXT("Saved encoded GUID to directory"))
}

FString ULab4GameInstance::LoadBase64EncodedData(const FString& FilePath)
{
	FString encodedData;
	FString filePath = FilePath;

	if (FFileHelper::LoadFileToString(encodedData, *filePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully read data from file: %s"), *filePath)

		FString decodedData;
		FBase64::Decode(encodedData, decodedData);
		return decodedData;
	}

	UE_LOG(LogTemp, Error, TEXT("Data not found in file: %s"), *filePath)
	return TEXT("");
}

void ULab4GameInstance::ShowMatchmakingInputWidget(bool bIsVisible = true)
{
	if (!bIsVisible && MatchmakingInputWidget != nullptr)
	{
		MatchmakingInputWidget->RemoveFromViewport();
		MatchmakingInputWidget = nullptr;
		
		return;
	}
	if (MatchmakingInputWidget == nullptr && BPMatchmakingInputWidgetClass)
	{
		MatchmakingInputWidget = CreateWidget<UMatchmakingInputWidget>(this, BPMatchmakingInputWidgetClass);
		if (MatchmakingInputWidget)
		{
			MatchmakingInputWidget->AddToViewport();
		}
	}
}

bool ULab4GameInstance::CheckIfGuidExists()
{
	FString filePath = FPaths::ProjectSavedDir() + TEXT("AppData/AppGUID.dat");

	return FPlatformFileManager::Get().GetPlatformFile().FileExists(*filePath);
}

void ULab4GameInstance::SavePurchaseToFile(const FString& PurchaseOfferId)
{
	if (!CheckIfGuidExists())
	{
		UE_LOG(LogTemp, Error, TEXT("App GUID not found"))
		return;
	}

	FString playerLogin = GetPlayerName();
	FString offerIdHash = GetSHA256Hash(PurchaseOfferId);
	FString appGuid = LoadBase64EncodedData(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AppData"), TEXT("AppGUID.dat")));
	FString fileName = offerIdHash + TEXT("_") + playerLogin + TEXT(".dat");
	FString filePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Purchases"), fileName);

	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*filePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Offer with ID: %s already exists, hash: %s. Cancel saving data..."), *PurchaseOfferId, *offerIdHash);
		return;
	}

	FString fileContent = FString::Printf(TEXT("AppGUID=%s\nOfferId=%s"), *appGuid, *PurchaseOfferId);
	SaveBase64EncodedData(fileContent, filePath);
	UE_LOG(LogTemp, Log, TEXT("Saved information about offer: [OfferId=%s, UserLogin=%s]]"), *PurchaseOfferId, *playerLogin);
}

bool ULab4GameInstance::CheckIfOfferAcquired()
{
	FString directoryPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Purchases"));
	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString appGuid = LoadBase64EncodedData(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AppData"), TEXT("AppGUID.dat")));
	FString playerLogin = GetPlayerName();
	TArray<FString> files;

	if (!platformFile.DirectoryExists(*directoryPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Purchases directory does not exist. Access forbidden"))
		return false;
	}

	platformFile.FindFiles(files, *directoryPath, TEXT(""));
	if (files.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Purchases directory does not exist. Access forbidden"))
		return false;
	}

	for (const FString& filePath : files)
	{
		FString fileContent = LoadBase64EncodedData(filePath);
		if (fileContent.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("fileContent is empty for filePath: %s"), *filePath);
			continue;
		}

		FString fileAppGuid, fileOfferId, filePlayerName;
		FParse::Value(*fileContent, TEXT("AppGUID="), fileAppGuid);
		FParse::Value(*fileContent, TEXT("OfferId"), fileOfferId);

		if (appGuid != fileAppGuid)
		{
			UE_LOG(LogTemp, Warning, TEXT("FileAppGuid: %s and apdGuid: %s are not equal. Access to offer is forbidden"))
			continue;
		}

		FString fileName = FPaths::GetBaseFilename(filePath); // Получаем только имя файла без расширения
		FString hashPart;
		FString userLogin;
		if (fileName.Split(TEXT("_"), &hashPart, &userLogin))
		{
			if (userLogin != playerLogin)
			{
				UE_LOG(LogTemp, Warning, TEXT("Player login %s not found in fileName. Access to offer is forbidden"), *playerLogin)
				continue;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to split filename: %s"), *fileName);
			continue;
		}

		fileOfferId = fileOfferId.Replace(TEXT("\n"), TEXT("")).Replace(TEXT("="), TEXT("")).TrimStartAndEnd();
		if (fileOfferId == OfferIdAudience)
		{
			UE_LOG(LogTemp, Log, TEXT("Found valid purchase for starting dedicated matches"))
			bCanStartDedicatedMatch = true;
			return true;
		}
	}
	return false;
}

void ULab4GameInstance::SetMatchmakingHintTextVisibility(bool bIsVisible)
{
	m_pMainMenu->SetMatchmakingHintTextVisibility(bIsVisible);
}

void ULab4GameInstance::SetCreateGameHintTextVisibility(bool bIsVisible)
{
	m_pMainMenu->SetCreateGameHintTextVisibility(bIsVisible);
}

void ULab4GameInstance::GiveAccessToCreateMatchSection()
{
	m_pMainMenu->GiveAccessToCreateMatchSection();
}

FString ULab4GameInstance::GetSHA256Hash(const FString& Data)
{
	FSHAHash hash;
	FSHA1::HashBuffer(TCHAR_TO_UTF8(*Data), Data.Len(), hash.Hash);
	return hash.ToString();
}
