// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PersonProjectile.generated.h"

UCLASS()
class LAB4_API APersonProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APersonProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(VisibleAnywhere, Category="Components")
	class USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category="Components")
	class UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, Category="Components")
	class UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY(VisibleAnywhere, Category="Components")
	class UParticleSystem* ExplosionEffect;

	UPROPERTY(VisibleAnywhere, Category="Audio")
	class USoundBase* ExplosionSoundEffect;

	UPROPERTY(EditAnywhere, Category="Damage")
	TSubclassOf<class UDamageType> DamageType;

	UPROPERTY(EditAnywhere, Category="Damage")
	float Damage;

	UFUNCTION(Category="Projectile")
	void OnProjectileImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;
};
