#pragma once

#include "CoreMinimal.h"
#include "PlayerTableRow.h"
#include "Blueprint/UserWidget.h"
#include "RankedLeaderboardRow.h"
#include "Components/SizeBox.h"
#include "Components/EditableText.h"
#include "MainMenu.generated.h"

class UScrollBox;
UCLASS()
class LAB4_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenu(const FObjectInitializer& ObjectInitializer);
	
	void SetMainMenu(class AMainMenuInitializer* const pMainMenu);
	void SetServersList(const TArray<FString>& List);

	void SetServerName(const FString& SessionName) const;
	void SetPlayerName(const FString& Name) const;
	
	UFUNCTION()
	void OnClickedCreate();
	
	UFUNCTION()
	void OnClickedJoin();
	
	UFUNCTION()
	void OnClickedMain();
	
	UFUNCTION()
	void OnClickedPlayerNameAtJoining();
	
	UFUNCTION()
	void OnClickedPlayerNameAtCreation();
	
	UFUNCTION()
	void OnClickedStartGame();
	
	UFUNCTION()
	void OnLanButtonClicked();
	
	UFUNCTION()
	void OnInternetButtonClicked();

	UFUNCTION()
	void OnToSelectModeButtonClicked();

	UFUNCTION()
	void OnTogglePasswordButtonPressed();

	UFUNCTION()
	void OnTogglePasswordButtonReleased();

	UFUNCTION()
	void OnJoinMatchPasswordToggleButtonPressed();

	UFUNCTION()
	void OnJoinMatchPasswordToggleButtonReleased();

	UFUNCTION()
	void OnCreateMatchPasswordToggleButtonPressed();

	UFUNCTION()
	void OnCreateMatchPasswordToggleButtonReleased();

	UFUNCTION()
	void OnLogInButtonClicked();
	
	UFUNCTION()
	void OnCancelAuthButtonClicked();

	UFUNCTION()
	void OnMainMenuExitButtonClicked();

	UFUNCTION()
	void OnLoginViaAccountButtonClicked();

	UFUNCTION()
	void OnCancelCredentialsClicked();

	UFUNCTION()
	void OnConfirmCredentialsClicked();

	UFUNCTION()
	void OnLeaderboardsButtonClicked();

	UFUNCTION()
	void OnLeaderboardBackButtonClicked();

	UFUNCTION()
	void OnRecruitsLeagueButtonClicked();

	UFUNCTION()
	void OnGuardiansLeagueButtonClicked();

	UFUNCTION()
	void OnCrusaidersLeagueButtonClicked();

	UFUNCTION()
	void OnLegendsLeagueButtonClicked();

	UFUNCTION()
	void OnMatchmakingButtonClicked();

	UFUNCTION()
	void OnMatchmakingCreateButtonClicked();

	UFUNCTION()
	void OnMatchmakingBackButtonClicked();

	UFUNCTION()
	void OnPrivateGameButtonClicked();

	UFUNCTION()
	void OnCancelPrivateGameButtonClicked();

	UFUNCTION()
	void OnCancelLanGameButtonClicked();
	
	TArray<FText> GetCredentials();
	
	void Setup();
	void Teardown();
	void SetWidgetOnLoginComplete();
	void AddRankedLeaderBoardRow(URankedLeaderboardRow* PlayerTableRow);
	void ClearRankedLeaderboardList();
	void SetFindingMatchStatusWidgetVisibility(bool bIsVisible);
	void SelectIndex(uint32 Index);
	void SetMatchmakingHintTextVisibility(bool bIsVisible);
	void SetCreateGameHintTextVisibility(bool bIsVisible);
	void HandleMatchmakingStatusAndConnect();
	void GiveAccessToCreateMatchSection();

protected:
	virtual bool Initialize() override;
	
