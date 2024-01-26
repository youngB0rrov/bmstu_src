#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LabelName.generated.h"

UCLASS()
class LAB4_API ULabelName : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(const FString& NameText);
	void Teardown();

private:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock *Name;
};
