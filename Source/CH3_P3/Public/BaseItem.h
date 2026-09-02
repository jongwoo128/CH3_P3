
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemInterface.h"
#include "BaseItem.generated.h"


class USphereComponent;

UCLASS()
class CH3_P3_API ABaseItem : public AActor, public IItemInterface
{
	GENERATED_BODY()
	
public:	

	ABaseItem();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Component")
	USceneComponent* Scene;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Component")
	USphereComponent* Collision;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Component")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effects")
	UParticleSystem* PickupParticle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effects")
	USoundBase* PickupSound; //언리얼 엔진에 있는 모든 사운드 에셋이 공유하는 가장 기본적인 클래스(Sound Wave와 Sound Cue 또한 이것을 상속받아 구현됨.)
	//sound Cue: 설정들이 많이 들어간 사운드 파일로, 여러개의 사운드를 조합하거나 페이드, 블렌딩 등의 동적 처리가 들어가는 동적 사운드. (거리에 따른 효과를 준다거나 피치를 조정하거나 믹싱을 하는 등의 효과)
	//Sound Wave: 간단한 단일 오디오 파일


	virtual void OnItemOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;
	virtual void OnItemEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex) override;
	virtual void ActivateItem(AActor* Activator) override;
	virtual FName GetItemType() const override;

	virtual void DestroyItem();


};
