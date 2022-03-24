#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Engine/DemoNetDriver.h"
#include "Engine/StaticMesh.h"
#include "USaveState.generated.h"

/**
 * Custom Struct holding Information and the Bytes to recreate an Actors using
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
	bool bIsSimulatingPhysics = false;

	FSavedObjectInfo()
	{
		ActorClass = AActor::StaticClass();
		ActorData = TArray<uint8>();
		ActorName = FName();
		ActorTransform = FTransform();
		bool bIsSimulatingPhysics = false;
	}

	FSavedObjectInfo(const AActor* InputActor)
	{
		ActorClass = InputActor->GetClass();
		ActorData = TArray<uint8>();
		ActorName = InputActor->GetFName();
		ActorTransform = InputActor->GetActorTransform();
		if (UPrimitiveComponent* RefRootC = Cast<UPrimitiveComponent>(InputActor->GetRootComponent()))
		{
			bIsSimulatingPhysics = RefRootC->IsSimulatingPhysics();
		}
	}

	friend FORCEINLINE FArchive& operator<<(FArchive &Ar, FSavedObjectInfo* SavedObjectInfo)
	{
		Ar << SavedObjectInfo->ActorClass;
		Ar << SavedObjectInfo->ActorName;
		Ar << SavedObjectInfo->ActorTransform;
		Ar << SavedObjectInfo->bIsSimulatingPhysics;
		Ar << SavedObjectInfo->ActorData;
		
		return Ar;
	}
};


UCLASS()
class USTATESAVEPLUGIN_API USaveState : public UObject
{
	GENERATED_BODY()

public:
	/** SaveState represented by a TMap and a custom struct. */
	TMap<FString, FSavedObjectInfo*> SavedState = TMap<FString, FSavedObjectInfo*>();

	USaveState(){};

	/**
	 * Initiates the Saving Process and saves all actors who are from the same Subclass of Actors
	 *
	 * @param InWorld World from which to save into the SaveState.
	 * @param InClasses Array of AActor Classes which to serialize and save in the given world.
	 * @return true if the saving has been done successfully
	 */
	bool SaveFromWorld(UWorld* InWorld, const TArray<TSubclassOf<AActor>>& InClasses);

	/**
	 * Initiates the Loading Process and applies the saved Byte Array onto the World.
	 *
	 * @param InWorld World into which to load the SaveState onto.
	 * @return List of Actors which have been loaded
	 */
	TArray<AActor*> LoadOntoWorld(UWorld* InWorld);

	/**
	 * Function to Serialize the SaveState. To be used along to bring it into an USaveGame
	 *
	 * @param OutSavedItemAmount An reference to an uint8 on which we're to track how many items we've saved.
	 * @return The serialized data of this SaveState.
	 */
	TArray<uint8> SerializeState(int32& OutSavedItemAmount) const;

	/**
	 * Function intended to reapply previously serialized data back to be used.
	 *
	 * @param ByteArray Data which has been created using Serialize.
	 * @param InSavedItemAmount Amount of Items which has been saved before.
	 */
	void ApplySerializeOnState(const TArray<uint8> ByteArray, const int& InSavedItemAmount);

	/** Gets the Array with References of Classes being saved here. */
	TArray<UClass*> GetSavedClasses() const;

	/**
	 * We're getting all Actors from the . 
	 * 
	 * @param InWorld World in which to check for the Actors.
	 * @param InClassArray Array holding Pointers to Classes to save.
	 * @return Unsorted TArray of Actors which have been found.
	 */
	UFUNCTION()
	static TArray<AActor*> GetActorsOfSavedClasses(UWorld* InWorld, UPARAM(ref) TArray<UClass*> InClassArray);
	
private:
	/** Set of unique Classes saved in this state */
	TArray<UClass*> SavedClasses = {};

	/** Clears the Internal Variables from it's current references */
	void ClearContents();

	/**
	 * Serializes the Input Actor into an ByteArray, to work with Serialize.
	 *
	 * @param InActor Actor pointer from which to Serialize the Data.
	 * @return ByteArray representing the data of the InActor.
	 */
	static TArray<uint8> SerializeActor(AActor* InActor);

	/**
	 * Applies the given ByteArray onto the pointed Actor
	 * 
	 * @param ByteArray ByteArray of .
	 * @param InActor Actor on which to apply the Seriaized Data.
	 */
	static void ApplySerializationActor(TArray<uint8>& ByteArray, AActor* InActor);
};