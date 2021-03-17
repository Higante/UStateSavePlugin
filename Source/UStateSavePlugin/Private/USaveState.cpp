#include "USaveState.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

USaveState::USaveState()
{
	SavedClasses = {};
	SavedState = TMap<FString, FSavedObjectInfo>();
}

TMap<FString, FSavedObjectInfo> USaveState::GetSavedState()
{
	return SavedState;
}

void USaveState::ClearContents_Implementation()
{
	SavedClasses.Empty();
	SavedState = TMap<FString, FSavedObjectInfo>();
}

bool USaveState::Save_Implementation(UWorld * World, TArray<UClass*>& ToSave)
{
	// Save Classes saved for later used
	SavedClasses = ToSave;

	for (UClass* RefClass : ToSave)
	{
		TArray<AActor*> OutArray = TArray<AActor*>();
		UGameplayStatics::GetAllActorsOfClass(World, RefClass, OutArray);

		// Alternative?
		for (AActor* FoundActor : OutArray)
		{
			if (Cast<UPrimitiveComponent>(FoundActor->GetRootComponent()) != nullptr && Cast<UPrimitiveComponent>(FoundActor->GetRootComponent())->Mobility == EComponentMobility::Movable)
			{
				SavedState.Add(FoundActor->GetName(), FSavedObjectInfo(FoundActor));
			}
			else if (Cast<USceneComponent>(FoundActor->GetRootComponent()) != nullptr && Cast<USceneComponent>(FoundActor->GetRootComponent())->Mobility == EComponentMobility::Movable)
			{
				SavedState.Add(FoundActor->GetName(), FSavedObjectInfo(FoundActor));
			}
		}
	}

	return true;
}

bool USaveState::Load_Implementation(UWorld * World)
{
	TMap<FString, FSavedObjectInfo> CurrentSavedState = GetSavedState();

	for (UClass* RefClass : SavedClasses)
	{
		// Get the Actors of the Class within the World
		TArray<AActor*> OutActors = TArray<AActor*>();
		UGameplayStatics::GetAllActorsOfClass(World, RefClass, OutActors);

		// Alternative?
		for (AActor* FoundActor : OutActors)
		{
			if (CurrentSavedState.Find(FoundActor->GetName()) != nullptr)
			{
				FSavedObjectInfo* SavedInfo = CurrentSavedState.Find(FoundActor->GetName());

				// Set Saved Infos
				FoundActor->SetActorLocation(SavedInfo->ActorLocation);
				FoundActor->SetActorRotation(SavedInfo->ActorRotation);
				FoundActor->Tags = SavedInfo->Tags;

				// Velocity to 0 (Check if it may throw an error)
				if (Cast<UPrimitiveComponent>(FoundActor->GetRootComponent()) != nullptr)
				{
					Cast<UPrimitiveComponent>(FoundActor->GetRootComponent())->SetPhysicsLinearVelocity(FVector());
					Cast<UPrimitiveComponent>(FoundActor->GetRootComponent())->SetPhysicsAngularVelocityInDegrees(FVector());
				}
				else if (Cast<USceneComponent>(FoundActor->GetRootComponent()) != nullptr)
				{
					Cast<USceneComponent>(FoundActor->GetRootComponent())->ComponentVelocity = FVector();
				}
			}
		}
	}

	return true;
}
