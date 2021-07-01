#include "AStateSaveObject.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "USaveState.h"

AStateSaveObject::AStateSaveObject()
{
	PrimaryActorTick.bCanEverTick = true;

	ClassesToSave.Add(AStaticMeshActor::StaticClass());
}

void AStateSaveObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bSave)
	{
		SaveState(SlotToWork);
		bSave = false;
	}

	if (bLoad)
	{
		LoadState(SlotToWork);
		bLoad = false;
	}
}

void AStateSaveObject::BeginPlay()
{
	Super::BeginPlay();

	World = this->GetWorld();

	// Setup the SaveSlots
	for (int i = 0; i < MaximumSaveStates; i++)
	{
		SavedStates.Add(NewObject<USaveState>());
	}

	FOnActorSpawned::FDelegate OnActorSpawnDelegate = FOnActorSpawned::FDelegate::CreateUObject(this, &AStateSaveObject::SpawnHandler);
	GetWorld()->AddOnActorSpawnedHandler(OnActorSpawnDelegate);
}

/*
	This function is called to save the current state of all dynamic 
	objects in a world as is currently. 

	@param Slot Input integer value with telling on which slot it all will be saved on.
	@return bool true is successful, false if not.
*/
bool AStateSaveObject::SaveState(int Slot)
{
	// Check whether the given int is within range.
	if (Slot < 0 || Slot >= MaximumSaveStates)
	{
		UE_LOG(LogTemp, Error, TEXT("Error (%s): Slot given not in range."), TEXT(__FUNCTION__));
		return false;
	}

	return SavedStates[Slot]->Save(World, ClassesToSave);
}

/*
	Function serving to load the state saved on one of the slots of SavedSlots.

	@param Slot Input integer value with telling on which slot it all will be loaded from.
	@return bool true is successful, false if not.
*/
bool AStateSaveObject::LoadState(int Slot)
{
	UE_LOG(LogTemp, Warning, TEXT("%s : Begin loading State!"), TEXT(__FUNCTION__));
	// Check whether the given int is within range
	if (Slot < 0 || Slot > MaximumSaveStates)
	{
		UE_LOG(LogTemp, Error, TEXT("Error (%s): Slot given not in range."), TEXT(__FUNCTION__));
		return false;
	}

	if (SavedStates[Slot] == NULL || SavedStates[Slot] == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Error (%s): SavedState on Slot not set."), TEXT(__FUNCTION__));
		return false;
	}

	return SavedStates[Slot]->Load(World);
}

void AStateSaveObject::SpawnHandler(AActor * InActor)
{
	if (!ClassesToSave.Contains(InActor->GetClass()))
		return;

	UE_LOG(LogTemp, Warning, TEXT("%s: %s has been spawned."), TEXT(__FUNCTION__), *InActor->GetName());

	for (int i = 0; i < MaximumSaveStates; i++)
	{
		SavedStates[i]->OnSpawnChange(InActor);
	}
}
