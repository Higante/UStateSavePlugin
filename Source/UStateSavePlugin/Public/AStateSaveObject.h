// Copyright Notices

#pragma once

#include <CoreMinimal.h>
#include <Engine/World.h>
#include <GameFramework/Actor.h>
#include <Map.h>
#include <Tuple.h>
#include "AStateSaveObject.generated.h"

USTRUCT(BlueprintType)
struct FSavedObjectInfo
{
	GENERATED_BODY()

	FVector ActorLocation;
	FRotator ActorRotation;
	TArray<FName> Tags;

	FSavedObjectInfo()
	{
		ActorLocation = FVector();
		ActorRotation = FRotator();
		Tags.Init(FName(), 0);
	}
};

UCLASS()
class USTATESAVEPLUGIN_API AStateSaveObject : public AActor
{
	GENERATED_BODY()
// Variables
public: 
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

private:
	UWorld* World;
	TArray<TMap<FString, FSavedObjectInfo>> SavedSlots;
	
// Functions
public:
	AStateSaveObject();

	virtual void Tick(float DeltaTime) override;

	bool SaveState(int Slot);
	bool LoadState(int Slot);

private:
	// WIP, in combination with a better interface
	bool Save(int Slot);
	bool Load(int Slot);

protected:
	virtual void BeginPlay() override;

};