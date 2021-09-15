// Copyright Notices

#pragma once

#include "CoreMinimal.h"
#include "FROSLoadStateLevel.h"
#include "FROSSaveStateLevel.h"
#include "GameFramework/Actor.h"
#include "USaveState.h"
#include "AStateSaveObject.generated.h"

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSaveThisState, FString, SaveFileName, FString, SaveFilePath);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLoadThisState, FString, LoadFileName, FString, SaveFilePath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSaveThisState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLoadThisState);
DECLARE_DYNAMIC_DELEGATE_RetVal(TArray<FString>, FListAllFilesInFolder);

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
	FSaveThisState SaveDelegate;
	FLoadThisState LoadDelegate;

	// ROS Services
	TSharedPtr<FROSSaveStateLevel> SaveService;
	TSharedPtr<FROSLoadStateLevel> LoadService;

	const FString SaveServiceTopic = FString("/unreal_save_system/save");
	const FString LoadServiceTopic = FString("/unreal_save_system/load");

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> ClassesToSave;

	AStateSaveObject();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void CallSave();
	UFUNCTION()
	void CallLoad();
	
	UFUNCTION()
	void SaveState(FString FileName,FString FilePath);
	UFUNCTION()
	void LoadState(FString FileName,FString FilePath);

	UFUNCTION()
	TArray<FString> ListAllSaveFilesAtLocation();

protected:
	virtual void BeginPlay() override;
	FListAllFilesInFolder ListDelegate;

private:
	FString SaveFilePath = FPaths::ProjectSavedDir() + "UStateSavePlugin/";
	UPROPERTY(EditAnywhere)
	FString SaveSlotName = "Foobar";
	USaveState* SavedState;
	TArray<FString> FilesInFolder = TArray<FString>();
};