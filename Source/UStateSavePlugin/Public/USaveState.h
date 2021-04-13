#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "USaveState.generated.h"

/**
 * Struct holding the relevant information about the Saved Object.
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
	TArray<uint8> ActorData;

	FSavedObjectInfo()
	{
		ActorClass = AActor::StaticClass();

		ActorLocation = FVector();
		ActorRotation = FRotator();
		ActorData = {};
	}

	FSavedObjectInfo(AActor* InputActor)
	{
		ActorClass = InputActor->GetClass();
		ActorLocation = InputActor->GetActorLocation();
		ActorRotation = InputActor->GetActorRotation();

		ActorData = {};
	}

	FORCEINLINE FArchive& Serialize(FArchive& Archive)
	{
		Archive << ActorClass;
		Archive << ActorLocation;
		Archive << ActorRotation;
		Archive << ActorData;
		return Archive;
	}
};

UCLASS()
class USTATESAVEPLUGIN_API USaveState : public UObject
{
	GENERATED_BODY()

public:
	USaveState();

	TMap<FString, FSavedObjectInfo> SavedState;
	
	bool Save(UWorld* World, TArray<UClass*>& ToSave);
	bool Load(UWorld* World);
	void OnSpawnChange(AActor* InActor);
	void OnDeleteChange(AActor* InActor);

private:
	// Save Objects etc.
	TArray<UClass*> SavedClasses;
	UPROPERTY()
	TArray<AActor*> ObjectsToDelete;
	UPROPERTY()
	TArray<FSavedObjectInfo> ObjectsToSpawn;

	void ClearContents();
	AActor* CreateSpawningActor(AActor* InActor);
};