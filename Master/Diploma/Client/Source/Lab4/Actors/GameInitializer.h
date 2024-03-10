#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameInitializer.generated.h"

UCLASS()
class LAB4_API AGameInitializer : public AActor
{
	GENERATED_BODY()

public:
	AGameInitializer();

	void ConnectCharacter(class ALab4Character* const pCharacter);

	void SetPlayerName(const FString& PlayerName);
	
	void CharacterOnShowPlayers(const TArray<FString>& PlayerNames) const;
	void CharacterOnHidePlayers() const;
	
	void TeardownAll() const;
	
	FORCEINLINE FString GetPlayerName() const { return m_PlayerName;}
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	float m_TimePingRecheck;
	
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	float m_TimerExit;

	UPROPERTY()
	class ALab4Character *m_pCharacter;

	UPROPERTY(VisibleAnywhere)
	TArray<ALab4Character*> p_AllCharacters;
	
	UPROPERTY()
	class UWidgetPlayers *m_pAllPlayers;
	
	UPROPERTY()
	class ULabelName *m_pLabelName;
	
	TSubclassOf<UUserWidget> m_AllPlayersWidget;
	TSubclassOf<UUserWidget> m_LabelNameWidget;

	FString m_PlayerName;
};
