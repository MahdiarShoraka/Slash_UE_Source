#include "Items/Item.h"
#include "Components/SphereComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	RootComponent = ItemMesh;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(RootComponent);
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	// Bind the callback function to the delegate
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSpherEndOverlap);
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//RunningTime += DeltaTime;
}

float AItem::TransformSin() const
{
	return Amplitude * FMath::Sin(TimeConst * RunningTime); 
}

float AItem::TransformCos() const
{
	return Amplitude * FMath::Cos(TimeConst * RunningTime); 
}

// callback function definition
void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComponent ,int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FString OtherActorName = OtherActor->GetName();
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Blue, FString("Overlapped with: ") + OtherActorName);
	}
}

void AItem::OnSpherEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	FString OtherActorName = OtherActor->GetName();
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red, FString("Ending Overlap with: ") + OtherActorName);
	}
}



