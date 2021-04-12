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
	SavedClasses = {};
	SavedState = TMap<FString, FSavedObjectInfo>();

	ObjectsToDelete = TArray<AActor*>();
	ObjectsToSpawn = TArray<FSavedObjectInfo>();
}

bool USaveState::Save(UWorld * World, TArray<UClass*>& ToSave)
{
	// Reset To Be Saved/Deleted
	ObjectsToDelete = TArray<AActor*>();
	ObjectsToSpawn = TArray<FSavedObjectInfo>();

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

bool USaveState::Load(UWorld * World)
{
	UE_LOG(LogTemp, Display, TEXT("Start to Load SaveState!"));
	// Deal with the Two Arrays needing spawning/deleting
	for (AActor* ToDelete : ObjectsToDelete)
	{
		UE_LOG(LogTemp, Display, TEXT("Start To delete Object!"));
		ToDelete->Destroy();
	}

	for (FSavedObjectInfo ObjectRecord : ObjectsToSpawn)
	{
		UE_LOG(LogTemp, Display, TEXT("Start To Spawn Object!"));
		
		AActor* NewActor = World->SpawnActor(ObjectRecord.ActorClass, &ObjectRecord.ActorLocation, &ObjectRecord.ActorRotation);
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
	}

	return true;
}

void USaveState::OnSpawnChange(AActor * InActor)
{
	/**
	 * TODO: Find a new way to check in ObjectsToSpawn
	 */

	ObjectsToDelete.Add(InActor);
}

void USaveState::OnDeleteChange(AActor * InActor)
{
	if (ObjectsToDelete.Contains(InActor))
	{
		ObjectsToDelete.Remove(InActor);
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Beginning to Archive Destroyed Actor's Data..."));
	FSavedObjectInfo ObjectSpawnInfo = FSavedObjectInfo(InActor);
	FMemoryWriter MemoryWriter(ObjectSpawnInfo.ActorData);
	MemoryWriter.ArIsSaveGame = true;
	InActor->Serialize(MemoryWriter);
	UE_LOG(LogTemp, Display, TEXT("Done!!!"));

	ObjectsToSpawn.Add(ObjectSpawnInfo);
}