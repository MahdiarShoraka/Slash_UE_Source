#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class AAIController;
class UPawnSensingComponent;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	virtual void Tick(float DeltaTime) override;
	void CheckPatrolTarget();
	void CheckCombatTarget();
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	
protected:
	virtual void BeginPlay() override;

	virtual void Die() override;
	bool InTargetRange(AActor* Target, double Radius);
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual int32 PlayDeathMontage() override;
	virtual void AttackEnd() override;
	
	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(EditAnywhere,  Category = "Combat Properties")
	float DeathLifeSpan = 10.f;
	
	UPROPERTY(EditAnywhere, Category = "Combat Properties")
	double CombatRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Combat Properties")
	double AttackRadius = 175.f;

	UPROPERTY(EditAnywhere, Category = "Combat Properties")
	TSubclassOf<class AWeapon> WeaponClass;
	
private:

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;
	
	/**
	*	Navigation
	*/
	UPROPERTY()
	AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolRadius = 300.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double MinWaitTime = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double MaxWaitTime = 30.f;

	FTimerHandle PatrolTimer;
	
	void PatrolTimerFinished();
	void ClearPatrolTimer();

	/**
	*	AI behavior
	*/
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	
	UPROPERTY(EditAnywhere, Category="Combat Properties")
	float PatrollingSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category="Combat Properties")
	float ChasingSpeed = 300.f;

	/**
	*	Combat
	*/
	FTimerHandle AttackTimer;

	void StartAttackTimer();
	void ClearAttackTimer();
	
	UPROPERTY(EditAnywhere, Category="Combat Properties")
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category="Combat Properties")
	float AttackMax = 2.f;
};
