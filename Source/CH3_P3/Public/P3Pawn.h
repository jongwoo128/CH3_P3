
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "P3Pawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCapsuleComponent;
class UFloatingPawnMovement;
class USkeletalMeshComponent;
struct FInputActionValue;

UCLASS()
class CH3_P3_API AP3Pawn : public APawn
{
	GENERATED_BODY()

public:

	AP3Pawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	UCapsuleComponent* CapsuleComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UFloatingPawnMovement* PawnMovementComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* MeshComp;


protected:

	//virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void StartJump(const FInputActionValue& Value);
	void StopJump(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);

	void Jump();
	void StopJumpping();

private:	
	float NormalSpeed;
	float SprintSpeed;
	float SprintSpeedMultiplier;

	float CurrentSpeed;
	float LookSpeed = 2.5f;
	float CurrentPitch = 0.f;

	float GravityScale = 980.0f;
	float ZVelocity = 0.0f;
	float JumpZVelocity = 420.0f;
	bool bIsGrounded = true;


};
