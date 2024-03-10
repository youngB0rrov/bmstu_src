#include "GameInitializer.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Lab4/Lab4Character.h"
#include "Lab4/UserWidgets/LabelName.h"
#include "Lab4/UserWidgets/WidgetPlayers.h"
#include "Net/UnrealNetwork.h"

AGameInitializer::AGameInitializer()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	static ConstructorHelpers::FClassFinder<UUserWidget> LabelNameWBP(TEXT("/Game/MainMenu/WBP_LabelName"));
	m_LabelNameWidget = LabelNameWBP.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> AllPlayersWBP(TEXT("/Game/MainMenu/WBP_AllPlayers"));
	m_AllPlayersWidget = AllPlayersWBP.Class;
}

void AGameInitializer::BeginPlay()
{
	Super::BeginPlay();

	m_pLabelName = CreateWidget<ULabelName>(GetWorld(), m_LabelNameWidget);
	m_pLabelName->Setup(m_PlayerName);

	m_pAllPlayers = CreateWidget<UWidgetPlayers>(GetWorld(), m_AllPlayersWidget);
}

void AGameInitializer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	TeardownAll();
}

void AGameInitializer::ConnectCharacter(ALab4Character* const pCharacter)
{
	m_pCharacter = pCharacter;
	p_AllCharacters.Add(pCharacter);
}

void AGameInitializer::SetPlayerName(const FString& PlayerName)
{
	m_PlayerName = PlayerName;

	if (m_pLabelName != nullptr)
	{
		m_pLabelName->Setup(PlayerName);
	}
}

void AGameInitializer::CharacterOnShowPlayers(const TArray<FString>& PlayerNames) const
{
	if (m_pAllPlayers)
	{
		m_pAllPlayers->Setup();
		m_pAllPlayers->SetPlayersList(PlayerNames);
	}
}

void AGameInitializer::CharacterOnHidePlayers() const
{
	if (m_pAllPlayers)
	{
		m_pAllPlayers->Teardown();
	}
}

void AGameInitializer::TeardownAll() const
{
	m_pAllPlayers->Teardown();
	m_pLabelName->Teardown();
}
