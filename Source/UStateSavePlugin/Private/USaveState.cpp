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

bool USaveState::Save(UWorld * World, TArray<UClass*>& ClassesToSave)
{
	ClearContents();
	SavedClasses = ClassesToSave;

	TArray<AActor*> ActorsToSave = GetEligibleActorsToSave(World, SavedClasses);
	for (AActor* FoundActor : ActorsToSave)
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

	return true;
}

bool USaveState::Load(UWorld * World)
{
	for (AActor* ToDelete : ObjectsToDelete)
	{
		UE_LOG(LogTemp, Display, TEXT("Start To delete Object!"));
		ToDelete->Destroy();
	}

	for (FSavedObjectInfo ObjectRecord : ObjectsToSpawn)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = FName(*ObjectRecord.ActorName);
		FTransform TempTransform = ObjectRecord.ActorTransform;

		AActor* NewActor = World->SpawnActor(ObjectRecord.ActorClass, 
			&TempTransform, SpawnParameters);
		UE_LOG(LogTemp, Warning, TEXT("%s: Class %s Actor spawned."), 
			TEXT(__FUNCTION__), *ObjectRecord.ActorClass->GetName());
		
		FMemoryReader Reader(ObjectRecord.ActorData, true);
		FSaveStateGameArchive Archive(Reader);
		NewActor->Serialize(Archive);
		UE_LOG(LogTemp, Display, TEXT("Spawning Object Complete!"));
	}

	// Clear both Arrays
	ObjectsToDelete = TArray<AActor*>();
	ObjectsToSpawn = TArray<FSavedObjectInfo>();

	TArray<AActor*> PotentialLoad = GetEligibleActorsToSave(World, SavedClasses);
	for (AActor* FoundActor : PotentialLoad)
	{
		if (SavedState.Find(FoundActor->GetName()) != nullptr)
		{
			FSavedObjectInfo* SavedInfo = SavedState.Find(FoundActor->GetName());
			if (SavedInfo != nullptr)
			{
				// Set Saved Infos
				FoundActor->SetActorTransform(SavedInfo->ActorTransform);

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
		return;
	}
	
	FSavedObjectInfo ObjectSpawnInfo = FSavedObjectInfo(InActor);
	FMemoryWriter MemoryWriter(ObjectSpawnInfo.ActorData, true);
	FSaveStateGameArchive Archive(MemoryWriter);
	InActor->Serialize(Archive);

	this->ObjectsToSpawn.Add(ObjectSpawnInfo);
}

TArray<AActor*> USaveState::GetEligibleActorsToSave(UWorld* InWorld, TArray<UClass*> TRefClasses)
{
	TArray<AActor*> OutArray = {};
	
	for (UClass* RefClass : TRefClasses)
	{
		TArray<AActor*> AddToArray = {};
		UGameplayStatics::GetAllActorsOfClass(InWorld, RefClass, AddToArray);
		OutArray.Append(AddToArray);
	}

	return OutArray;
}