// Fill out your copyright notice in the Description page of Project Settings.
// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GroomComponent.h"
#include "Items/Item.h"
#include "Items/Weapon.h"
#include "Enemies/Enemy.h"
#include "Animation/AnimMontage.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "HUD/HealthBarComponent.h"
#include "Components/AttributeComponent.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/Heal.h"
#include "Kismet/KismetMathLibrary.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// stamina system
	if (Attributes && SlashOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}

	// Lock On system
	if (bIsLockedOn)
	{
		if (FindLockOnTarget() != LockedOnTarget)
		{
			UnlockFromTarget(); // Unlock from the old target
			LockOn(); // Lock on the new target
		}
	}
	RotateToLockedTarget(DeltaTime);
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Triggered, this, &ASlashCharacter::LockOn);
		EnhancedInputComponent->BindAction(UnlockAction, ETriggerEvent::Triggered, this, &ASlashCharacter::UnlockFromTarget);
	}
}

void ASlashCharacter::Jump()
{
	if (ActionState == EActionState::EAS_Unoccupied)
	{
		Super::Jump();
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Attributes->GetHealthPercent() > 0)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSouls(Attributes->GetSouls());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGold(Attributes->GetGold());
	}
}

void ASlashCharacter::Heal(AHeal* Heal)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddHealth(Heal->GetHealAmount());
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
	InitializeSlashOverlay(PlayerController);
	Tags.Add(FName("EngageableTarget"));
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->Destroy();
		}
		EquipWeapon(OverlappingWeapon);
	}
	else if (CanDisarm())
	{
		Disarm();
	}
	else if (CanArm())
	{
		Arm();
	}
}

void ASlashCharacter::Dodge()
{
	if (ActionState != EActionState::EAS_Unoccupied || !HasStaminaForDodge()) return;
	if (ActionState != EActionState::EAS_Unoccupied || !HasStaminaForDodge()) return;

	ActionState = EActionState::EAS_Dodge;
	PlayDodgeMontage();
	if (Attributes && SlashOverlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::LockOn()
{
	AEnemy* Target = Cast<AEnemy>(FindLockOnTarget());
	if (Target)
	{
		//UE_LOG(LogTemp, Warning, TEXT("The lockedOn target is: %s"), *(Target->GetActorNameOrLabel()));
		UHealthBarComponent* EnemyHealthBar = Target->GetHealthBarWidget();
		EnemyHealthBar->SetVisibility(true);
		LockedOnTarget = Target;
		bIsLockedOn = true;
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}
	else
	{
		UnlockFromTarget();
	}
}

void ASlashCharacter::UnlockFromTarget()
{
	AEnemy* Target = Cast<AEnemy>(LockedOnTarget);
	if (Target)
	{
		UHealthBarComponent* EnemyHealthBar = Target->GetHealthBarWidget();
		EnemyHealthBar->SetVisibility(false);
	}
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	LockedOnTarget = nullptr;
	bIsLockedOn = false;
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	EquippedWeapon = Weapon;
	OverlappingItem = nullptr;
}

void ASlashCharacter::Attack()
{
	Super::Attack();

	if (CanAttack())
	{
		ActionState = EActionState::EAS_Attacking;
		PlayAttackMontage();
	}
}

void ASlashCharacter::AttackEnd()
{
	Super::AttackEnd();
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::HasStaminaForDodge()
{
	return Attributes && Attributes->GetCurrentStamina() >= Attributes->GetDodgeCost();
}

void ASlashCharacter::Die()
{
	Super::Die();
	ActionState = EActionState::EAS_Dead;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

bool ASlashCharacter::CanArm() const
{
	return ActionState == EActionState::EAS_Unoccupied
		&& CharacterState == ECharacterState::ECS_Unequipped
		&& EquippedWeapon;
}

bool ASlashCharacter::CanDisarm() const
{
	return ActionState == EActionState::EAS_Unoccupied
		&& CharacterState != ECharacterState::ECS_Unequipped
		&& EquippedWeapon;
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage("Unequip");
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage("Equip");
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

ABaseCharacter* ASlashCharacter::FindLockOnTarget()
{
	FVector PlayerLocation = GetActorLocation();

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHasHit = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		PlayerLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldDynamic),
		FCollisionShape::MakeSphere(LockOnRadius),
		QueryParams
	);

	ABaseCharacter* NearestTarget = nullptr;
	float NearestDistance = FLT_MAX;

	if (bHasHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* OverlappedActor = Result.GetActor();

			if (OverlappedActor && OverlappedActor != this && !OverlappedActor->ActorHasTag(FName("Dead")))
			{
				float DistanceToTarget = FVector::Dist(PlayerLocation, OverlappedActor->GetActorLocation());

				// Check if this actor is the closest one so far
				if (DistanceToTarget < NearestDistance)
				{
					NearestTarget = Cast<ABaseCharacter>(OverlappedActor);
					if (NearestTarget)
					{
						NearestDistance = DistanceToTarget;
					}
				}
			}
		}
	}

	return NearestTarget;
}

void ASlashCharacter::RotateToLockedTarget(float DeltaTime)
{
	if (LockedOnTarget == nullptr || !bIsLockedOn)
	{
		UnlockFromTarget();
		return;
	}
	FVector PlayerLocation = GetActorLocation();
	FVector LockedTargetLocation = LockedOnTarget->GetActorLocation();

	if (ActionState != EActionState::EAS_Dodge && !LockedOnTarget->ActorHasTag("Dead"))
	{
		// character's rotation
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLocation, LockedTargetLocation);
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), LookAtRotation, DeltaTime, InterpSpeed);
		SetActorRotation(NewRotation);

		// camera's rotation
		FRotator NewControlRotation = GetControlRotation();
		NewControlRotation.Yaw = FMath::RInterpTo(GetControlRotation(), LookAtRotation, DeltaTime, CameraInterpSpeed).Yaw;
		GetController()->SetControlRotation(NewControlRotation);
	}
	else if (LockedOnTarget->ActorHasTag("Dead"))
	{
		UnlockFromTarget();
	}
}

void ASlashCharacter::InitializeSlashOverlay(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();
			if (SlashOverlay && Attributes)
			{
				SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
			}
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}
