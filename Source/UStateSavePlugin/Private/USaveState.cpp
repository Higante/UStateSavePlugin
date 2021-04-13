#include "USaveState.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

USaveState::USaveState()
{
	SavedClasses = {};
	SavedState = TMap<FString, FSavedObjectInfo>();
	ObjectsToDelete = TArray<AActor*>();
	ObjectsToSpawn = TArray<FSavedObjectInfo>();
}

void USaveState::ClearContents()
{
	SavedClasses.Empty();
	SavedState.Empty();
	ObjectsToDelete.Empty();
	ObjectsToSpawn.Empty();
}

bool USaveState::Save(UWorld * World, TArray<UClass*>& ToSave)
{
	ClearContents();
	SavedClasses = ToSave;

	for (UClass* RefClass : ToSave)
	{
		TArray<AActor*> OutArray = TArray<AActor*>();
		UGameplayStatics::GetAllActorsOfClass(World, RefClass, OutArray);

		for (AActor* FoundActor : OutArray)
		{
			if (Cast<UPrimitiveComponent>(FoundActor->GetRootComponent()) != nullptr && Cast<UPrimitiveComponent>(FoundActor->GetRootComponent())->Mobility == EComponentMobility::Movable)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s: Added %s to SavedState. PrimitiveComponent"), TEXT(__FUNCTION__), *FoundActor->GetName());
				SavedState.Add(FoundActor->GetName(), FSavedObjectInfo(FoundActor));
			}
			else if (Cast<USceneComponent>(FoundActor->GetRootComponent()) != nullptr && Cast<USceneComponent>(FoundActor->GetRootComponent())->Mobility == EComponentMobility::Movable)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s: Added %s to SavedState. SceneComponent"), TEXT(__FUNCTION__), *FoundActor->GetName());
				SavedState.Add(FoundActor->GetName(), FSavedObjectInfo(FoundActor));
			}
		}
	}

	return true;
}

bool USaveState::Load(UWorld * World)
{
	UE_LOG(LogTemp, Display, TEXT("Start to Load SaveState!"));
	for (AActor* ToDelete : ObjectsToDelete)
	{
		UE_LOG(LogTemp, Display, TEXT("Start To delete Object!"));
		ToDelete->Destroy();
	}

	for (FSavedObjectInfo ObjectRecord : ObjectsToSpawn)
	{
		check(ObjectRecord.ActorClass != NULL);

		AActor* NewActor = World->SpawnActor(ObjectRecord.ActorClass, &ObjectRecord.ActorLocation, &ObjectRecord.ActorRotation);
		UE_LOG(LogTemp, Warning, TEXT("%s: Class %s Actor spawned."), TEXT(__FUNCTION__), *ObjectRecord.ActorClass->GetName());
		
		FMemoryReader Reader = FMemoryReader(ObjectRecord.ActorData);
		NewActor->Serialize(Reader);
		UE_LOG(LogTemp, Display, TEXT("Spawning Object Complete!"));
	}

	// Clear both Arrays
	ObjectsToDelete = TArray<AActor*>();
	ObjectsToSpawn = TArray<FSavedObjectInfo>();

	// TODO: Search for a better alternative to this bit.
	for (UClass* RefClass : SavedClasses)
	{
		// Get the Actors of the Class within the World
		TArray<AActor*> OutActors = TArray<AActor*>();
		UGameplayStatics::GetAllActorsOfClass(World, RefClass, OutActors);

		for (AActor* FoundActor : OutActors)
		{
			if (SavedState.Find(FoundActor->GetName()) != nullptr)
			{
				FSavedObjectInfo* SavedInfo = SavedState.Find(FoundActor->GetName());
				if (SavedInfo != nullptr)
				{
					// Set Saved Infos
					FoundActor->SetActorLocation(SavedInfo->ActorLocation);
					FoundActor->SetActorRotation(SavedInfo->ActorRotation);

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
	}

	return true;
}

void USaveState::OnSpawnChange(AActor * InActor)
{
	/**
	 * TODO: Find a new way to check in ObjectsToSpawn
	 */

	ObjectsToDelete.Add(InActor);
	UE_LOG(LogTemp, Display, TEXT("%s: Adding to amount of items to delete later! New Amount: %d"), TEXT(__FUNCTION__), ObjectsToDelete.Num());
}

void USaveState::OnDeleteChange(AActor * InActor)
{
	if (ObjectsToDelete.Contains(InActor))
	{
		ObjectsToDelete.Remove(InActor);
		UE_LOG(LogTemp, Display, TEXT("%s: Removing Data from List of to be deleted. New Amount = %d"), TEXT(__FUNCTION__), ObjectsToDelete.Num());
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("%s: Saving Data of destroyed Actor = %s"), TEXT(__FUNCTION__), *InActor->GetName());

	FSavedObjectInfo ObjectSpawnInfo = FSavedObjectInfo(InActor);
	FMemoryWriter MemoryWriter(ObjectSpawnInfo.ActorData);
	MemoryWriter.ArIsSaveGame = true;
	InActor->Serialize(MemoryWriter);

	ObjectsToSpawn.Add(ObjectSpawnInfo);

	UE_LOG(LogTemp, Display, TEXT("%s: Saving Data Complete! New Amount of Items to Spawn = %d"), TEXT(__FUNCTION__), ObjectsToSpawn.Num());
}