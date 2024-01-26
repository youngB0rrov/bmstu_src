// Fill out your copyright notice in the Description page of Project Settings.


#include "PersonProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Lab4/Lab4Character.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
APersonProjectile::APersonProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	SphereComponent->SetSphereRadius(37.5f);
	SphereComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = SphereComponent;

	if (HasAuthority())
	{
		SphereComponent->OnComponentHit.AddDynamic(this, &APersonProjectile::OnProjectileImpact);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Engine/BasicShapes/Sphere"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(RootComponent);

	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -37.5f));
		StaticMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	}

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);
	ProjectileMovementComponent->InitialSpeed = 1500.0f;
	ProjectileMovementComponent->MaxSpeed = 1500.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.1f;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultExplosionEffect(TEXT("/Game/StarterContent/Particles/P_Explosion"));
	if (DefaultExplosionEffect.Succeeded())
	{
		ExplosionEffect = DefaultExplosionEffect.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultExplosionSoundEffect(TEXT("/Game/StarterContent/Audio/Explosion_Cue"));
	if (DefaultExplosionSoundEffect.Succeeded())
	{
		ExplosionSoundEffect = DefaultExplosionSoundEffect.Object;
	}
	
	DamageType = UDamageType::StaticClass();
	Damage = 50.0f;
}

// Called when the game starts or when spawned
void APersonProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void APersonProjectile::OnProjectileImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor)
	{
		APlayerController* PlayerController = static_cast<APlayerController*>(OtherActor->GetInstigatorController());

		if (PlayerController == nullptr)
		{
			Destroy();
			return;
		}

		ALab4Character* OtherPlayerCharacter = static_cast<ALab4Character*>(PlayerController->GetPawn());

		if (GetInstigator()->Controller == nullptr)
		{
			Destroy();
			return;
		}
		
		ALab4Character* InsitagorPlayer = static_cast<ALab4Character*>(GetInstigator()->Controller->GetPawn());

		if (OtherPlayerCharacter == nullptr || InsitagorPlayer == nullptr)
		{
			Destroy();
			return;
		}
		
		if (OtherPlayerCharacter != InsitagorPlayer && OtherPlayerCharacter->GetCurrentHealth() > 0.0f)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigator()->Controller,
			this, DamageType);
		}
	}
	
	Destroy();
}

// Called every frame
void APersonProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APersonProjectile::Destroyed()
{
	Super::Destroyed();
	FVector spawnLocation = GetActorLocation();
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
	UGameplayStatics::SpawnSoundAtLocation(this, ExplosionSoundEffect, spawnLocation, FRotator::ZeroRotator);
}