private:
	void UpdateChildren(uint32 NewIndexSelected);
	void SetMatchmakingStatusText(const FString& StatusText);
	
	UPROPERTY(meta=(BindWidget))
	class UButton* CreatePrivateGameButton;

	UPROPERTY(meta=(BindWidget))
	UButton* JoinPrivateGameButton;

	UPROPERTY(meta=(BindWidget))
	UButton *CancelJoinConfirmButton;

	UPROPERTY(meta=(BindWidget))
	UButton *JoinGameConfirmButton;

	UPROPERTY(meta=(BindWidget))
	UButton *CancelSessionNameConfirmButton;

	UPROPERTY(meta=(BindWidget))
	UButton *SessionNameConfirmButton;

	UPROPERTY(meta=(BindWidget))
	UButton *CancelNameConfirmButton;

	UPROPERTY(meta=(BindWidget))
	UButton* MainMenuExitButton;
	
	UPROPERTY(meta=(BindWidget))
	UButton *NameConfirmButton;

	UPROPERTY(meta=(BindWidget))
	UButton *LANButton;

	UPROPERTY(meta=(BindWidget))
	UButton *InternetButton;

	UPROPERTY(meta=(BindWidget))
	UButton *LogInButton;

	UPROPERTY(meta=(BindWidget))
	UButton* LoginAccountButton;

	UPROPERTY(meta=(BindWidget))
	UButton *CancelAuthButton;

	UPROPERTY(meta=(BindWidget))
	UButton* CancelCredentialsButton;

	UPROPERTY(meta=(BindWidget))
	UButton* ConfirmCredentials;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchmakingCreateButtonHintText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CreateGameHintTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchmakingStatusText;

	UPROPERTY(meta=(BindWidget))
	UButton* LeaderboardBackButton;

	UPROPERTY(meta=(BindWidget))
	UButton* TogglePassword;

	UPROPERTY(meta=(BindWidget))
	UButton* RecruitsLeagueButton;

	UPROPERTY(meta=(BindWidget))
	UButton* GuardiansLeagueButton;

	UPROPERTY(meta=(BindWidget))
	UButton* CrusaidersLeagueButton;

	UPROPERTY(meta=(BindWidget))
	UButton* LegendsLeagueButton;

	UPROPERTY(meta=(BindWidget))
	UScrollBox* RankedPlayersList;

	UPROPERTY(meta=(BindWidget))
	UEditableText* UserEmail;

	UPROPERTY(meta=(BindWidget))
	UEditableText* UserPassword;

	UPROPERTY(meta=(BindWidget))
	UEditableText* JoinMatchPassword;

	UPROPERTY(meta = (BindWidget))
	UEditableText* CreateMatchPassword;

	UPROPERTY(meta=(BindWidget))
	UButton* JoinMatchPasswordToggleButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateMatchPasswordToggleButton;

	UPROPERTY(meta=(BindWidget))
	class UWidgetSwitcher *MenuSwitcher;

	UPROPERTY(meta=(BindWidget))
	class UWidget *ModeSelect;

	UPROPERTY(meta=(BindWidget))
	class UWidget *AuthMenu;
	
	UPROPERTY(meta=(BindWidget))
	class UWidget *JoinMenu;

	UPROPERTY(meta=(BindWidget))
	class UWidget *SessionNameMenu;
	
	UPROPERTY(meta=(BindWidget))
	class UWidget *NameMenu;

	UPROPERTY(meta=(BindWidget))
	class UWidget* InternetGameWidget;

	UPROPERTY(meta=(BindWidget))
	UButton* InternetPrivateGameButton;

	UPROPERTY(meta = (BindWidget))
	UButton* RaitingGameButton;

	UPROPERTY(meta=(BindWidget))
	UButton* TopPlayersButton;

	UPROPERTY(meta=(BindWidget))
	UWidget* LanGameWidget;

	UPROPERTY(meta=(BindWidget))
	UButton* LanPrivateGameButton;

	UPROPERTY(meta=(BindWidget))
	UButton* CancelLanGameButton;

	UPROPERTY(meta=(BindWidget))
	UButton* CancelInternetGameButton;

	UPROPERTY(meta=(BindWidget))
	UWidget* Credentials;

	UPROPERTY(meta=(BindWidget))
	UWidget* Leaderboard;

	UPROPERTY(meta = (BindWidget))
	UWidget* Matchmaking;

	UPROPERTY(meta = (BindWidget))
	UButton* MatchmakingCreateButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MatchmakingBackButton;

	UPROPERTY(meta = (BindWidget))
	UWidget* PrivateMatch;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelPrivateGameButton;

	UPROPERTY(meta = (BindWidget))
	USizeBox* StatusSizeBox;

	UPROPERTY(meta=(BindWidget))
	class UPanelWidget *ServersList;

	UPROPERTY(meta=(BindWidget))
	class UEditableText *SessionNameEnterText;
	
	UPROPERTY(meta=(BindWidget))
	class UEditableText *NameEnterText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* LeagueNameText;
	
	UPROPERTY()
	class AMainMenuInitializer *m_pMainMenu;

	TSubclassOf<UUserWidget> m_ServerRowWidget;

	TOptional<uint32> m_SelectedIndex;

	bool m_bIsCreateGame;

	bool bWasLoggedIn;

	const FString Waiting = "Please, wait...";
	const FString Nothing = "Sorry, nothing has found. Try again...";
	const FText LanGame = FText::FromString("LAN Game");
	const FText InternetGame = FText::FromString("Internet Game");
};
