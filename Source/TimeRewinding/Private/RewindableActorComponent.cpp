// Fill out your copyright notice in the Description page of Project Settings.


#include "RewindableActorComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values for this component's properties
URewindableActorComponent::URewindableActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void URewindableActorComponent::BeginPlay()
{
	Super::BeginPlay();

	PrepareForRewind();
}

void URewindableActorComponent::PrepareForRewind()
{
	StaticMeshComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();

	if (StaticMeshComponent)
	{
		bWasSimulatingPhysics = StaticMeshComponent->IsSimulatingPhysics();
		LastCollisionType = StaticMeshComponent->GetCollisionEnabled();
	}

	Character = Cast<ACharacter>(GetOwner());

	if (Character && StaticMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[REWIND] Character Found But There Is A Static Mesh Component"));
	}

	RewindableActorComponents.Add(this);
}


void URewindableActorComponent::BeginRewind()
{
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetSimulatePhysics(false);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void URewindableActorComponent::EndRewind()
{
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetSimulatePhysics(bWasSimulatingPhysics);
		StaticMeshComponent->SetCollisionEnabled(LastCollisionType);
	}
}


void URewindableActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	bIsRewinding = false;
	RewindableActorComponents.Remove(this);
}


bool URewindableActorComponent::bIsAbleToAddTransform()
{
	if (RecordedTransformArray.Num() == 0) return true;
	auto lastPosition = RecordedTransformArray[RecordedTransformArray.Num() - 1].GetLocation();
	auto currentPosition = GetOwner()->GetActorTransform().GetLocation();
	auto distance = FVector::Dist(lastPosition, currentPosition);
	return (distance > 5.0f);
}

// Called every frame
void URewindableActorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (!bIsRewinding)
	{
		float Time = GetWorld()->GetTimeSeconds();

		if (Time > RewindTime && bIsAbleToAddTransform())
		{
			RewindTime = Time + REWINDDELAY;
			if (RecordedTransformArray.Num() > MAX_TRANSFORM)
			{
				RecordedTransformArray.RemoveAt(0); // Remove the first element

				// Shift all elements to the left
				for (int i = 0; i < RecordedTransformArray.Num(); i++)
				{
					RecordedTransformArray[i] = RecordedTransformArray[i + 1];
				}

				RecordedTransformArray.RemoveAt(RecordedTransformArray.Num() - 1); // Remove the last element
			}

			// Add the current transform to the array
			RecordedTransformArray.Add(GetOwner()->GetActorTransform());
		}
	}
	else
	{
		if (RecordedTransformArray.Num() > 0)
		{
			if (Character)
			{
				Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
			}

			//lerp between the current transform and the last recorded transform
			auto lastTransform = RecordedTransformArray[RecordedTransformArray.Num() - 1];
			auto currentTransform = GetOwner()->GetActorTransform();

			const float alpha = 0.05f;

			//lerp position
			auto lerpPosition = FMath::Lerp(currentTransform.GetLocation(), lastTransform.GetLocation(), alpha);
			GetOwner()->SetActorLocation(lerpPosition);

			//lerp rotation
			auto lerpRotation = FMath::Lerp(currentTransform.GetRotation().Rotator(),
			                                lastTransform.GetRotation().Rotator(), alpha);
			GetOwner()->SetActorRotation(lerpRotation.Quaternion());

			//lerp scale
			auto lerpScale = FMath::Lerp(currentTransform.GetScale3D(), lastTransform.GetScale3D(), alpha);
			GetOwner()->SetActorScale3D(lerpScale);

			if (lastTransform.Equals(currentTransform, 0.25f))
			{
				RecordedTransformArray.RemoveAt(RecordedTransformArray.Num() - 1);
			}
		}
	}

	// ...
}
