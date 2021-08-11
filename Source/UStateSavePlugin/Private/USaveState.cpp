#include "USaveState.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectReader.h"
#include "Serialization/ObjectWriter.h"

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
			TSharedPtr<FSavedObjectInfo> ObjectSpawnInfo = MakeShareable<FSavedObjectInfo>(new FSavedObjectInfo(FoundActor));
			ObjectSpawnInfo->ActorData = SaveSerilization(FoundActor);

			SavedState.Emplace(FoundActor->GetName(), ObjectSpawnInfo);
		}
	}
	return true;
}

bool USaveState::Load(UWorld * World)
{
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
	
	// Create UObjects
	TArray<TSharedPtr<FSavedObjectInfo>> LoadObjectRecords = 
		TArray<TSharedPtr<FSavedObjectInfo>>();

	CopiedMap.GenerateValueArray(LoadObjectRecords);
	for (int i = 0; LoadObjectRecords.IsValidIndex(i); i++)
	{
		TSharedPtr<FSavedObjectInfo> ObjectRecord = LoadObjectRecords[i];

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ObjectRecord->ActorName;
		SpawnParameters.OverrideLevel = World->PersistentLevel;
		FTransform TempTransform = FTransform();
		AActor* NewActor = World->SpawnActor(ObjectRecord->ActorClass, 
			&TempTransform, SpawnParameters);

		ApplySerilizationActor(ObjectRecord->ActorData, NewActor);
		NewActor->SetActorTransform(ObjectRecord->ActorTransform);
		NewActor->UpdateComponentTransforms();
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

TArray<uint8> USaveState::SaveSerilization(AActor* SaveObject)
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

void USaveState::ApplySerilizationActor(UPARAM(ref) TArray<uint8> ObjectRecord, AActor* LoadObject)
{
	FMemoryReader MemoryReader(ObjectRecord, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
	LoadObject->Serialize(Archive);

	// Components Begin
	TArray<UActorComponent*> ChildComponents;
	LoadObject->GetComponents(ChildComponents);

	for (UActorComponent* CompToLoad : ChildComponents)
	{
		CompToLoad->Serialize(Archive);
	}
	// Components End
}

void USaveState::SaveToFile(const FString FilePath)
{
	TArray<uint8> SaveBytesStream = TArray<uint8>();

	FMemoryWriter MemoryWriter(SaveBytesStream);
	FArchive Archive(MemoryWriter);
	FBufferArchive BinaryData;
	for (TPair<FString,TSharedPtr<FSavedObjectInfo>> ToSave : SavedState)
	{
		FFileHelper::SaveArrayToFile(BinaryData, *FilePath);
	}
}

bool USaveState::LoadBytesFromFile(TArray<uint8>& OutBytes, const FString FilePath)
{
	return FFileHelper::LoadFileToArray(OutBytes, *FilePath);
}