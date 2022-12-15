#include "ServerRow.h"

void UServerRow::SetServerName(const FString& Name) const
{
	ServerName->SetText(FText::FromString(Name));
}

void UServerRow::Setup(UMainMenu* pParent, uint32 Index)
{
	m_pParent = pParent;
	m_Index = Index;
	
	ServerRowButton->OnClicked.AddDynamic(this,
		&UServerRow::OnClicked);
}

void UServerRow::OnClicked()
{
	m_pParent->SelectIndex(m_Index);
}
