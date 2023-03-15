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
#include "eos_sdk.h"
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
	void SubmitPlayerScores(const uint32 PlayerFrags);
	UFUNCTION(Exec)
	void GetLeaderboardData();
	void LoginViaSDK();
	static void EOS_CALL CompletionDelegate(const EOS_Auth_LoginCallbackInfo* Data);
	static void EOS_CALL CompletionDelegateLeaderboards(const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo* Data);
	static void EOS_CALL CompletionDelegateIngestPlayerData(const EOS_Stats_IngestStatCompleteCallbackInfo* Data);
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Error);

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

	static EOS_EpicAccountId LoggedInUserId;
	
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
	EOS_HAuth AuthInterface;
	EOS_HConnect ConnectInterface;
	EOS_ProductUserId LocalUserId;
	EOS_ContinuanceToken ContinuanceToken;
	EOS_HStats StatsInterfaceHandle;
	EOS_HLeaderboards LeaderboardsHandle;
	EOS_HSessions SessionsHandle;
	EOS_ProductUserId LoggedInUserID;
	static EOS_EpicAccountId Test;
	
	bool bWasLoggedIn;
	bool bIsLanGame;
	
	FDelegateHandle TickDelegateHandle;
	TSharedPtr<FOnlineSessionSearch> SearchSettings;
	IOnlineSubsystem *OnlineSubsystem;
	IOnlineSessionPtr SessionPtr;
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
	
	const FName ServerNameKey = "ServerName";
	const FName SessionNameConst = "Session";
	const FString TravelGamePath = TEXT("/Game/ThirdPersonCPP/Maps/ThirdPersonExampleMap?listen");
	const FString TravelMainMenuPath = TEXT("/Game/MainMenu/MainMenuMap");
};
