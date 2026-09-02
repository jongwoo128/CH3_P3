#include "BaseItem.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ABaseItem::ABaseItem()
{

	PrimaryActorTick.bCanEverTick = false;


	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetCollisionProfileName(TEXT("OvarlapAllDynamic"));
	Collision->SetupAttachment(Scene);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(Collision);

	Collision->OnComponentBeginOverlap.AddDynamic(this, &ABaseItem::OnItemOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &ABaseItem::OnItemEndOverlap);
}

void ABaseItem::OnItemOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->ActorHasTag("Player")) {
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Overlap!!")));
		ActivateItem(OtherActor);
	}
}
void ABaseItem::OnItemEndOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex) {

}
void ABaseItem::ActivateItem(AActor* Activator) {

	UParticleSystemComponent* Particle = nullptr;

	if (PickupParticle) {
		Particle = UGameplayStatics::SpawnEmitterAtLocation( // 지정된 위치나 회전에 따라서 파티클 효과를 생성하는 함수
			GetWorld(), // 현재 게임이 실행되고 있는 월드 객체를 가져오라.(파티클을 생성한 월드 정보가 있어야 하기 때문)
			PickupParticle, // 지정한 파티클 에셋
			GetActorLocation(), // 이 액터의 월드 위치
			GetActorRotation(), // 이 액터의 회전 위치
			false // bool형의 AutoDestroy 값을 넣어주는 것으로 파티클 효과가 끝난 다음 메모리에서 자동으로 제거되도록 설정해주는 것.
		);
	}

	if (PickupSound) {
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			PickupSound,
			GetActorLocation()
		);
	}

	if (Particle) {
		FTimerHandle DestroyParticleTimerHandle;
		TWeakObjectPtr<UParticleSystemComponent> WeakParticle(Particle);

		GetWorld()->GetTimerManager().SetTimer(
			DestroyParticleTimerHandle,
			// Lambda형태의 함수. 익명 함수라고도 불리는 함수이며,
			// []를 캡처리스트라고 부르고 이 람다를 실행할 때 Particle이라는 변수를 바깥 스코프에서 값을 가져다가 사용할 수 있도록 함.
			// 함수를 직접 구현하는 것은 번거롭고 간단하게 명령을 내려야 할 때, 함수같이 사용하고 싶을 때 람다식의 형태를 많이 사용함.
			[WeakParticle]() {
				if (WeakParticle.IsValid()) {
					WeakParticle->DestroyComponent();
				}
			},
			2.0f,
			false
		);
	}
}
FName ABaseItem::GetItemType() const {
	return ItemType;
}

void ABaseItem::DestroyItem() {
	Destroy();
}