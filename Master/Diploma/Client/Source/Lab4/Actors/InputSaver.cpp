#include "InputSaver.h"

AInputSaver::AInputSaver()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AInputSaver::BeginPlay()
{
	Super::BeginPlay();

	m_pFile = new FXmlFile(FPaths::ProjectConfigDir() + m_Path, EConstructMethod::ConstructFromFile);
	
	if (m_pFile == nullptr) return;
	
	if (m_pFile->GetRootNode() == nullptr)
	{
		m_pFile = new FXmlFile(m_BaseContent, EConstructMethod::ConstructFromBuffer);
		m_pFile->Save(FPaths::ProjectConfigDir() + m_Path);
	}
}

void AInputSaver::SaveServerName(const FString& ServerName) const
{
	FXmlNode *Root = m_pFile->GetRootNode();
	FXmlNode *Child = Root->FindChildNode(m_ServerNameNode); 

	if (Child == nullptr)
	{
		Root->AppendChildNode(m_ServerNameNode, ServerName);
	}
	else
	{
		Child->SetContent(ServerName);
	}

	m_pFile->Save(FPaths::ProjectConfigDir() + m_Path);
}

void AInputSaver::SavePlayerName(const FString& PlayerName) const
{
	FXmlNode *Root = m_pFile->GetRootNode();
	FXmlNode *Child = Root->FindChildNode(m_PlayerNameNode); 

	if (Child == nullptr)
	{
		Root->AppendChildNode(m_PlayerNameNode, PlayerName);
	}
	else
	{
		Child->SetContent(PlayerName);
	}

	m_pFile->Save(FPaths::ProjectConfigDir() + m_Path);
}

FString AInputSaver::LoadServerName() const
{
	const FXmlNode *Child = m_pFile->GetRootNode()->FindChildNode(m_ServerNameNode); 

	if (Child != nullptr)
	{
		return Child->GetContent();
	}

	return "";
}

FString AInputSaver::LoadPlayerName() const
{
	const FXmlNode *Child = m_pFile->GetRootNode()->FindChildNode(m_PlayerNameNode); 

	if (Child != nullptr)
	{
		return Child->GetContent();
	}

	return "";
}
