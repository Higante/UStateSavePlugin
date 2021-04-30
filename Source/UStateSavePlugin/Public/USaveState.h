#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "USaveState.generated.h"

/**
 * TODO: Write bit more.
 * Archive which works as a proxy to Serialize Objects in a way that works.
 */
struct USTATESAVEPLUGIN_API FSaveStateGameArchive : public FObjectAndNameAsStringProxyArchive
{
	FSaveStateGameArchive(FArchive& InInnerArchive)
		: FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
	{
		ArIsSaveGame = true;
	}
};

/**
 * Custom Struct holding Informations and the Bytes to recreate an Actors using
 * the Serialize Method provided by Unreal Engine 4.
 */
USTRUCT()
struct FSavedObjectInfo
{
	GENERATED_USTRUCT_BODY()

public:
	// Optional Variables
	UPROPERTY()
	UClass* ActorClass;
	FString ActorName;
	FTransform ActorTransform;
	UStaticMesh* ActorStaticMeshRef;
	TArray<uint8> ActorData;

	FSavedObjectInfo()
	{
		ActorClass = AActor::StaticClass();
		ActorName = FString();
		ActorTransform = FTransform();
		ActorStaticMeshRef = nullptr;
		ActorData = TArray<uint8>();
	}

	FSavedObjectInfo(AActor* InputActor)
	{
		ActorClass = InputActor->GetClass();
		ActorName = InputActor->GetName();
		ActorTransform = InputActor->GetActorTransform();
		ActorStaticMeshRef = nullptr;
		ActorData = TArray<uint8>();
	}
};

UCLASS()
class USTATESAVEPLUGIN_API USaveState : public UObject
{
	GENERATED_BODY()

public:
	TMap<FString, FSavedObjectInfo> SavedState;
	TArray<FSavedObjectInfo> ObjectsToSpawn;

	USaveState();
	bool Save(UWorld* World, TArray<UClass*>& ToSave);
	bool Load(UWorld* World);

	/**
	 * Workaround Method to track Objects which have been spawned whilst in runtime.
	 * 
	 * @param InActor Reference to an Actor which shall be noted down to be deleted later.
	 */
	void OnSpawnChange(AActor* InActor);

	/**
	 * Workaround Method to track Objects which have been deleted whilst in runtime.
	 * 
	 * @param InActor Reference to an Actor which shall be noted down to be deleted later.
	 */
	void OnDeleteChange(AActor* InActor);

private:
	TArray<UClass*> SavedClasses;
	TArray<AActor*> ObjectsToDelete;

	/**
	 * Auxialiary Function. Clears the SaveStates Internal Variables.
	 */
	void ClearContents();

	/**
	 * Auxialiary Function. Helps getting all the relevant Actors in a InputWorld into one TArray. 
	 * 
	 * ? Is this truly something the SaveGame should check
	 * @param World The Reference to the GameWorld for which the SaveState should check for
	 * @param TRefClasses An Array of Classes which are looked for.
	 * @return Unsorted TArray of Actors which have been found.
	 */
	TArray<AActor*> GetEligibleActors(UWorld* InWorld, TArray<UClass*> TRefClasses);

	/**
	 * Auxialiary Function. Helps, using Serialize, loading Objects which have been deleted.
	 * 
	 * @param World The Reference to the GameWorld for which the SaveState has been created by.
	 * @param ObjectRecord Input Reference to the relevant Object which should be loaded using Serialization.
	 */
	void LoadSerilization(UWorld* InWorld, FSavedObjectInfo ObjectRecord);
};