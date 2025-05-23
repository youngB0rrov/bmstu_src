// Copyright Epic Games, Inc. All Rights Reserved.

#include "Lab4Character.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Actors/GameInitializer.h"
#include "Actors/PersonProjectile.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameInstances/Lab4GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Lab4GameMode.h"
#include <string>

#include "Lab4HUD.h"
#include "Lab4PlayerController.h"
#include "Lab4PlayerState.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "UserWidgets/WidgetPlayers.h"

//////////////////////////////////////////////////////////////////////////
// ALab4Character

ALab4Character::ALab4Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	PlayerScore = 0;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = true; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	SetReplicates(true);

	ProjectileClass = APersonProjectile::StaticClass();

	FireRate = 0.25f;
	bIsFiring = false;
	bIsElimed = false;
	ElimDelay = 0.1f;
	bIsInGameMenu = false;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ALab4Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ALab4Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ALab4Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ALab4Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ALab4Character::LookUpAtRate);

	PlayerInputComponent->BindAction("ShowInGameMenu", IE_Pressed, this, &ALab4Character::ShowInGameMenu);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ALab4Character::StartFire);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ALab4Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ALab4Character::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ALab4Character::OnResetVR);

	PlayerInputComponent->BindAction("Show or Hide Players List", IE_Pressed, this,
	                                 &ALab4Character::OnPressedPlayersList);
	PlayerInputComponent->BindAction("Show or Hide Players List", IE_Released, this,
	                                 &ALab4Character::OnReleasedPlayersList);
	PlayerInputComponent->BindAction("PushToTalk", IE_Pressed, this, &ALab4Character::PushToTalkPressed);
	PlayerInputComponent->BindAction("PushToTalk", IE_Released, this, &ALab4Character::PushToTalkReleased);
}

void ALab4Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PollInit();
}

void ALab4Character::OnHealthUpdate()
{
	// вызывается дважды при изменении всех реплицированных переменных на клиенте
	UpdateHUDHealth();
}

void ALab4Character::OnScoreUpdate()
{
	if (IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
		                                 FString::Printf(TEXT("You now have %u"), PlayerScore));
	}
}

void ALab4Character::OnResetVR()
{
	// If Lab4 is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in Lab4.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ALab4Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ALab4Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ALab4Character::StartFire()
{
	if (bIsInGameMenu) return;
	if (bDisableGameplay || bDisableFire) return;
	
	if (!bIsFiring)
	{
		bIsFiring = true;
		UWorld* World = GetWorld();

		if (World == nullptr) return;

		World->GetTimerManager().SetTimer(FiringTimer, this, &ALab4Character::StopFire, FireRate, false);
		HandleFire();
	}
}

void ALab4Character::StopFire()
{
	bIsFiring = false;
}

void ALab4Character::HandleFire_Implementation()
{
	FVector spawnLocation = GetActorLocation() + (GetControlRotation().Vector() * 100.0f) + (GetActorUpVector() *
		50.0f);
	FRotator spawnRotation = GetControlRotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = GetInstigator();
	SpawnParameters.Owner = this;

	GetWorld()->SpawnActor<APersonProjectile>(
		spawnLocation, spawnRotation, SpawnParameters);
}

void ALab4Character::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void ALab4Character::OnRep_PlayerScore()
{
	OnScoreUpdate();
}

void ALab4Character::PollInit()
{
	if (Lab4PlayerState == nullptr)
	{
		Lab4PlayerState = GetPlayerState<ALab4PlayerState>();
		GameInstance = GameInstance == nullptr ? GetGameInstance<ULab4GameInstance>() : GameInstance;
		
		if (Lab4PlayerState)
		{
			if (GameInstance->GetIsLanGame())
			{
				m_PlayerName = GameInstance->GetPlayerName();
				AddPlayerNameOnServer(m_PlayerName);
			}
			if ((HasAuthority() && IsLocallyControlled()) || (GetLocalRole() == ROLE_AutonomousProxy))
			{
				Lab4PlayerState->AddToScore(0);
			}
		}
	}
}

void ALab4Character::PushToTalkPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("Player is talking"));
	GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("ToggleSpeaking 1")));
}

void ALab4Character::PushToTalkReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("Player isn't talking anymore"));
	GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("ToggleSpeaking 0")));
}

void ALab4Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ALab4Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ALab4Character::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ALab4Character::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ALab4Character::Destroyed()
{
	Super::Destroyed();
}

void ALab4Character::BroadcastSubmitPlayerRankedScores(float TotalNormalizedPlayerScores)
{
	SubmitRankedScores(TotalNormalizedPlayerScores);
}

