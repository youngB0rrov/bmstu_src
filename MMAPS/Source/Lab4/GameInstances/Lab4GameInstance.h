#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Lab4/UserWidgets/PlayerHealthBar.h"
#include "eos_types.h"
#include "eos_auth_types.h"
#include "eos_common.h"
#include "eos_connect_types.h"
#include "eos_sessions_types.h"
#include "eos_sdk.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Lab4GameInstance.generated.h"

UCLASS(Config=Engine)
class LAB4_API ULab4GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(Config)
	FString DefaultPlatformService;

	UPROPERTY(Config)
	FString bEnabled;
	
	virtual void Init() override;
	virtual void Shutdown() override;
	bool Tick(float DeltaSeconds);
	
	void ConnectMainMenuInitializer(class AMainMenuInitializer* const pInitializer);

	FString GetPlayerName();
	ULab4GameInstance();
	void SetPlayerName(const FString& Name);
	void SetServerName(const FString& SessionName);
	void SetJoinIndex(const uint32 Index);

	void AddCharacter(class ALab4Character* const Character, const FString& Name);
	void RemoveCharacter(class ALab4Character* const Character);
	
	void Host() const;
	void Join() const;

	void RefreshServersList();
	
	bool ChangeConfigToOnline();
	bool ChangeConfigToLan();
	
	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Result);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	
	void LogIn();
	void InitializeSDKCredentials();
	void LoginViaSDKAccountPortal();
	void InitializePlatformInterface();
	void InitializeAuthInterfaceViaCredentials();
	void InitializeAuthInterfaceViaExchangeCode();
	void InitializeAuthInterfaceViaAccountPortal();
	void InitializeStatsHandler();
	void InitializeLeaderboardsHandler();
	void InitializeConnectHandler();
	void InitializeSessionsHandler();
	
	UFUNCTION(Exec)
	void SubmitPlayerScores() const;
	
	UFUNCTION(Exec)
	void QueryRanks() const;

	UFUNCTION(Exec)
	void QueryGlobalRanks();

	UFUNCTION(Exec)
	void IngestMatchData();
	void LoginViaCredentials();
	static void ConnectViaSDK();
	void CreateSessionViaSDK() const;

	UFUNCTION(Exec)
	void DestroySessionViaSDK();
	
	static void EOS_CALL CompletionDelegate(const EOS_Auth_LoginCallbackInfo* Data);
	static void EOS_CALL CompletionDelegateLeaderboards(const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo* Data);
	static void EOS_CALL CompletionDelegateIngestPlayerData(const EOS_Stats_IngestStatCompleteCallbackInfo* Data);
	static void EOS_CALL CompletionDelegateConnect(const EOS_Connect_LoginCallbackInfo* Data);
	static void EOS_CALL CompletionDelegateSessionCreate(const EOS_Sessions_UpdateSessionCallbackInfo* Data);
	static void EOS_CALL CompletionDelegateSessionDestroy(const EOS_Sessions_DestroySessionCallbackInfo* Data);
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Error);
	void HandleQueryGlobalRanksResult(const bool bWasSuccessful, FOnlineLeaderboardReadRef LeaderboardRef);

	UFUNCTION(Exec)
	void FindSessions();
	void OnFindOnlineSessionsComplete(const bool bWasSuccessful);

	void DestroySession();
	
	FString GetPlayerName() const { return m_PlayerName; }
	
	UFUNCTION(Exec)
	void ShowInGameMenu();
	
	UFUNCTION(Exec)
	void ShowGameOverMenu();

	
	void SetIsLanGame(const bool bIsLan);
	void SetIsOnlineGame(const bool bIsLan);
	bool GetIsLanGame() const;
	TArray<class ALab4Character*> GetAllCharacters() const { return m_Characters; }
	bool GetShouldBePause() const { return bShouldBePaused;}

	UFUNCTION(Exec)
	void HideGameOverMenu();

	static char const* EpicAccountIDToString(EOS_EpicAccountId InAccountId);
	static char const* ProductUserIDToString(EOS_ProductUserId InAccountId);
	static EOS_EpicAccountId EpicAccountIdFromString(const char* AccountString);
	static EOS_ProductUserId ProductUserIdFromString(const char* AccountString);
	
private:
	void CreateSession() const;
	void LoadMainMenu() const;
	bool bShouldBePaused;
	
	static AMainMenuInitializer *m_pMainMenu;
	
	UPROPERTY()
	UPlayerHealthBar* PlayerHealthBar;

	UPROPERTY()
	class UGameOverMenu* GameOverWidget;

	UPROPERTY()
	class UGameMenu* InGameMenu;
	
	EOS_HPlatform PlatformInterface;
	static EOS_HAuth AuthInterface;
	EOS_ContinuanceToken ContinuanceToken;
	EOS_HStats StatsInterfaceHandle;
	EOS_HLeaderboards LeaderboardsHandle;
	EOS_HSessions SessionsHandle;
	static EOS_HConnect ConnectHandle;
	static EOS_EpicAccountId LoggedInUserID;
	static EOS_ProductUserId Eos_ProductUserId;
	static const char* CurrentSessionId;
	FDelegateHandle QueryGlobalRanksDelegateHandle;
	
	bool bWasLoggedIn;
	bool bIsLanGame;
	
	FDelegateHandle TickDelegateHandle;
	TSharedPtr<FOnlineSessionSearch> SearchSettings;
	IOnlineSubsystem *OnlineSubsystem;
	IOnlineSessionPtr SessionPtr;
	IOnlineLeaderboardsPtr LeaderboardsPtr;
	IOnlineIdentityPtr IdentityPtr;
	IOnlineStatsPtr StatsPtr;
	TSharedPtr<FOnlineSessionSearch> m_pSessionSearch;

	FString m_PlayerName;
	FName m_ServerName;
	uint32 m_JoinIndex;

	TArray<FString> m_PlayerNames;
	TArray<class ALab4Character*> m_Characters;
	
	TSubclassOf<UUserWidget> m_ConnectionFailureWidget;
	TSubclassOf<UUserWidget> BPInGameMenuClass;
	TSubclassOf<UUserWidget> BPGameOverMenu;
	TSubclassOf<UUserWidget> BPHealthBarClass;
	TSubclassOf<UUserWidget> BPWinnerWidgetClass;
	TSubclassOf<UUserWidget> BPRankedLeaderboardRowClass;
	
	const FName ServerNameKey = "ServerName";
	const FName SessionNameConst = "Session";
	const FName RankedLeaderboardName = TEXT("PlayersFragsLeaderboard");
	const FName RankedStatName = TEXT("PlayerFragsHighScore");
	static const FString TravelGamePath;
	const FString TravelMainMenuPath = TEXT("/Game/MainMenu/MainMenuMap");
	const uint32 ScoreCoefficient = 25;
};
