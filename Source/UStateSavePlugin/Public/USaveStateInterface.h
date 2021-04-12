#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "USaveStateInterface.generated.h"

UINTERFACE()
class USTATESAVEPLUGIN_API USaveStateInterface : public UInterface
{
	GENERATED_BODY()
};

class USTATESAVEPLUGIN_API ISaveStateInterface
{
	GENERATED_BODY()

public:
	/*
		Method is intended to clear the Variables hold within this SaveState.
	*/
	UFUNCTION()
	virtual void ClearContents();
	
	/*
		Method intended to save the Objects relevant.
	*/
	UFUNCTION()
	virtual bool Save(UWorld* World, TArray<UClass*>& ToSave);

	UFUNCTION()
	virtual bool Load(UWorld* World);

	UFUNCTION()
	virtual void OnSpawnChange(AActor* InActor);

	UFUNCTION()
	virtual void OnDeleteChange(AActor* InActor);
};