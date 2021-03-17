#include "AStateSaveObject.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

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
	SavedStates.Init(NewObject<USaveState>(), MaximumSaveStates);
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
	if (Slot < 0 || Slot > MaximumSaveStates)
	{
		UE_LOG(LogTemp, Error, TEXT("Error (%s): Slot given not in range."), __FUNCTION__);
		return false;
	}

	SavedStates[Slot]->ClearContents();

	return SavedStates[Slot]->Save(World, ClassesToSave);
}

/*
	Function serving to load the state saved on one of the slots of SavedSlots.

	@param Slot Input integer value with telling on which slot it all will be loaded from.
	@return bool true is successful, false if not.
*/
bool AStateSaveObject::LoadState(int Slot)
{
	// Check whether the given int is within range
	if (Slot < 0 || Slot > MaximumSaveStates)
	{
		UE_LOG(LogTemp, Error, TEXT("Error (%s): Slot given not in range."), TEXT(__FUNCTION__));
		return false;
	}

	if (SavedStates[Slot] == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Error (%s): SavedState on Slot not set."), TEXT(__FUNCTION__));
		return false;
	}

	return SavedStates[Slot]->Load(World);
}