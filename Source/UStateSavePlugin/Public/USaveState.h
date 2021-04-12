#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "USaveStateInterface.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "USaveState.generated.h"

/**
 * Struct holding the relevant information about the Saved Object.
 * 
 * TODO: Increase it's range of variables
 */
USTRUCT()
struct FSavedObjectInfo
{
	GENERATED_BODY()

public:
	// Optional Variables
	UClass* ActorClass;

	FVector ActorLocation;
	FRotator ActorRotation;
	TArray<FName> Tags;
	TArray<uint8> ActorData;

	FSavedObjectInfo()
	{
		ActorLocation = FVector();
		ActorRotation = FRotator();
		Tags.Init(FName(), 0);
	}

	FSavedObjectInfo(AActor* InputActor)
	{
		ActorClass = InputActor->GetClass();

		ActorLocation = InputActor->GetActorLocation();
		ActorRotation = InputActor->GetActorRotation();
		Tags = InputActor->Tags;
	}

	/**
	 * Initial attempt to make it able to save the ActorData
	 * Inspiration Link: https://github.com/shinaka/UE4ActorSaveLoad/blob/master/PersistentStore.h
	 * TODO: Put this whole thing into its own file
	 */
	FORCEINLINE FArchive& Serialize(FArchive& Archive)
	{
		Archive << ActorData;
		return Archive;
	}
};

UCLASS()
class USTATESAVEPLUGIN_API USaveState : public UObject, public ISaveStateInterface
{
	GENERATED_BODY()

public:
	USaveState();

	TArray<UClass*> SavedClasses;
	TMap<FString, FSavedObjectInfo> SavedState;
	
	// Save Objects etc.
	TArray<AActor*> ObjectsToDelete;
	TArray<FSavedObjectInfo> ObjectsToSpawn;

	UFUNCTION()
	virtual void ClearContents() override;

	UFUNCTION()
	virtual bool Save(UWorld* World, TArray<UClass*>& ToSave) override;

	UFUNCTION()
	virtual bool Load(UWorld* World) override;

	UFUNCTION()
	virtual void OnSpawnChange(AActor* InActor) override;

	UFUNCTION()
	virtual void OnDeleteChange(AActor* InActor) override;

private:
	AActor* CreateSpawningActor(AActor* InActor);
};