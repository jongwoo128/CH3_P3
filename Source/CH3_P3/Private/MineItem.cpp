#include "MineItem.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AMineItem::AMineItem() {

	ExplosionDelay = 0.5f;
	ExplosionRadius = 300.0f;
	ExplosionDamage = 30.0f;
	ItemType = "Mine";
	bHasExploded = false;

	ExplosionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollision"));
	ExplosionCollision->InitSphereRadius(ExplosionRadius);
	ExplosionCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ExplosionCollision->SetupAttachment(Scene);

}

void AMineItem::ActivateItem(AActor* Activator) {

	if (bHasExploded) return;

	Super::ActivateItem(Activator);

	GetWorld()->GetTimerManager().SetTimer(
		ExplosionTimerHandle,
		this,
		&AMineItem::Explode,
		ExplosionDelay,
		false);

	bHasExploded = true;
}



void AMineItem::Explode() {

	UParticleSystemComponent* Particle = nullptr;

	if (ExplosionParticle) {
		Particle = UGameplayStatics::SpawnEmitterAtLocation( // 월드 상에서 독립적으로 스폰을 시키는 함수. 즉, 아이템에 붙어서 나오는 것이 아니기 때문에 아이템의 Destroy와 별개 자체적인 라이프사이클이 존재함.
			GetWorld(),
			ExplosionParticle,
			GetActorLocation(),
			GetActorRotation(),
			false // 파티클의 사이클이 끝나면 자동으로 사라진다는 의미의 true를 false로 바꾼 이유:
				  // 반복적으로 도는 형태의 파티클인 경우 자동을 삭제하는 상황에 해당 되지 않으며,
				  // 우리가 구현할 몇 초 후 사라지게하는 등의 세밀한 조정을 필요로 하는 상황들은 자동이 아닌 직접 삭제를 해줘야 한다.
		);
	}

	if (ExplosionSound) {
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			ExplosionSound,
			GetActorLocation()
		);
	}

	TArray<AActor*> OverlappingActors;
	ExplosionCollision->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors) {
		if (Actor && Actor->ActorHasTag("Player")) {
			UGameplayStatics::ApplyDamage(
				Actor,//데미지가 들어갈 대상
				ExplosionDamage, // 어느정도의 데미지를?
				nullptr, // 대미지를 준 대상인데, controller가 따로 있는 대상이 아니라 지뢰이기때문에 그런 대상이 없기에 nullptr로 대체
				this, //데미지를 주는 액터는? 죄라는 이 객체라는 뜻으로 this
				UDamageType::StaticClass() // 주는 데미지의 타입인데, 현재의 가장 기본적인 데미지 타입으로 입력.
				);
		}
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

	DestroyItem();
}