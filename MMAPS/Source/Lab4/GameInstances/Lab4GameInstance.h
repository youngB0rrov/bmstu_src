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
	void InitializeSDK();
	void InitializePlatformInterface();
	void InitializeAuthInterface();
	void InitializeAuthInterfaceViaExchangeCode();
	void InitializeConnectInterface();
	void LoginViaSDK();
	static void EOS_CALL CompletionDelegate(const EOS_Auth_LoginCallbackInfo* Data);
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Error);

	UFUNCTION(Exec)
	void FindSessions();
	void OnFindOnlineSessionsComplete(const bool bWasSuccessful);

	void OnJoinOnlineSessionComplete(const FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void DestroySession();
	
	FString GetPlayerName() const { return m_PlayerName; }
	
	UFUNCTION(Exec)
	void ShowInGameMenu();

	UFUNCTION()
	void SetupWinnerWidget(FString WinnerName);
	
	UFUNCTION(Exec)
	void ShowGameOverMenu();
	
	void CheckGameState();

	void RefreshGameState();
	
	void ChangeHealthBarState(float Persantage);
	void SetIsLanGame(const bool bIsLan);
	void SetIsOnlineGame(const bool bIsLan);
	bool GetIsLanGame() const;
	TArray<class ALab4Character*> GetAllCharacters() const { return m_Characters; }
	bool GetShouldBePause() const { return bShouldBePaused;}

	UFUNCTION(Exec)
	void HideGameOverMenu();
	
private:
	void CreateSession() const;
	void LoadMainMenu() const;
	bool bShouldBePaused;
	
	UPROPERTY()
	AMainMenuInitializer *m_pMainMenu;
	
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
	
	UPROPERTY()
	int32 FragsToWin = 3;
};
