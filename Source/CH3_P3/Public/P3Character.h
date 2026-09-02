
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "P3Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
struct FInputActionValue;

UCLASS()
class CH3_P3_API AP3Character : public ACharacter
{
	GENERATED_BODY()

public:

	AP3Character();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* OverheadWidget;

	UFUNCTION(BlueprintPure, Category = "health")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "health")
	void AddHealth(float Amount);
	UFUNCTION(BlueprintPure, Category = "health")
	float GetMaxHealth() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
	float Health;

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent, // 데미지의 유형 등을 추가적으로 확장해서 구현할 경우의 정보값을 받기위해.(불 속성, 물 속성 등..)
		AController* EventInstigator, // 데미지를 누가 입혔는지(상대 플레이어, 몬스터 등..)
		AActor* DamageCauser) override; // 하지만 우리가 만드는 지뢰(오브젝트)의 데미지는 여기서 정보를 다룸.

	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void StartJump(const FInputActionValue& value);
	UFUNCTION()
	void StopJump(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);

	void OnDeath();

	void UpdateOverheadHP();

private:
	float NormalSpeed;
	float SprintSpeedMultiplier;
	float SprintSpeed;
};
