#include "LabelName.h"

#include "Components/TextBlock.h"

void ULabelName::Setup(const FString& NameText)
{
	this->AddToViewport();
	
	Name->SetText(FText::FromString(NameText));
}

void ULabelName::Teardown()
{
	this->RemoveFromViewport();
}
