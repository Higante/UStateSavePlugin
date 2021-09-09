// Copyright Notices

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "USaveState.h"
#include "AStateSaveObject.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSaveThisState, FString, SaveFileName, FString, SaveFilePath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLoadThisState, FString, LoadFileName, FString, SaveFilePath);

UCLASS()
class USTATESAVEPLUGIN_API AStateSaveObject : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	bool bDebug = false;
	UPROPERTY(EditAnywhere)
	bool bSave = false;
	UPROPERTY(EditAnywhere)
	bool bLoad = false;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> ClassesToSave;

	AStateSaveObject();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SaveState(FString FileName,FString FilePath);
	
	UFUNCTION()
	void LoadState(FString FileName,FString FilePath);

	UFUNCTION()
	TArray<FString> ListAllSaveFilesAtLocation() const;

protected:
	virtual void BeginPlay() override;
	FSaveThisState SaveDelegate;
	FLoadThisState LoadDelegate;

private:
	FString SaveFilePath = FPaths::ProjectSavedDir() + "UStateSavePlugin/";
	UPROPERTY(EditAnywhere)
	FString SaveSlotName = "Foobar";
	USaveState* SavedState;
	TArray<FString> FilesInFolder = TArray<FString>();
};