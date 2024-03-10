#pragma once

#include "CoreMinimal.h"
#include "XmlFile.h"
#include "GameFramework/Actor.h"
#include "InputSaver.generated.h"

UCLASS()
class LAB4_API AInputSaver : public AActor
{
	GENERATED_BODY()

public:
	AInputSaver();

	void SaveServerName(const FString& ServerName) const;
	void SavePlayerName(const FString& PlayerName) const;

	FString LoadServerName() const;
	FString LoadPlayerName() const;

protected:
	virtual void BeginPlay() override;

private:
	FString m_ServerNameNode = "ServerName";
	FString m_PlayerNameNode = "PlayerName";
	FString m_Path = "SavedData.xml";
	FString m_BaseContent = "<SavedData>\n</SavedData>";
	FXmlFile *m_pFile;
};
