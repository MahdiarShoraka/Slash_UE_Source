// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Heal.h"
#include "Interfaces/PickupInterface.h"

void AHeal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DriftDown(DeltaTime);
}

void AHeal::BeginPlay()
{
	Super::BeginPlay();

	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.f, 0.f, 1000.f); // for tracing down to the ground

	DetermineSpawnLocation(Start, End);
}

void AHeal::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		PickupInterface->Heal(this);
		SpawnPickupSystem();
		SpawnPickupSound();
		Destroy();
	}
}
