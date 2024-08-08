#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
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
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

protected:
	virtual void BeginPlay() override;
	virtual void Die() override;
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual void AttackEnd() override;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

private:
	// AI Behavior
	void CheckPatrolTarget();
	void CheckCombatTarget();
	void PatrolTimerFinished();
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	void ClearPatrolTimer();
	void StartAttackTimer();
	void ClearAttackTimer();
	bool InTargetRange(AActor* Target, double Radius);
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
	void SpawnDefaultWeapon();

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);	//Callback for OnPawnSeen in UPawnSensingComponent

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere)
	double CombatRadius = 1000.f;

	UPROPERTY(EditAnywhere)
	double AttackRadius = 175.f;

	UPROPERTY()
	AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 300.f;

	FTimerHandle PatrolTimer;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolMinWaitTime = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolMaxWaitTime = 30.f;

	UPROPERTY(EditAnywhere, Category = "Combat Properties")
	float PatrollingSpeed = 125.f;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat Properties")
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat Properties")
	float AttackMax = 2.f;

	UPROPERTY(EditAnywhere, Category = "Combat Properties")
	float ChasingSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "Combat Properties")
	float DeathLifeSpan = 10.f;

};
