#include "USaveStateInterface.h"

void ISaveStateInterface::ClearContents()
{
    UE_LOG(LogTemp, Error, TEXT("ClearContents hasn't been implemented yet!"));
}

bool ISaveStateInterface::Save(UWorld* World, TArray<UClass*>& ToSave)
{
    UE_LOG(LogTemp, Error, TEXT("Save hasn't been implemented yet!"));
    return false;
}

bool ISaveStateInterface::Load(UWorld* World)
{
    UE_LOG(LogTemp, Error, TEXT("Load hasn't been implemented yet!"));
    return false;
}

void ISaveStateInterface::OnSpawnChange(AActor* InActor)
{
    UE_LOG(LogTemp, Error, TEXT("OnSpawnChange hasn't been implemented yet!"));
}

void ISaveStateInterface::OnDeleteChange(AActor* InActor)
{
    UE_LOG(LogTemp, Error, TEXT("OnDeleteChange hasn't been implemented yet!"));
}