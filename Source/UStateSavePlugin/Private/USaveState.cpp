#include "USaveState.h"

#include "FileManagerGeneric.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

USaveState::USaveState()
{
	SavedClasses = TArray<UClass*>();
}

void USaveState::ClearContents()
{
	SavedClasses.Empty();
	SavedState.Empty();
}

bool USaveState::Save(UWorld * World, const TArray<TSubclassOf<AActor>>& ClassesToSave)
{
	ClearContents();

	for (TSubclassOf<AActor> ClassToSave : ClassesToSave)
	{
		SavedClasses.Add(ClassToSave.Get());
	}
	
	TArray<AActor*> ActorsToSave = GetSavableActorsOfClass(World, SavedClasses);
	for (AActor* FoundActor : ActorsToSave)
	{
		UPrimitiveComponent* RootRefC = Cast<UPrimitiveComponent>(FoundActor->GetRootComponent());
		if (RootRefC != nullptr && RootRefC->Mobility == EComponentMobility::Movable)
		{
			FSavedObjectInfo* ObjectSpawnInfo = new FSavedObjectInfo();
			ObjectSpawnInfo->ActorName = FoundActor->GetFName();
			ObjectSpawnInfo->ActorTransform = FoundActor->GetTransform();
			ObjectSpawnInfo->ActorClass = FoundActor->GetClass();
			ObjectSpawnInfo->ActorData = SaveSerialization(FoundActor);

			SavedState.Emplace(FoundActor->GetName(), ObjectSpawnInfo);
		}
	}
	return true;
}

bool USaveState::Load(UWorld * World)
{
	TMap<FString, FSavedObjectInfo*> CopiedMap = SavedState;
	TArray<AActor*> ActorsToDelete = TArray<AActor*>();
	TArray<AActor*> ActorArray = GetSavableActorsOfClass(World, SavedClasses);

	// Move Objects if they still reside within the Level.
	for (AActor* FoundActor : ActorArray)
	{
		if (SavedState.Find(FoundActor->GetName()))
		{
			FSavedObjectInfo* SavedInfo =  
				*SavedState.Find(FoundActor->GetName());
			if (SavedInfo)
			{
				FoundActor->SetActorTransform(SavedInfo->ActorTransform);

				// Set Velocity etc. to zero.
				UPrimitiveComponent* RefRootC = 
					Cast<UPrimitiveComponent>(FoundActor->GetRootComponent());
				if (RefRootC != nullptr)
				{
					RefRootC->SetPhysicsLinearVelocity(FVector());
					RefRootC->SetPhysicsAngularVelocityInDegrees(FVector());
				}
				// Remove Object which have bene found from List.
				CopiedMap.Remove(FoundActor->GetName());
			}
		}
		else
		{
			if (EComponentMobility::Static != FoundActor->GetRootComponent()->Mobility)
				ActorsToDelete.Add(FoundActor);
		}
	}

	// Remove Unlisted Objects
	for (AActor* ActorToDelete : ActorsToDelete)
	{
		UE_LOG(LogTemp, Error, TEXT("DELETING %s"), *ActorToDelete->GetName());
		ActorToDelete->Destroy();
	}
	
	// Respawn Objects
	TArray<FSavedObjectInfo*> LoadObjectRecords = TArray<FSavedObjectInfo*>();

	CopiedMap.GenerateValueArray(LoadObjectRecords);
	for (int i = 0; LoadObjectRecords.IsValidIndex(i); i++)
	{
		FSavedObjectInfo* ObjectRecord = LoadObjectRecords[i];

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ObjectRecord->ActorName;
		SpawnParameters.OverrideLevel = World->PersistentLevel;
		FTransform TempTransform = FTransform();
		AActor* NewActor = World->SpawnActor(ObjectRecord->ActorClass, 
			&TempTransform, SpawnParameters);

		ApplySerializationActor(ObjectRecord->ActorData, NewActor);
		NewActor->SetActorTransform(ObjectRecord->ActorTransform);
		NewActor->UpdateComponentTransforms();
	}
	
	return true;
}

TArray<uint8> USaveState::SerializeState(int& OutSavedItemAmount) const
{
	TArray<uint8> OutputSerialization = {};
	TArray<FString> ObjectNameArray = {};
	TArray<FSavedObjectInfo*> ObjectInfoArray = {};
	
	FMemoryWriter MemoryWriter(OutputSerialization, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
	
	SavedState.GenerateKeyArray(ObjectNameArray);
	SavedState.GenerateValueArray(ObjectInfoArray);

	for (OutSavedItemAmount = 0; OutSavedItemAmount < SavedState.Num(); OutSavedItemAmount++)
	{
		// Archive << ObjectNameArray[OutSavedItemAmount];
		Archive << ObjectInfoArray[OutSavedItemAmount];
	}

	return OutputSerialization;
}

void USaveState::ApplySerializeOnState(const TArray<uint8> SerializedState, const int& InSavedItemAmount)
{
	TArray<FString> ObjectNameArray = {};
	TArray<FSavedObjectInfo*> ObjectInfoArray = {};
	
	FMemoryReader MemoryReader(SerializedState, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
	if (SavedState.Num() > 0)
	{
		// SavedState was not cleaned ere using it again.
		ensure(false);
		SavedState.Empty();
	}

	for (int Counter = 0; InSavedItemAmount > Counter; Counter++)
	{
		FString ObjectName = FString();
		FSavedObjectInfo* ObjectData = new FSavedObjectInfo();
		
		// Archive << ObjectName;
		Archive << ObjectData;
		ObjectName = ObjectData->ActorName.ToString();

		if (!SavedClasses.Contains(ObjectData->ActorClass))
			SavedClasses.Add(ObjectData->ActorClass);
		UE_LOG(LogTemp, Warning, TEXT("Adding %s"), *ObjectName);

		SavedState.Add(ObjectName, ObjectData);
	}
}


TArray<AActor*> USaveState::GetSavableActorsOfClass(UWorld* InWorld, TArray<UClass*> TRefClasses)
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

TArray<uint8> USaveState::SaveSerialization(AActor* SaveObject)
{
	TArray<uint8> OutputData;
	FMemoryWriter MemoryWriter(OutputData, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
	SaveObject->Serialize(Archive);

	// Components Begin
	TArray<UActorComponent*> ChildComponents;
	SaveObject->GetComponents(ChildComponents);

	for (UActorComponent* CompToSave : ChildComponents)
	{
		CompToSave->Serialize(Archive);
	}
	// Components End

	return OutputData;
}

void USaveState::ApplySerializationActor(UPARAM(ref) TArray<uint8>& ObjectRecord, AActor* ObjectToApplyOn)
{
	FMemoryReader MemoryReader(ObjectRecord, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
	ObjectToApplyOn->Serialize(Archive);

	// Components Begin
	TArray<UActorComponent*> ChildComponents;
	ObjectToApplyOn->GetComponents(ChildComponents);

	for (UActorComponent* CompToLoad : ChildComponents)
	{
		CompToLoad->Serialize(Archive);
	}
	// Components End
}

TArray<UClass*> USaveState::GetSavedClasses()
{
	return SavedClasses;
}
