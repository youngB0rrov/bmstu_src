// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab4ElimAnouncement.h"

#include "Components/TextBlock.h"

void ULab4ElimAnouncement::SetElimAnnouncement(const FString Attacker, const FString Victim)
{
	if (ElimText)
	{
		FString ElimAnnouncementText = FString::Printf(TEXT("%s eliminated %s"), *Attacker, *Victim);
		ElimText->SetText(FText::FromString(ElimAnnouncementText));
	}
}
