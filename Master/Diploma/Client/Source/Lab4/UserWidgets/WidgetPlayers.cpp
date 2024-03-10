#include "WidgetPlayers.h"

#include "PlayerRow.h"
#include "Components/PanelWidget.h"

UWidgetPlayers::UWidgetPlayers(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> PlayerRowWBP(TEXT("/Game/MainMenu/WBP_PlayerRow"));

	m_PlayerRowWidget = PlayerRowWBP.Class;
}

void UWidgetPlayers::SetPlayersList(const TArray<FString>& PlayerNames) const
{
	UWorld *pWorld = GetWorld();
}

void UWidgetPlayers::Setup()
{
	this->AddToViewport();
}

void UWidgetPlayers::Teardown()
{
	this->RemoveFromViewport();
}
