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
#include "VoiceChat.h"
#include "Networking.h"
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

	UPROPERTY()
	class UGameMenu* InGameMenu;

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
	
	void Host();
	void Join();

	void RefreshServersList();
	
	bool ChangeConfigToOnline();
	bool ChangeConfigToLan();
	
	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Result);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);\
	void AddLobbyHostWidget();
	
	UPROPERTY()
	class ULobbyHostWidget* LobbyHostWidget;
	
	void LogIn();
	void InitializeSDKCredentials();
	void InitializePlatformInterface();
	void InitializeAuthInterfaceViaCredentials();

	UFUNCTION(Exec)
	void QueryGlobalRanks(const int32 LeftBoundry, const int32 RightBoundry);

	UFUNCTION(Exec)
	void IngestMatchData();
	void LoginViaCredentials();

	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Error);
	void HandleQueryGlobalRanksResult(const bool bWasSuccessful, FOnlineLeaderboardReadRef LeaderboardRef);

	UFUNCTION(Exec)
	void FindSessions();
	void OnFindOnlineSessionsComplete(const bool bWasSuccessful);

	void DestroySession();
	
	FString GetPlayerName() const { return m_PlayerName; }
	FORCEINLINE bool GetLoginStatus() const { return bWasLoggedIn;}
	
	UFUNCTION(Exec)
	void ShowInGameMenu();
	
	UFUNCTION(Exec)
	void ShowGameOverMenu();

	void SetIsLanGame(const bool bIsLan);
	void SetIsOnlineGame(const bool bIsLan);
	bool GetIsLanGame() const;
	FORCEINLINE FName GetSessionName() const { return SessionNameConst; }

	UFUNCTION(Exec)
	void HideGameOverMenu();

	TArray<class ALab4Character*> GetAllCharacters() const { return m_Characters; }
	bool GetShouldBePause() const { return bShouldBePaused;}
	bool CreateSocketConnection();
	bool SendMessageToHostSocket(const FString& Message);
	void SetFindingMatchProgress(bool bIsFindingMatch);
	FORCEINLINE bool GetIsFindingMatchInProgress() const { return bIsFindingMatchInProgress; }
	void InitializeReceiveSocketThread();
	void DisposeReceiveSocketThread();

private:
	UFUNCTION()
	void CreateSession();
	void LoadMainMenu() const;
	void FromStringToBinaryArray(const FString& Message, TArray<uint8>& OutBinaryArray);
	bool bShouldBePaused;
	
	static AMainMenuInitializer *m_pMainMenu;
	
	UPROPERTY()
	UPlayerHealthBar* PlayerHealthBar;

	UPROPERTY()
	class UGameOverMenu* GameOverWidget;
	
	UPROPERTY()
	UUserWidget* LoadingWidget;
	
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
	IVoiceChatUser* Lab4VoiceChatUser;

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
	TSubclassOf<UUserWidget> BPLobbyHostButtonClass;
	TSubclassOf<UUserWidget> BPLoadingWidgetClass;
	
	const FName ServerNameKey = "ServerName";
	const FName SessionNameConst = "Session";
	const FName RankedLeaderboardName = TEXT("PlayersFragsLeaderboard");
	const FName RankedStatName = TEXT("PlayerFragsHighScore");
	static const FString TravelGamePath;
	FString TravelLobbyPath;
	const FString TravelMainMenuPath = TEXT("/Game/MainMenu/MainMenuMap");
	const uint32 ScoreCoefficient = 25;
	int32 LeftScoreBoundary, RightScoreBoundary; 
	//const FString HostSocketAddress = TEXT("127.0.0.1");
	const FString HostSocketAddress = TEXT("127.0.0.1");
	int32 HostSocketPort = 8870;
	FIPv4Address HostIp;
	FSocket* ConnectionSocket;
	bool bIsFindingMatchInProgress = false;
	FRunnableThread* ReceiveThread = nullptr;
};