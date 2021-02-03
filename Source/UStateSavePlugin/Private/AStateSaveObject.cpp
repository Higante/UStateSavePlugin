#include "AStateSaveObject.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AStateSaveObject::AStateSaveObject()
{
	PrimaryActorTick.bCanEverTick = true;
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
	SavedSlots.Init(TMap<FString, TTuple<FVector, FRotator, TArray<FName>>>(), MaximumSaveStates);
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

	TMap<FString, TTuple<FVector, FRotator, TArray<FName>>> SavingMap = TMap<FString, TTuple<FVector, FRotator, TArray<FName>>>();
	TArray<AActor*> OutArray = TArray<AActor*>();

	UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), OutArray);
	if (bDebug)
		UE_LOG(LogTemp, Warning, TEXT("%s: Amount of StaticActors... %d"), TEXT(__FUNCTION__), OutArray.Num());
	for (AActor* actor : OutArray)
	{
		if (Cast<AStaticMeshActor>(actor)->GetStaticMeshComponent()->Mobility == EComponentMobility::Movable)
		{
			SavingMap.Add(actor->GetName(), MakeTuple(actor->GetActorLocation(), actor->GetActorRotation(), actor->Tags));
		}
	}

	if (bDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Saved Things"), TEXT(__FUNCTION__));
	}
	SavedSlots[Slot-1] = SavingMap;
	return true;
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

	// Get the Current Actors still in the world.
	TMap<FString, TTuple<FVector, FRotator, TArray<FName>>> LoadingMap = SavedSlots[Slot-1];
	TArray<AActor*> OutArray = TArray<AActor*>();

	UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), OutArray);
	for (AActor* actor : OutArray)
	{
		if (Cast<AStaticMeshActor>(actor)->GetStaticMeshComponent()->Mobility == EComponentMobility::Movable && LoadingMap.Find(actor->GetName()) != nullptr)
		{
			// Set Location, Rotation and tags to what it was before
			TTuple<FVector, FRotator, TArray<FName>>* LoadedObject = LoadingMap.Find(actor->GetName());
			actor->SetActorLocation(LoadedObject->Get<0>());
			actor->SetActorRotation(LoadedObject->Get<1>());
			actor->Tags = LoadedObject->Get<2>();

			// Set Velocity of the Object ot 0
			Cast<AStaticMeshActor>(actor)->GetStaticMeshComponent()->SetPhysicsLinearVelocity(FVector(0, 0, 0));
			Cast<AStaticMeshActor>(actor)->GetStaticMeshComponent()->SetPhysicsAngularVelocity(FVector(0, 0, 0));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("%s: Loaded Things"), TEXT(__FUNCTION__));

	return true;
}

	bool AStateSaveObject::Save(int Slot)
	{
		if (SavedSlots[Slot-1].Num != 0) 
		{
		}

		return false;
	}

	bool AStateSaveObject::Load(int Slot)
	{
		return false;
	}
