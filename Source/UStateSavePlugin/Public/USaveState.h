#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "USaveState.generated.h"

struct USTATESAVEPLUGIN_API FSaveStateGameArchive : public FObjectAndNameAsStringProxyArchive
{
	FSaveStateGameArchive(FArchive& InInnerArchive)
		: FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
	{
		ArIsSaveGame = true;
	}
};

/**
 * Struct holding the relevant information about the Saved Object.
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
	TArray<uint8> ActorData;

	FSavedObjectInfo()
	{
		ActorClass = AActor::StaticClass();
		ActorName = FString();
		ActorTransform = FTransform();
		ActorData = {};
	}

	FSavedObjectInfo(AActor* InputActor)
	{
		ActorClass = InputActor->GetClass();
		ActorName = InputActor->GetName();
		ActorTransform = InputActor->GetActorTransform();
		ActorData = {};
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
	TArray<UClass*> SavedClasses;
	TArray<AActor*> ObjectsToDelete;
	TArray<FSavedObjectInfo> ObjectsToSpawn;

	void ClearContents();
	AActor* CreateSpawningActor(AActor* InActor);
	TArray<AActor*> GetEligibleActorsToSave(UWorld* InWorld, TArray<UClass*> TRefClasses);
};