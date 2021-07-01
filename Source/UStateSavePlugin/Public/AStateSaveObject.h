// Copyright Notices

#pragma once

#include <CoreMinimal.h>
#include <Engine/World.h>
#include <GameFramework/Actor.h>
#include <Map.h>
#include "USaveState.h"
#include "AStateSaveObject.generated.h"

UCLASS()
class USTATESAVEPLUGIN_API AStateSaveObject : public AActor
{
	GENERATED_BODY()
// Variables
public:
	UWorld* World;

	UPROPERTY(EditAnywhere)
	bool bDebug = false;

	UPROPERTY(EditAnywhere)
	bool bSave = false;
	UPROPERTY(EditAnywhere)
	bool bLoad = false;
	UPROPERTY(EditAnywhere)
	int MaximumSaveStates = 3;
	UPROPERTY(EditAnywhere)
	int SlotToWork = 0;


	UPROPERTY(EditAnywhere)
	TArray<UClass*> ClassesToSave;

private:
	UPROPERTY()
	TArray<USaveState*> SavedStates;
	
// Functions
public:
	AStateSaveObject();

	virtual void Tick(float DeltaTime) override;

	bool SaveState(int Slot);
	bool LoadState(int Slot);

private:
	/**
	 * This Function will be called to work as a middleman between itself
	 * and SaveStates who then track what needs to be deleted once the need
	 * calls for it.
	 * 
	 * @param InActor The Spawned Actor in the World
	 */
	UFUNCTION()
	void SpawnHandler(AActor* InActor);

protected:
	virtual void BeginPlay() override;

};