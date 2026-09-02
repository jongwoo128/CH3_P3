

#include "P3Pawn.h"
#include "P3PlayerController.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"


AP3Pawn::AP3Pawn()
{

	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->InitCapsuleSize(36.0f, 91.0f);
	CapsuleComp->SetSimulatePhysics(false);
	SetRootComponent(CapsuleComp);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -CapsuleComp->GetScaledCapsuleHalfHeight()));
	MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	PawnMovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("PawnMovement"));

	NormalSpeed = 600.0f;
	SprintSpeedMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	CurrentSpeed = NormalSpeed;
	PawnMovementComp->MaxSpeed = NormalSpeed;

}
/*
void AP3Pawn::BeginPlay()
{
	Super::BeginPlay();
	
}
*/

void AP3Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ZVelocity -= GravityScale * DeltaTime;
	AddActorWorldOffset(FVector(0.f, 0.f, ZVelocity * DeltaTime), false);

	const float TraceDistance = CapsuleComp->GetScaledCapsuleHalfHeight() + 5.0f;
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0.f, 0.f, TraceDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult Hit;
	bool bHitGround = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, QueryParams);

	if (bHitGround && ZVelocity <= 0.f) {

		bIsGrounded = true;
		ZVelocity = 0.f;

		SetActorLocation(FVector(Start.X, Start.Y,
			Hit.ImpactPoint.Z + CapsuleComp->GetScaledCapsuleHalfHeight()));
	}
	else {
		bIsGrounded = false;
	}
}

void AP3Pawn::Jump() {
	if (bIsGrounded) {
		ZVelocity = JumpZVelocity;
		bIsGrounded = false;
	}
}

void AP3Pawn::StopJumpping() {

}

void AP3Pawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		if (AP3PlayerController* PlayerController = Cast<AP3PlayerController>(GetController())) {
			if (PlayerController->MoveAction) {
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&AP3Pawn::Move
				);
			}

			if (PlayerController->JumpAction) {
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&AP3Pawn::StartJump
				);

				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Completed,
					this,
					&AP3Pawn::StopJump
				);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&AP3Pawn::Look
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&AP3Pawn::StartSprint
				);

				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&AP3Pawn::StopSprint
				);
			}
		}
	}
}

void AP3Pawn::Move(const FInputActionValue& value) {
	if (!Controller) return;

	const FVector2D MoveInput = value.Get<FVector2D>();
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	const FVector Offset = FVector(MoveInput.X, MoveInput.Y, 0.f) * CurrentSpeed * DeltaTime;
	AddActorLocalOffset(Offset, true);
}

void AP3Pawn::StartJump(const FInputActionValue& value) {
	if (value.Get<bool>()) {
		Jump();
	}
}

void AP3Pawn::StopJump(const FInputActionValue& value) {
	if (!value.Get<bool>()) {
		StopJumpping();
	}
}

void AP3Pawn::Look(const FInputActionValue& value) {
	const FVector2D LookInput = value.Get<FVector2D>();


	AddActorLocalRotation(FRotator(0.f, LookInput.X * LookSpeed, 0.f));

	CurrentPitch = FMath::Clamp(CurrentPitch + LookInput.Y * LookSpeed, -80.f, 80.f);

	FRotator NewSpringArmRotation = SpringArmComp->GetRelativeRotation();
	NewSpringArmRotation.Pitch = CurrentPitch;
	SpringArmComp->SetRelativeRotation(NewSpringArmRotation);
}

void AP3Pawn::StartSprint(const FInputActionValue& value)
{
	CurrentSpeed = SprintSpeed;
}

void AP3Pawn::StopSprint(const FInputActionValue& value)
{
	CurrentSpeed = NormalSpeed;
}