// Copyright Notices

#pragma once

#include <CoreMinimal.h>
#include <Engine/World.h>
#include <GameFramework/Actor.h>
#include "USaveState.h"
#include <Map.h>
#include "AStateSaveObject.generated.h"

UCLASS()
class USTATESAVEPLUGIN_API AStateSaveObject : public AActor
{
	GENERATED_BODY()
// Variables
public:
	UWorld* World;

	// Change to Editor Only
	UPROPERTY(EditAnywhere)
	int MaximumSaveStates = 5;

	UPROPERTY(EditAnywhere)
	bool bDebug = false;

	// Temporary Values until Alternative implemented
	UPROPERTY(EditAnywhere)
	bool bSave = false;
	UPROPERTY(EditAnywhere)
	bool bLoad = false;
	UPROPERTY(EditAnywhere)
	int SlotToWork = 1;


	UPROPERTY(EditAnywhere)
	TArray<UClass*> ClassesToSave;

private:
	TArray<USaveState*> SavedStates;
	
// Functions
public:
	AStateSaveObject();

	virtual void Tick(float DeltaTime) override;

	bool SaveState(int Slot);
	bool LoadState(int Slot);

private:

protected:
	virtual void BeginPlay() override;

};