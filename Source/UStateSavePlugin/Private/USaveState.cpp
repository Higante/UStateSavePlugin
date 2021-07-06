#include "USaveState.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/BufferArchive.h"

USaveState::USaveState()
{
	SavedClasses = TArray<UClass*>();
	SavedState = TMap<FString, TSharedPtr<FSavedObjectInfo>>();
	ObjectsToDelete = TArray<AActor*>();
}

void USaveState::ClearContents()
{
	SavedClasses.Empty();
	SavedState.Empty();
	ObjectsToDelete.Empty();
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

			TSharedPtr<FSavedObjectInfo> ObjectSpawnInfo = MakeShareable<FSavedObjectInfo>(new FSavedObjectInfo(FoundActor));
			FMemoryWriter MemoryWriter(ObjectSpawnInfo->ActorData, true);
			FSaveStateGameArchive Archive(MemoryWriter);
			MemoryWriter.Seek(0);
			FoundActor->Serialize(Archive);

			SavedState.Emplace(FoundActor->GetName(), ObjectSpawnInfo);
		}
	}

	//SaveToFile(FileToSaveOn);

	return true;
}

bool USaveState::Load(UWorld * World)
{
	// Delete Objects which have no reason to stay any longer.
	for (int i = 0; ObjectsToDelete.IsValidIndex(i); i++)
	{
		if (ObjectsToDelete[i])
			ObjectsToDelete[i]->Destroy();
	}

	// Clear Array
	ObjectsToDelete = TArray<AActor*>();

	TMap<FString, TSharedPtr<FSavedObjectInfo>> CopiedMap = SavedState;
	for (AActor* FoundActor : GetEligibleActors(World, SavedClasses))
	{
		if (SavedState.Find(FoundActor->GetName()))
		{
			TSharedPtr<FSavedObjectInfo> SavedInfo =  
				*SavedState.Find(FoundActor->GetName());
			if (SavedInfo)
			{
				FoundActor->SetActorTransform(SavedInfo->ActorTransform);
				ApplySerilization(SavedInfo, FoundActor);

				// Set Velocity etc. to zero.
				UPrimitiveComponent* RefRootC = 
					Cast<UPrimitiveComponent>(FoundActor->GetRootComponent());
				if (RefRootC != nullptr)
				{
					RefRootC->SetPhysicsLinearVelocity(FVector());
					RefRootC->SetPhysicsAngularVelocityInDegrees(FVector());
				}
				
				CopiedMap.Remove(FoundActor->GetName());
			}
		}
	}
	
	TArray<TSharedPtr<FSavedObjectInfo>> TempArray = 
		TArray<TSharedPtr<FSavedObjectInfo>>();
	CopiedMap.GenerateValueArray(TempArray);
	for (int i = 0; TempArray.IsValidIndex(i); i++)
	{
		// Create the new actor
		TSharedPtr<FSavedObjectInfo> ObjectRecord = TempArray[i];
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = FName(*ObjectRecord->ActorName);

		FTransform TempTransform = FTransform();
		AActor* NewActor = World->SpawnActor(ObjectRecord->ActorClass, 
			&TempTransform, SpawnParameters);
		NewActor->SetActorLabel(*ObjectRecord->ActorName);
		NewActor->SetActorTransform(ObjectRecord->ActorTransform);

		UPrimitiveComponent* RefRootC = 
			Cast<UPrimitiveComponent>(NewActor->GetRootComponent());
		if (RefRootC)
			RefRootC->SetMobility(EComponentMobility::Movable);
		
		if (ObjectRecord->ActorStaticMeshRef)
			Cast<AStaticMeshActor>(NewActor)->GetStaticMeshComponent()->
				SetStaticMesh(ObjectRecord->ActorStaticMeshRef);

		// Apply Serialization.
		ApplySerilization(ObjectRecord, NewActor);
	}

	return true;
}

void USaveState::OnSpawnChange(AActor * InActor)
{
	ObjectsToDelete.Emplace(InActor);
}

TArray<AActor*> USaveState::GetEligibleActors(UWorld* InWorld, TArray<UClass*> TRefClasses)
{
	check(InWorld);
	TArray<AActor*> OutArray = TArray<AActor*>();
	
	for (UClass* RefClass : TRefClasses)
	{
		TArray<AActor*> AddToArray = TArray<AActor*>();
		UGameplayStatics::GetAllActorsOfClass(InWorld, RefClass, AddToArray);
		OutArray.Append(AddToArray);
	}

	return OutArray;
}

void USaveState::ApplySerilization(TSharedPtr<FSavedObjectInfo> ActorProxy, AActor* RefActor)
{
	UE_LOG(LogTemp, Warning, TEXT("%s: Loading %d amount from the Byte Array."), TEXT(__FUNCTION__), ActorProxy->ActorData.Num());
	FMemoryReader Reader(ActorProxy->ActorData, true);
	FSaveStateGameArchive Archive(Reader);
	RefActor->Serialize(Archive);
}

void USaveState::SaveToFile(const FString FilePath)
{
	TArray<uint8> SaveBytesStream = TArray<uint8>();

	FMemoryWriter MemoryWriter(SaveBytesStream, true);
	//MemoryWriter.Seek(0);
	FArchive Archive(MemoryWriter);
	FBufferArchive BinaryData;
	for (TPair<FString,TSharedPtr<FSavedObjectInfo>> ToSave : SavedState)
	{
		//BinaryData << ToSave.Get<1>();
		FFileHelper::SaveArrayToFile(BinaryData, *FilePath);
	}
}

bool USaveState::LoadBytesFromFile(TArray<uint8>& OutBytes, const FString FilePath)
{
	return FFileHelper::LoadFileToArray(OutBytes, *FilePath);
}