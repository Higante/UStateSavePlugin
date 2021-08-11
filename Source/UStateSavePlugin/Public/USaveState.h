#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Engine/StaticMesh.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "USaveState.generated.h"

/**
 * Custom Struct holding Informations and the Bytes to recreate an Actors using
 * the Serialize Method provided by Unreal Engine 4.
 */
USTRUCT()
struct FSavedObjectInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FName ActorName;
	FTransform ActorTransform;
	TArray<uint8> ActorData;
	TArray<uint8> StaticMeshData;
	UClass* ActorClass;
	UObject* OuterObject;
	int32 OuterID;

	FSavedObjectInfo()
	{
		ActorClass = AActor::StaticClass();
		ActorData = TArray<uint8>();
		ActorName = FName();
		ActorTransform = FTransform();
		OuterObject = nullptr;
		OuterID = 0;
	}

	FSavedObjectInfo(AActor* InputActor)
	{
		ActorClass = InputActor->GetClass();
		ActorData = TArray<uint8>();
		StaticMeshData = TArray<uint8>();
		ActorName = InputActor->GetFName();
		ActorTransform = InputActor->GetActorTransform();
		OuterObject = InputActor->GetOuter();
		OuterID = InputActor->GetOuter()->GetUniqueID();
	}
};


UCLASS()
class USTATESAVEPLUGIN_API USaveState : public UObject
{
	GENERATED_BODY()

public:
	TMap<FString, TSharedPtr<FSavedObjectInfo>> SavedState;
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
	 * @param World The Reference to the GameWorld for which the SaveState should check for
	 * @param TRefClasses An Array of Classes which are looked for.
	 * @return Unsorted TArray of Actors which have been found.
	 */
	TArray<AActor*> GetEligibleActors(UWorld* InWorld, TArray<UClass*> TRefClasses);

	TArray<uint8> SaveSerilization(AActor* SaveObject);

	/**
	 * Auxialiary Function. Helps, using Serialize, loading Objects which have been deleted.
	 * 
	 * @param ObjectRecord Input Reference to the relevant Object which should be loaded using Serialization.
	 */
	void ApplySerilizationActor(TArray<uint8> ObjectRecord, AActor* LoadObject);

	// TODO: WIP
	void SaveToFile(const FString FilePath);

	// TODO: WIP
	bool LoadBytesFromFile(TArray<uint8>& OutBytes, const FString FilePath);
};