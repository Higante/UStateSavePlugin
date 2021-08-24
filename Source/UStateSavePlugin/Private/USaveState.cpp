#include "USaveState.h"
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
	SavedState = TMap<FString, TSharedPtr<FSavedObjectInfo>>();
}

void USaveState::ClearContents()
{
	SavedClasses.Empty();
	SavedState.Empty();
}

bool USaveState::Save(UWorld * World, const TArray<UClass*>& ClassesToSave)
{
	ClearContents();

	SavedClasses = ClassesToSave;
	TArray<AActor*> ActorsToSave = GetSavableActorsOfClass(World, SavedClasses);
	for (AActor* FoundActor : ActorsToSave)
	{
		UPrimitiveComponent* RootRefC = Cast<UPrimitiveComponent>(FoundActor->GetRootComponent());
		if (RootRefC != nullptr && RootRefC->Mobility == EComponentMobility::Movable)
		{
			TSharedPtr<FSavedObjectInfo> ObjectSpawnInfo = MakeShareable<FSavedObjectInfo>(new FSavedObjectInfo(FoundActor));
			ObjectSpawnInfo->ActorData = SaveSerialization(FoundActor);

			SavedState.Emplace(FoundActor->GetName(), ObjectSpawnInfo);
		}
	}
	return true;
}

bool USaveState::Load(UWorld * World)
{
	TMap<FString, TSharedPtr<FSavedObjectInfo>> CopiedMap = SavedState;
	for (AActor* FoundActor : GetSavableActorsOfClass(World, SavedClasses))
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
		const TSharedPtr<FSavedObjectInfo> ObjectRecord = LoadObjectRecords[i];

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

TArray<uint8> USaveState::SerializeState(uint8& OutSavedItemAmount) const
{
	TArray<uint8> OutputSerialization = {};
	TArray<FString> ObjectNameArray = {};
	TArray<TSharedPtr<FSavedObjectInfo>> ObjectInfoArray = {};
	
	FMemoryWriter MemoryWriter(OutputSerialization, true);
	// FArchive Archive(MemoryWriter, true)
	
	SavedState.GenerateKeyArray(ObjectNameArray);
	SavedState.GenerateValueArray(ObjectInfoArray);

	for (OutSavedItemAmount = 0; OutSavedItemAmount < SavedState.Num(); OutSavedItemAmount++)
	{
		MemoryWriter << ObjectNameArray[OutSavedItemAmount];
		MemoryWriter << ObjectInfoArray[OutSavedItemAmount];
	}

	return OutputSerialization;
}

void USaveState::ApplySerializeOnState(const TArray<uint8> SerializedState, const uint8& InSavedItemAmount)
{
	// TODO: Find a method
	TArray<FString> ObjectNameArray = {};
	TArray<TSharedPtr<FSavedObjectInfo>> ObjectInfoArray = {};
	
	FMemoryReader MemoryReader(SerializedState, true);
	SavedState.Empty();

	for (uint8 Counter = 0; InSavedItemAmount > Counter; Counter++)
	{
		FString ObjectName = FString();
		TSharedPtr<FSavedObjectInfo> ObjectData = TSharedPtr<FSavedObjectInfo>();
		
		MemoryReader << ObjectName;
		ObjectNameArray.Add(ObjectName);
		MemoryReader << ObjectData;
		ObjectInfoArray.Add(ObjectData);

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