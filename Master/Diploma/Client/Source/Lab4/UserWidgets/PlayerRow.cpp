#include "PlayerRow.h"

void UPlayerRow::SetPlayerName(const FString& Name) const
{
	PlayerName->SetText(FText::FromString(Name));
}
