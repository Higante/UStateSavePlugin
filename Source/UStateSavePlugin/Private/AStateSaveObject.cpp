#include "AStateSaveObject.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "USaveState.h"
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

	if (bDeleteDebug)
	{
		for (USaveState* SaveState : SavedStates)
		{
			UE_LOG(LogTemp, Warning, TEXT("Amount to Delete: %d"), SaveState->ObjectsToDelete.Num());
			UE_LOG(LogTemp, Warning, TEXT("Amount to Spawn: %d"), SaveState->ObjectsToSpawn.Num());
		}

		bDeleteDebug = false;
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
	//SavedStates.Init(NewObject<USaveState>(), MaximumSaveStates);

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
	if (Slot < 0 || Slot > MaximumSaveStates)
	{
		UE_LOG(LogTemp, Error, TEXT("Error (%s): Slot given not in range."), *__FUNCTION__);
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

void AStateSaveObject::OnDestroyHandler(AActor* InActor)
{
	UE_LOG(LogTemp, Warning, TEXT("%s : Begin handling the Destruction"), TEXT(__FUNCTION__));
	for (int i = 0; i < MaximumSaveStates; i++)
	{
		SavedStates[i]->OnDeleteChange(InActor);
	}
	/*
	for (USaveState* SaveState : SavedStates)
	{
		SaveState->OnDeleteChange(InActor);
	}
	*/
}

void AStateSaveObject::SpawnHandler(AActor * InActor)
{
	if (!ClassesToSave.Contains(InActor->GetClass()))
		return;

	UE_LOG(LogTemp, Warning, TEXT("%s has been spawned."), *InActor->GetName());

	InActor->OnDestroyed.AddDynamic(this, &AStateSaveObject::OnDestroyHandler);

	for (int i = 0; i < MaximumSaveStates; i++)
	{
		SavedStates[i]->OnSpawnChange(InActor);
	}

	/*
	for (USaveState* SaveState : SavedStates)
	{
		SaveState->OnSpawnChange(InActor);
	}
	*/
}