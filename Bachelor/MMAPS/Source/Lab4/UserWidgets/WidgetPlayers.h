#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WidgetPlayers.generated.h"

UCLASS()
class LAB4_API UWidgetPlayers : public UUserWidget
{
	GENERATED_BODY()

public:
	UWidgetPlayers(const FObjectInitializer& ObjectInitializer);
	
	void SetPlayersList(const TArray<FString>& PlayerNames) const;
	
	void Setup();
	void Teardown();

private:
	TSubclassOf<UUserWidget> m_PlayerRowWidget;
};
