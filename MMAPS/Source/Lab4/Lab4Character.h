// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "GameInstances/Lab4GameInstance.h"
#include "UserWidgets/GameMenu.h"
#include "Lab4Character.generated.h"

UCLASS(config=Game)
class ALab4Character : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	ALab4Character();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:
	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	virtual void Destroyed() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	UPROPERTY(ReplicatedUsing=OnRep_PlayerScore)
	uint32 PlayerScore;
	
	UPROPERTY(EditDefaultsOnly, Category="Gameplay and Projectile")
	TSubclassOf<class APersonProjectile> ProjectileClass;

	/** Delay between shots in seconds */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay")
	float FireRate;

	UPROPERTY()
	class ACurrentPlayerState* PlayerStatement;
	
	bool bIsFiring;
	
	bool bIsElimed;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 0.f;
	
	FTimerHandle ElimTimer;

	void ElimTimerFinished();
	
	UFUNCTION(Category="Gameplay")
	void StartFire();
	
	/** Function for stopping fire. When firing is stopped, player can start fire again. */
	UFUNCTION(Category="Gameplay")
	void StopFire();

	/** Function for spawning projectiles on server */
	UFUNCTION(Server, Reliable)
	void HandleFire();
	
	UFUNCTION()
	void OnRep_CurrentHealth();
	
	UFUNCTION()
	void OnRep_PlayerScore();

	void PollInit();
	
	FTimerHandle FiringTimer;
	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void Tick(float DeltaSeconds) override;
	
	void OnHealthUpdate();

	void OnScoreUpdate();
	
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintPure, Category="Health")
	FORCEINLINE float GetMaxHealth() const  { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(Category="Score")
	FORCEINLINE uint32 GetPlayerScore() const { return PlayerScore; }

	UFUNCTION(BlueprintCallable, Category="Health")
	void SetCurrentHealth(float healthValue);

	// UFUNCTION(BlueprintCallable, Category="Health")
	// virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	
public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShowPlayersFrags(const ALab4Character* p_Instigator, const ALab4Character* AttackerPlayer);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	// функция для возрождения игрока, выполняемая на сервере
	void Elim();

	void ShowPlayersFrags(const ALab4Character*, const ALab4Character*);

	void UpdateHUDHealth();
	
	UFUNCTION()
	void OnPressedPlayersList();
	
	UFUNCTION()
	void OnReleasedPlayersList();

	UFUNCTION(Reliable, Server, WithValidation)
	void AddPlayerName(const FString& Name);
	
	UFUNCTION(Reliable, Server, WithValidation)
	void RemovePlayerName();

	void SetPlayerNames(const TArray<FString>& Names);

	void ShowInGameMenu();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	float GetPlayerMaxHealth() const { return MaxHealth; }
	
	FString GetPlayerName() const;

	FString GetLocalPlayerName() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool IsServerPlayer() const;
	bool IsClientPlayer() const;
	
	UPROPERTY()
	class AGameInitializer *m_pGameInitializer;
	
	UPROPERTY(Replicated)
	TArray<FString> m_PlayerNames;

	UPROPERTY(Replicated)
	FString m_PlayerName;

	UPROPERTY()
	ULab4GameInstance *GameInstance;

	FVector2D MouseInput;

	UPROPERTY()
	class ALab4PlayerState* Lab4PlayerState;

	UPROPERTY()
	class ALab4HUD* Lab4HUD;

	UPROPERTY()
	class ALab4PlayerController* Lab4PlayerController;
};

