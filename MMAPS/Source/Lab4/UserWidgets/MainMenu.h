#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "MainMenu.generated.h"

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

	TArray<FText> GetCredentials();
	
	void Setup();
	void Teardown();
	void SetWidgetOnLoginComplete();
	
	void SelectIndex(uint32 Index);

protected:
	virtual bool Initialize() override;
	
private:
	void UpdateChildren(uint32 NewIndexSelected);
	
	UPROPERTY(meta=(BindWidget))
	class UButton *CreateGameButton;

	UPROPERTY(meta=(BindWidget))
	UButton *JoinGameButton;

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
	UButton *CancelInternetGame;

	UPROPERTY(meta=(BindWidget))
	UButton *CancelAuthButton;

	UPROPERTY(meta=(BindWidget))
	UButton* CancelCredentialsButton;

	UPROPERTY(meta=(BindWidget))
	UButton* ConfirmCredentials;

	UPROPERTY(meta=(BindWidget))
	UButton* TogglePassword;

	UPROPERTY(meta=(BindWidget))
	UEditableText* UserEmail;

	UPROPERTY(meta=(BindWidget))
	UEditableText* UserPassword;

	UPROPERTY(meta=(BindWidget))
	class UWidgetSwitcher *MenuSwitcher;

	UPROPERTY(meta=(BindWidget))
	class UWidget *MainMenu;

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
	UWidget* Credentials;

	UPROPERTY(meta=(BindWidget))
	class UPanelWidget *ServersList;

	UPROPERTY(meta=(BindWidget))
	class UEditableText *SessionNameEnterText;
	
	UPROPERTY(meta=(BindWidget))
	class UEditableText *NameEnterText;
	
	UPROPERTY()
	class AMainMenuInitializer *m_pMainMenu;

	TSubclassOf<UUserWidget> m_ServerRowWidget;

	TOptional<uint32> m_SelectedIndex;

	bool m_bIsCreateGame;

	bool bWasLoggedIn;

	const FString Waiting = "Please, wait...";
	const FString Nothing = "Sorry, nothing has found. Try again...";
};
