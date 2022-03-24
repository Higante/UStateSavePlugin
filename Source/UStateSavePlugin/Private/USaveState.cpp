#include "USaveState.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "FileManagerGeneric.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "SkeletalMeshTypes.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

void USaveState::ClearContents()
{
	SavedClasses.Empty();
	SavedState.Empty();
}

bool USaveState::SaveFromWorld(UWorld * InWorld, const TArray<TSubclassOf<AActor>>& InClasses)
{
	ClearContents();

	for (TSubclassOf<AActor> ClassToSave : InClasses)
	{
		SavedClasses.Add(ClassToSave.Get());
	}
	
	TArray<AActor*> ActorsToSave = GetActorsOfSavedClasses(InWorld, SavedClasses);
	for (AActor* FoundActor : ActorsToSave)
	{
		UPrimitiveComponent* RootRefC = Cast<UPrimitiveComponent>(FoundActor->GetRootComponent());
		if (RootRefC != nullptr && (RootRefC->Mobility == EComponentMobility::Movable || RootRefC->Mobility == EComponentMobility::Stationary))
		{
			FSavedObjectInfo* ObjectSpawnInfo = new FSavedObjectInfo();
			ObjectSpawnInfo->ActorName = FoundActor->GetFName();
			ObjectSpawnInfo->ActorTransform = FoundActor->GetActorTransform();
			ObjectSpawnInfo->ActorClass = FoundActor->GetClass();
			ObjectSpawnInfo->bIsSimulatingPhysics = RootRefC->IsSimulatingPhysics();
			ObjectSpawnInfo->ActorData = SerializeActor(FoundActor);

			SavedState.Emplace(FoundActor->GetName(), ObjectSpawnInfo);
		}
	}
	return true;
}

TArray<AActor*> USaveState::LoadOntoWorld(UWorld * InWorld)
{
	TMap<FString, FSavedObjectInfo*> CopiedMap = SavedState;
	TSet<AActor*> ActorsToDelete = TSet<AActor*>();
	TArray<AActor*> ActorArray = GetActorsOfSavedClasses(InWorld, SavedClasses);
	TArray<AActor*> OutActorArray = TArray<AActor*>();

	// Move Objects if they still reside within the Level.
	for (AActor* FoundActor : ActorArray)
	{
		if (FSavedObjectInfo** SavedObjectInfo = SavedState.Find(FoundActor->GetName()))
		{
			FSavedObjectInfo* SavedInfo = *SavedObjectInfo;
			FoundActor->SetActorRelativeTransform(SavedInfo->ActorTransform);
			FName CollisionProfile = "";
			
			if (UPrimitiveComponent* RefRootC = Cast<UPrimitiveComponent>(FoundActor->GetRootComponent()))
			{
				RefRootC->SetWorldTransform(SavedInfo->ActorTransform);
				CollisionProfile = RefRootC->GetCollisionProfileName();
			}

			/** Workaround with Physics and DestructibleMeshes. */
			if (CollisionProfile != FName("Destructible"))
			{
				CopiedMap.Remove(FoundActor->GetName());
			}
		}
		else
		{
			if (EComponentMobility::Static != FoundActor->GetRootComponent()->Mobility)
			{
				ActorsToDelete.Add(FoundActor);
			}
		}
	}

	// Remove Unlisted Objects
	for (AActor* ActorToDelete : ActorsToDelete)
	{
		UE_LOG(LogTemp, Error, TEXT("DELETING %s"), *ActorToDelete->GetName());
		ActorToDelete->Destroy();
	}
	
	// Respawn Objects
	TArray<FSavedObjectInfo*> SaveStateObjectInfo = TArray<FSavedObjectInfo*>();
	CopiedMap.GenerateValueArray(SaveStateObjectInfo);
	for (int i = 0; SaveStateObjectInfo.IsValidIndex(i); i++)
	{
		// Spawn Actors and then update their Actor Transform.
		FSavedObjectInfo* ObjectRecord = SaveStateObjectInfo[i];

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ObjectRecord->ActorName;
		SpawnParameters.OverrideLevel = InWorld->PersistentLevel;
		FTransform TempTransform = FTransform();
		AActor* NewActor = InWorld->SpawnActor(ObjectRecord->ActorClass, &TempTransform, SpawnParameters);
		
		NewActor->SetActorRelativeTransform(ObjectRecord->ActorTransform);
		ApplySerializationActor(ObjectRecord->ActorData, NewActor);
		
		UPrimitiveComponent* RefRootC = Cast<UPrimitiveComponent>(NewActor->GetRootComponent());
		RefRootC->SetSimulatePhysics(ObjectRecord->bIsSimulatingPhysics);
		if (RefRootC->IsSimulatingPhysics())
		{
			RefRootC->SetPhysicsLinearVelocity(FVector());
			RefRootC->SetPhysicsAngularVelocityInDegrees(FVector());
			RefRootC->RecreatePhysicsState();
		}
		
		NewActor->UpdateComponentTransforms();
		OutActorArray.Add(NewActor);
	}
	
	return OutActorArray;
}

