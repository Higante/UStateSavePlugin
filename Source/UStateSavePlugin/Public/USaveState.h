#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Engine/DemoNetDriver.h"
#include "Engine/StaticMesh.h"
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
	UClass* ActorClass;

	FSavedObjectInfo()
	{
		ActorClass = AActor::StaticClass();
		ActorData = TArray<uint8>();
		ActorName = FName();
		ActorTransform = FTransform();
	}

	FSavedObjectInfo(const AActor* InputActor)
	{
		ActorClass = InputActor->GetClass();
		ActorData = TArray<uint8>();
		ActorName = InputActor->GetFName();
		ActorTransform = InputActor->GetActorTransform();
	}

	friend FORCEINLINE FArchive& operator<<(FArchive &Ar, TSharedPtr<FSavedObjectInfo> SavedObjectInfo)
	{
		Ar << SavedObjectInfo->ActorClass;
		Ar << SavedObjectInfo->ActorName;
		Ar << SavedObjectInfo->ActorTransform;
		Ar << SavedObjectInfo->ActorData;
		
		return Ar;
	}
};


UCLASS()
class USTATESAVEPLUGIN_API USaveState : public UObject
{
	GENERATED_BODY()

public:
	TMap<FString, TSharedPtr<FSavedObjectInfo>> SavedState;

	USaveState();
	bool Save(UWorld* World, const TArray<UClass*>& ToSave);
	bool Load(UWorld* World);

	/**
	 * Function to Serialize the SaveState. To be used along to bring it into an USaveGame
	 *
	 * @param OutSavedItemAmount An reference to an uint8 on which we're to track how many items we've saved.
	 * @return The serialized data of this SaveState.
	 */
	TArray<uint8> SerializeState(uint8& OutSavedItemAmount) const;

	/**
	 * Function intended to reapply previously serialized data back to be used.
	 *
	 * @param SerializedState Data which has been created using Serialize.
	 * @param InSavedItemAmount Amount of Items which has been saved before.
	 */
	void ApplySerializeOnState(const TArray<uint8> SerializedState, const uint8& InSavedItemAmount);
	
private:
	TArray<UClass*> SavedClasses;

	/**
	 * Auxiliary Function. Clears the SaveStates Internal Variables.
	 */
	void ClearContents();

	/**
	 * Auxiliary Function. Helps getting all the relevant Actors in a InputWorld into one TArray. 
	 * 
	 * @param InWorld The Reference to the GameWorld for which the SaveState should check for
	 * @param TRefClasses An Array of Classes which are looked for.
	 * @return Unsorted TArray of Actors which have been found.
	 */
	static TArray<AActor*> GetSavableActorsOfClass(UWorld* InWorld, TArray<UClass*> TRefClasses);

	/**
	 * Auxiliary Function, intended to save given Actor in a format which can be restored later on.
	 *
	 * @param SaveObject Pointer to an Actor which is not Null.
	 * @return The Serialized Array of Bytes representing said Actor, able to be restored with Serialize.
	 */
	static TArray<uint8> SaveSerialization(AActor* SaveObject);

	/**
	 * Auxiliary Function. Helps, using Serialize, loading Objects which have been deleted.
	 * 
	 * @param ObjectRecord Reference to an Array holding the serialized Actor Data.
	 * @param ObjectToApplyOn Pointer to an Actor on which to apply given serialized data.
	 */
	static void ApplySerializationActor(TArray<uint8>& ObjectRecord, AActor* ObjectToApplyOn);
};