void ALab4Character::AddLogginMessageToHUD_Implementation(const FString& NewPlayerName, uint32 CurrenPlayersNum,
	uint32 TotalPlayersNum)
{
	if (GEngine)
	{
		const FVector2D TextSize = FVector2D(1.5f, 1.5f);
		
		GEngine->AddOnScreenDebugMessage(
			1,
			60.f,
			FColor::Yellow,
			FString::Printf(TEXT("Waiting for other players: %u/%u"), CurrenPlayersNum, TotalPlayersNum),
			false,
			TextSize
			);


		GEngine->AddOnScreenDebugMessage(
			-1,
			10.f,
			FColor::Green,
			FString::Printf(TEXT("Player %s has just joind the game!"), *NewPlayerName),
			false,
			TextSize
			);
	}
}

void ALab4Character::BroadcastAddLoginMessageToHUD(const FString& NewPlayerName, uint32 CurrenPlayersNum,
                                                   uint32 TotalPlayersNum)
{
	AddLogginMessageToHUD(NewPlayerName, CurrenPlayersNum, TotalPlayersNum);
}

void ALab4Character::RemoveLogginMessage_Implementation()
{
	if(GEngine)
	{
		GEngine->RemoveOnScreenDebugMessage(1);
		GEngine->RemoveOnScreenDebugMessage(-1);
	}
}

void ALab4Character::BroadcastRemoveLoginMessage()
{
	RemoveLogginMessage();
}

void ALab4Character::BeginPlay()
{
	Super::BeginPlay();

	if (IsServerPlayer() || IsClientPlayer())
	{
		/* Initialize Character State */
		if (m_PlayerName.IsEmpty())
		{
			m_PlayerName = GetGameInstance<ULab4GameInstance>()->GetPlayerName();
		}
		AddPlayerName(m_PlayerName);
		m_pGameInitializer = GetWorld()->SpawnActor<AGameInitializer>(AGameInitializer::StaticClass());
		m_pGameInitializer->ConnectCharacter(this);
		m_pGameInitializer->SetPlayerName(m_PlayerName);

		
		// APlayerController* PlayerController = Cast<APlayerController>(Controller);
		// Lab4HUD = Lab4HUD == nullptr ? PlayerController->GetHUD<ALab4HUD>() : Lab4HUD;
		// if (Lab4HUD && !Lab4HUD->GetIsSet())
		// {
		// 	Lab4HUD->AddCharacterOverlay();
		// }
		UpdateHUDHealth();
		
	}

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ALab4Character::ReceiveDamage);
	}
}

void ALab4Character::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	RemovePlayerName();
}

void ALab4Character::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth + 1.0f);
		// Обновить хад на сервере
		// UpdateHUDHealth();
	}
}

void ALab4Character::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                   AController* InstigatorController, AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();

	if (CurrentHealth <= 0)
	{
		Lab4PlayerController = Lab4PlayerController == nullptr
			                       ? Cast<ALab4PlayerController>(Controller)
			                       : Lab4PlayerController;
		ALab4PlayerController* AttackerController = Cast<ALab4PlayerController>(InstigatorController);
		ALab4GameMode* MyGameMode = GetWorld()->GetAuthGameMode<ALab4GameMode>();
		if (MyGameMode && Lab4PlayerController && AttackerController)
		{
			MyGameMode->PlayerEliminated(this, Lab4PlayerController, AttackerController);
		}
	}
}

void ALab4Character::UpdateHUDHealth()
{
	Lab4PlayerController = Lab4PlayerController == nullptr
		                       ? Cast<ALab4PlayerController>(Controller)
		                       : Lab4PlayerController;
	if (Lab4PlayerController)
	{
		Lab4PlayerController->SetHUDHealth(CurrentHealth == 0.0 ? 100.0 : CurrentHealth, MaxHealth);
	}
}

void ALab4Character::MulticastElim_Implementation()
{
	bIsElimed = true;
}

void ALab4Character::Elim()
{
	// будет вызвана на сервере
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ALab4Character::ElimTimerFinished,
		ElimDelay
	);
}

void ALab4Character::ElimTimerFinished()
{
	ALab4GameMode* MyGameMode = GetWorld()->GetAuthGameMode<ALab4GameMode>();
	if (MyGameMode)
	{
		MyGameMode->RequestRespawn(this, Controller);
	}
}

void ALab4Character::OnPressedPlayersList()
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	Lab4HUD = Lab4HUD == nullptr ? PlayerController->GetHUD<ALab4HUD>() : Lab4HUD;

	if (Lab4HUD)
	{
		Lab4HUD->AddPlayerList();
	}
}

void ALab4Character::OnReleasedPlayersList()
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	Lab4HUD = Lab4HUD == nullptr ? PlayerController->GetHUD<ALab4HUD>() : Lab4HUD;

	if (Lab4HUD)
	{
		Lab4HUD->RemovePlayerList();
	}
}

void ALab4Character::AddPlayerNameOnServer_Implementation(const FString& PlayerName)
{
	if (GameInstance->GetIsLanGame() == true)
	{
		Lab4PlayerState->SetPlayerName(PlayerName);
	}
}

void ALab4Character::AddPlayerName_Implementation(const FString& Name)
{
	GetGameInstance<ULab4GameInstance>()->AddCharacter(this, Name);
}