TArray<uint8> USaveState::SerializeState(int32& OutSavedItemAmount) const
{
	TArray<uint8> OutByteArray = {};
	TArray<FSavedObjectInfo*> SaveStateValueArray = {};
	
	FMemoryWriter MemoryWriter(OutByteArray, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
	
	SavedState.GenerateValueArray(SaveStateValueArray);

	for (OutSavedItemAmount = 0; OutSavedItemAmount < SavedState.Num(); OutSavedItemAmount++)
	{
		Archive << SaveStateValueArray[OutSavedItemAmount];
	}

	return OutByteArray;
}

void USaveState::ApplySerializeOnState(const TArray<uint8> ByteArray, const int& InSavedItemAmount)
{
	FMemoryReader MemoryReader(ByteArray, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
	if (SavedState.Num() > 0)
	{
		ensure(false);
		SavedState.Empty();
	}

	for (int Counter = 0; InSavedItemAmount > Counter; Counter++)
	{
		FString ObjectName = FString();
		FSavedObjectInfo* ObjectData = new FSavedObjectInfo();
		
		Archive << ObjectData;
		ObjectName = ObjectData->ActorName.ToString();

		if (!SavedClasses.Contains(ObjectData->ActorClass))
		{
			SavedClasses.Add(ObjectData->ActorClass);
		}
		UE_LOG(LogTemp, Warning, TEXT("Adding %s"), *ObjectName);

		SavedState.Add(ObjectName, ObjectData);
	}
}

TArray<AActor*> USaveState::GetActorsOfSavedClasses(UWorld* InWorld, TArray<UClass*> InClassArray)
{
	check(InWorld);
	TArray<AActor*> OutArray = TArray<AActor*>();
	
	for (UClass* RefClass : InClassArray)
	{
		TArray<AActor*> AddToArray = TArray<AActor*>();
		UGameplayStatics::GetAllActorsOfClass(InWorld, RefClass, AddToArray);
		OutArray.Append(AddToArray);
	}

	return OutArray;
}

TArray<uint8> USaveState::SerializeActor(AActor* InActor)
{
	TArray<uint8> OutputData;
	FMemoryWriter MemoryWriter(OutputData, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
	InActor->Serialize(Archive);

	// Iterate through the Child Components and serialize them as well.
	TArray<USceneComponent*> ChildComponents;
	InActor->GetComponents(ChildComponents);

	for (USceneComponent* CompToSave : ChildComponents)
	{
		CompToSave->Serialize(Archive);
	}

	return OutputData;
}

void USaveState::ApplySerializationActor(TArray<uint8>& ByteArray, AActor* InActor)
{
	FMemoryReader MemoryReader(ByteArray, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
	InActor->Serialize(Archive);

	// Iterate through the Child Components and serialize them as well.
	TArray<USceneComponent*> ChildComponents;
	InActor->GetComponents(ChildComponents);

	for (USceneComponent* CompToLoad : ChildComponents)
	{
		CompToLoad->Serialize(Archive);
		if (!CompToLoad->IsPhysicsStateCreated())
		{
			UE_LOG(LogTemp, Error, TEXT("Creating Physics for this Component: %s"), *CompToLoad->GetName())
			CompToLoad->RecreatePhysicsState();
		}
	}
}

TArray<UClass*> USaveState::GetSavedClasses() const
{
	return SavedClasses;
}
