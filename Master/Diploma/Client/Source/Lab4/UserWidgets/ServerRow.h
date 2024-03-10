#pragma once

#include "CoreMinimal.h"
#include "MainMenu.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "ServerRow.generated.h"

UCLASS()
class LAB4_API UServerRow : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetServerName(const FString& Name) const;

	void Setup(UMainMenu *pParent, uint32 Index);

	UFUNCTION()
	void OnClicked();

	UPROPERTY(BlueprintReadOnly)
	bool bIsSelected = false;
	
private:
	UPROPERTY(meta=(BindWidget))
	UTextBlock *ServerName;

	UPROPERTY(meta=(BindWidget))
	UButton *ServerRowButton;
	
	UPROPERTY()
	UMainMenu *m_pParent;

	uint32 m_Index;
};
