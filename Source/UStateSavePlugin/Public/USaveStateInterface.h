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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	void ClearContents();
	
	/*
		Method intended to save the Objects relevant.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	bool Save(UWorld* World, TArray<UClass*>& ToSave);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	bool Load(UWorld* World);
};