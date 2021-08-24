// Copyright Notices

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "USaveState.h"
#include "AStateSaveObject.generated.h"

UCLASS()
class USTATESAVEPLUGIN_API AStateSaveObject : public AActor
{
	GENERATED_BODY()
// Variables
public:
	UPROPERTY(EditAnywhere)
	bool bDebug = false;
	UPROPERTY(EditAnywhere)
	bool bSave = false;
	UPROPERTY(EditAnywhere)
	bool bLoad = false;

	UPROPERTY(EditAnywhere)
	TArray<UClass*> ClassesToSave;

	AStateSaveObject();

	virtual void Tick(float DeltaTime) override;

	bool SaveState(const FString FileName, const FString FilePath);
	bool LoadState(const FString FileName, const FString FilePath);

protected:
	virtual void BeginPlay() override;

private:
	const FString SaveFilePath;
	TSharedPtr<USaveState> SavedState;
	
	/**
	 * This Function will be called to work as a middleman between itself
	 * and SaveStates who then track what needs to be deleted once the need
	 * calls for it.
	 * 
	 * @param InActor The Spawned Actor in the World
	 */
	UFUNCTION()
	void SpawnHandler(AActor* InActor);
};