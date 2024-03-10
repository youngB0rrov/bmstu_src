// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHealthBar.h"

void UPlayerHealthBar::SetPersantage(float Persantage)
{
	HealthProgressBar->SetPercent(Persantage);
}
