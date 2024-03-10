#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MainMenuInitializer.generated.h"

UCLASS()
class LAB4_API AMainMenuInitializer : public AActor
{
	GENERATED_BODY()

public:
	AMainMenuInitializer();
	
	void OnWidgetToJoin() const;
	void OnWidgetToCreate() const;
	void OnWidgetToMain(bool bIsFromCreate) const;
	void OnWidgetToPlayerNameAtJoining(uint32 Index) const;
	void OnWidgetToPlayerNameAtCreation(const FString& ServerName) const;
	void OnWidgetToStartGame(const FString& PlayerName, bool bIsFromCreate) const;
	void OnInstanceFoundServers(const TArray<FString>& ServerNames) const;
	void SetWidgetOnLoginComplete();
	void AddRankedLeaderboardRow(class URankedLeaderboardRow* PlayerRow);
	void ClearRankedLeaderboardList();
	TArray<FText> GetCredentials();
	void OnUpdaterTimerRunOut() const;
	void PollInit();
	void TeardownAll() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
private:
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	float m_UpdateServersListTime;

	UPROPERTY()
	class ULab4GameInstance *m_pGameInstance;
	
	UPROPERTY()
	class UMainMenu *m_pMainMenu;

	UPROPERTY()
	class AInputSaver *m_pInputSaver;
	
	UPROPERTY()
	class AJoinGameUpdater *m_pServersListUpdater;

	UPROPERTY()
	class ALab4PlayerState* Lab4PlayerState;

	FString GamerName;
	
	TSubclassOf<UUserWidget> m_MainMenuWidget;
};