bool ALab4Character::AddPlayerName_Validate(const FString& Name)
{
	return true;
}

void ALab4Character::RemovePlayerName_Implementation()
{
	GetGameInstance<ULab4GameInstance>()->RemoveCharacter(this);
}

bool ALab4Character::RemovePlayerName_Validate()
{
	return true;
}

void ALab4Character::SetPlayerNames(const TArray<FString>& Names)
{
	m_PlayerNames = Names;
}

void ALab4Character::ShowInGameMenu()
{
	bIsInGameMenu = !bIsInGameMenu;
	GetGameInstance<ULab4GameInstance>()->ShowInGameMenu();
}

FString ALab4Character::GetPlayerName() const
{
	return this->GetPlayerState()->GetPlayerName();
	// return m_PlayerName;
}

void ALab4Character::GetNormalizedMatchData(float& NormalizedPlayerScore, TArray<float> PlayersFrags, const float PlayerFrags)
{
	const float MinFragsValue = FMath::Min<float>(PlayersFrags);
	const float MaxFragsValue = FMath::Max<float>(PlayersFrags);

	const float NormalizedPlayerFrags = (2.06 * PlayerFrags - 1.03 * MinFragsValue - 1.03 * MaxFragsValue) / (MaxFragsValue - MinFragsValue);

	NormalizedPlayerScore = .4849 * FMath::Pow(NormalizedPlayerFrags, 3) - 1E-14 * FMath::Pow(NormalizedPlayerFrags, 2) + .4674 * NormalizedPlayerFrags + 3E-14;
	NormalizedPlayerScore = FMath::Clamp(NormalizedPlayerScore, -1.f, 1.f);
}

void ALab4Character::IngestMatchData(float& TotalNormalizedPlayerScores)
{
	float NormalizedPlayerScore;
	TArray<float> AllPlayerFrags;
	
	for (FConstPlayerControllerIterator Item = GetWorld()->GetPlayerControllerIterator(); Item; ++Item)
	{
		const APlayerController* PlayerController = Cast<APlayerController>(*Item);
		AllPlayerFrags.Add(Cast<ALab4PlayerState>(PlayerController->PlayerState)->GetPlayerScore());
	}

	GetNormalizedMatchData(NormalizedPlayerScore, AllPlayerFrags, GetPlayerState<ALab4PlayerState>()->GetPlayerScore());
	TotalNormalizedPlayerScores = NormalizedPlayerScore;
}

void ALab4Character::SubmitRankedScores_Implementation(float TotalNormalizedPlayerScores)
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());

	if (OnlineSubsystem == nullptr)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Submited player score (for LAN): %d"), FMath::FloorToInt(RankedCoefficient * TotalNormalizedPlayerScores));

	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		UE_LOG(LogTemp, Warning, TEXT("Online subsystem is NULL, this is a LAN game"));
		return;
	}
	
	IOnlineIdentityPtr IdentityPtr = OnlineSubsystem->GetIdentityInterface();
	IOnlineStatsPtr StatsPtr = OnlineSubsystem->GetStatsInterface();

	FOnlineStatsUserUpdatedStats Stat = FOnlineStatsUserUpdatedStats(IdentityPtr->GetUniquePlayerId(0).ToSharedRef());

	Stat.Stats.Add(RankedStatName, FOnlineStatUpdate(FMath::FloorToInt(RankedCoefficient * TotalNormalizedPlayerScores), FOnlineStatUpdate::EOnlineStatModificationType::Sum));

	TArray<FOnlineStatsUserUpdatedStats> Stats;
	Stats.Add(Stat);

	StatsPtr->UpdateStats(
		IdentityPtr->GetUniquePlayerId(0).ToSharedRef(),
		Stats,
		FOnlineStatsUpdateStatsComplete::CreateLambda([](const FOnlineError &ResultState)
		{
			UE_LOG(LogTemp, Warning, TEXT("Total score submit result: %d"), ResultState.bSucceeded);

			if (!ResultState.bSucceeded)
			{
				UE_LOG(LogTemp, Error, TEXT("%s"), *ResultState.ErrorMessage.ToString());
			}
		}));
}

void ALab4Character::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALab4Character, m_PlayerNames);
	DOREPLIFETIME(ALab4Character, CurrentHealth);
	DOREPLIFETIME(ALab4Character, PlayerScore);
	DOREPLIFETIME(ALab4Character, bDisableGameplay);
	DOREPLIFETIME(ALab4Character, bDisableCharacterOverlay);
	DOREPLIFETIME(ALab4Character, bDisableFire);
}

bool ALab4Character::IsServerPlayer() const
{
	return GetLocalRole() == ROLE_Authority && GetRemoteRole() == ROLE_AutonomousProxy;
}

bool ALab4Character::IsClientPlayer() const
{
	return GetLocalRole() == ROLE_AutonomousProxy && GetRemoteRole() == ROLE_Authority;
}
