// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Lab4HUD.generated.h"

/**
 * 
 */
UCLASS()
class LAB4_API ALab4HUD : public AHUD
{
	GENERATED_BODY()
public:
	ALab4HUD();
	
	virtual void DrawHUD() override;

	UPROPERTY()
	class UPlayerHealthBar* PlayerHealthBar;

	UPROPERTY()
	class ULab4ElimAnouncement* ElimAnnouncement;

	UPROPERTY()
	class UPlayerTable* PlayerTableWidget;

	UPROPERTY()
	class UMyUserWidget* GameOverWidget;

	UPROPERTY()
	class UAnnouncement* AnnouncementWidget;
	
	UPROPERTY(EditAnywhere, Category="Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> PlayerTableClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> AnnouncementWidgetClass;
	
	void AddCharacterOverlay();

	void AddElimOverlay(FString Attacker, FString Victim);

	void ShowGameOverWidget(const class ALab4PlayerState* WinnerPlayerState, float NormalizedPlayerScore);

	void AddAnnouncement();

	UFUNCTION(Exec)
	void AddPlayerList();

	void RemovePlayerList();

	bool bIsSet;

	FORCEINLINE bool GetIsSet() const { return bIsSet; }
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	float ElimAnnouncementDuration;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(ULab4ElimAnouncement* MessageToRemove);
private:
	TArray<ULab4ElimAnouncement*> ElimMessages;
};
