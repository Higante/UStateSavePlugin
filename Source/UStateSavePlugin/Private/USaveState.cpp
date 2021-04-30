#include "USaveState.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

USaveState::USaveState()
{
	SavedClasses = TArray<UClass*>();
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
	TArray<AActor*> ActorsToSave = GetEligibleActors(World, SavedClasses);
	for (AActor* FoundActor : ActorsToSave)
	{
		UPrimitiveComponent* RootRefC = Cast<UPrimitiveComponent>(FoundActor->GetRootComponent());
		if (RootRefC != nullptr && RootRefC->Mobility == EComponentMobility::Movable)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Added %s to SavedState."),
				TEXT(__FUNCTION__), *FoundActor->GetName());
			SavedState.Emplace(FoundActor->GetName(), FSavedObjectInfo(FoundActor));
		}
	}

	return true;
}

bool USaveState::Load(UWorld * World)
{
	for (AActor* ToDelete : ObjectsToDelete)
		ToDelete->Destroy();

	for (FSavedObjectInfo ObjectRecord : ObjectsToSpawn)
		LoadSerilization(World, ObjectRecord);

	// Clear both Arrays
	ObjectsToDelete = TArray<AActor*>();
	ObjectsToSpawn = TArray<FSavedObjectInfo>();

	for (AActor* FoundActor : GetEligibleActors(World, SavedClasses))
	{
		FSavedObjectInfo* SavedInfo = SavedState.Find(FoundActor->GetName());
		if (SavedInfo == nullptr)
			return false;
		
		FoundActor->SetActorTransform(SavedInfo->ActorTransform);

		UPrimitiveComponent* RefRootC = Cast<UPrimitiveComponent>(FoundActor->GetRootComponent());
		if (RefRootC != nullptr)
		{
			RefRootC->SetPhysicsLinearVelocity(FVector());
			RefRootC->SetPhysicsAngularVelocityInDegrees(FVector());
		}
	}
	return true;
}

void USaveState::OnSpawnChange(AActor * InActor)
{
	if (ObjectsToSpawn.ContainsByPredicate([&](FSavedObjectInfo Info){return Info.ActorName == InActor->GetName();}))
	{
		UE_LOG(LogTemp, Display, TEXT("%s: %s needed to be spawned"), TEXT(__FUNCTION__), *InActor->GetName());
		return;
	}

	ObjectsToDelete.Emplace(InActor);
}

void USaveState::OnDeleteChange(AActor * InActor)
{
	if (ObjectsToDelete.Contains(InActor))
	{
		UE_LOG(LogTemp, Display, TEXT("%s: %s removing from ListToDelete."), 
			TEXT(__FUNCTION__), *InActor->GetName());
		ObjectsToDelete.Remove(InActor);
		return;
	}
	
	FSavedObjectInfo ObjectSpawnInfo = FSavedObjectInfo(InActor);

	// Workaround StaticMesh
	if (Cast<AStaticMeshActor>(InActor) != nullptr)
		ObjectSpawnInfo.ActorStaticMeshRef = Cast<AStaticMeshActor>(InActor)->GetStaticMeshComponent()->GetStaticMesh();

	FMemoryWriter MemoryWriter(ObjectSpawnInfo.ActorData, true);
	FSaveStateGameArchive Archive(MemoryWriter);
	InActor->Serialize(Archive);

	ObjectsToSpawn.Emplace(ObjectSpawnInfo);
	UE_LOG(LogTemp, Display, TEXT("Some Bullshit"));
}

TArray<AActor*> USaveState::GetEligibleActors(UWorld* InWorld, TArray<UClass*> TRefClasses)
{
	TArray<AActor*> OutArray = TArray<AActor*>();
	
	for (UClass* RefClass : TRefClasses)
	{
		TArray<AActor*> AddToArray = TArray<AActor*>();
		UGameplayStatics::GetAllActorsOfClass(InWorld, RefClass, AddToArray);
		OutArray.Append(AddToArray);
	}

	return OutArray;
}

void USaveState::LoadSerilization(UWorld* InWorld, FSavedObjectInfo ObjectRecord)
{
	check(InWorld);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = FName(*ObjectRecord.ActorName);

	FTransform TempTransform = FTransform();
	AActor* NewActor = InWorld->SpawnActor(ObjectRecord.ActorClass, 
		&TempTransform, SpawnParameters);

	UPrimitiveComponent* RefRootC = Cast<UPrimitiveComponent>(NewActor->GetRootComponent());
	if (RefRootC != nullptr)
		RefRootC->SetMobility(EComponentMobility::Movable);

	NewActor->SetActorTransform(ObjectRecord.ActorTransform);
	if (ObjectRecord.ActorStaticMeshRef != nullptr)
		Cast<AStaticMeshActor>(NewActor)->GetStaticMeshComponent()->
			SetStaticMesh(ObjectRecord.ActorStaticMeshRef);

	FMemoryReader Reader(ObjectRecord.ActorData, true);
	FSaveStateGameArchive Archive(Reader);
	NewActor->Serialize(Archive);

	Reader.FlushCache();
	Reader.Close();
}