// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RewindableActorComponent.generated.h"

#define		REWINDDELAY		(0.75f)

#define		MAX_TRANSFORM	(300)

class UStaticMeshComponent;
class ACharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TIMEREWINDING_API URewindableActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URewindableActorComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	inline static TArray<URewindableActorComponent*> RewindableActorComponents;
	inline static bool bIsRewinding = false;

	/** We dont need to store position data at every frame */
	float RewindTime = 0.0f;

	bool bIsAbleToAddTransform();

	UStaticMeshComponent* StaticMeshComponent;
	bool bWasSimulatingPhysics = false;
	ACharacter* Character;
	
	ECollisionEnabled::Type LastCollisionType;
	
	
	/** Stroing Transform Data */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rewindable Actor Component")
	TArray<FTransform> RecordedTransformArray;

	void BeginRewind();
	void EndRewind();
	void PrepareForRewind();
};
