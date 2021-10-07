// Copyright Notices

#pragma once

#include "CoreMinimal.h"
#include "FROSLoadStateLevel.h"
#include "FROSSaveStateLevel.h"
#include "GameFramework/Actor.h"
#include "USaveState.h"
#include "ASaveStateActor.generated.h"

UCLASS()
class USTATESAVEPLUGIN_API ASaveStateActor final : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	bool bDebug = false;
	UPROPERTY(EditAnywhere)
	bool bSave = false;
	UPROPERTY(EditAnywhere)
	bool bLoad = false;

	// ROS Services
	TSharedPtr<FROSSaveStateLevel> SaveService;
	TSharedPtr<FROSLoadStateLevel> LoadService;

	const FString SaveServiceTopic = FString("/unreal_save_system/ue4_ros_save");
	const FString LoadServiceTopic = FString("/unreal_save_system/ue4_ros_load");

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> ClassesToSave;

	ASaveStateActor();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void RosCallSave();
	UFUNCTION()
	void RosCallLoad();

	UFUNCTION()
	TArray<FString> ListAllSaveFilesAtLocation() const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	FString SaveSlotName = "Foobar";
	const FString SaveFilePath = FPaths::ProjectSavedDir() + "UStateSavePlugin/";
	
	USaveState* SavedState;
	
	UFUNCTION()
	void SaveState(FString FileName,FString FilePath);
	UFUNCTION()
	void LoadState(FString FileName,FString FilePath);
};