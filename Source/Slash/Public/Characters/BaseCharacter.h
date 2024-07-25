#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

protected:
	virtual void BeginPlay() override;

	virtual void Attack();
	virtual bool CanAttack();
	virtual void Die();
	virtual void HandleDamage(float DamageAmount);
	void PlayMontageSection(UAnimMontage* AnimMontage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* AnimMontage, const TArray<FName>& SectionNames);
	virtual int32 PlayAttackMontage();
	virtual int32 PlayDeathMontage();
	void DisableCapsule();
	
	bool IsAlive();
	void PlayHitReactMontage(const FName& SectionName);
	void DirectionalHitReact(const FVector& ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	AWeapon* EquippedWeapon;
	
	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(EditAnywhere, Category = "Montages")
	TArray<FName> AttackMontageSections;

	UPROPERTY(EditAnywhere, Category = "Montages")
	TArray<FName> DeathMontageSections;

	/*
	 * Animation Montages
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* DeathMontage;

private:
	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "VFXs")
	UParticleSystem* HitParticles;
};
