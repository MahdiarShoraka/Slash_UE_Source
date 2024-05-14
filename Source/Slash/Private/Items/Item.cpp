#include "Items/Item.h"
#include "Slash/Slash.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	RootComponent = ItemMesh;
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RunningTime += DeltaTime;

	FRotator Rotation = FRotator(0, DeltaTime * 2, 0);
	AddActorLocalRotation(Rotation);
}

float AItem::TransformSin() const
{
	return Amplitude * FMath::Sin(TimeConst * RunningTime); 
}

float AItem::TransformCos() const
{
	return Amplitude * FMath::Cos(TimeConst * RunningTime); 
}



