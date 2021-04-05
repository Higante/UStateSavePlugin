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
 * Struct works as a proxy for FArchive to use ObjectAndNameAsString Proxy
 * URL: https://github.com/shinaka/UE4ActorSaveLoad/blob/c58f9ff12301c7a764e4fbdeb8af2349195dadb3/PersistentStore.h#L6-L19
 */
struct USTATESAVEPLUGIN_API FSaveGameArchive : public FObjectAndNameAsStringProxyArchive
{
	FSaveGameArchive(FArchive& IsInnerArchive) : FObjectAndNameAsStringProxyArchive(IsInnerArchive, true)
	{
		ArIsSaveGame = true;
	}
};

/**
 * Struct holding the relevant information about the Saved Object.
 * 
 * ? Does it makes sense to put it into it's own file and inheritance ?
 * TODO: Increase it's range of variables
 */
USTRUCT(BlueprintType)
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
	friend FArchive& operator<<(FArchive& Archive, FSavedObjectInfo& ActorData);
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

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	void ClearContents();
	void ClearContents_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	bool Save(UWorld* World, TArray<UClass*>& ToSave);
	bool Save_Implementation(UWorld* World, TArray<UClass*>& ToSave) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	bool Load(UWorld* World);
	bool Load_Implementation(UWorld* World) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	void OnSpawnChange(AActor* InActor);
	void OnSpawnChange_Implementation(AActor* InActor) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	void OnDeleteChange(AActor* InActor);
	void OnDeleteChange_Implementation(AActor* InActor) override;

private:
	AActor* CreateSpawningActor(AActor* InActor);
};