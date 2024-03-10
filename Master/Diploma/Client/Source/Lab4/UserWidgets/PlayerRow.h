#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "PlayerRow.generated.h"

UCLASS()
class LAB4_API UPlayerRow : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPlayerName(const FString& Name) const;

private:
	UPROPERTY(meta=(BindWidget))
	UTextBlock *PlayerName;
};
