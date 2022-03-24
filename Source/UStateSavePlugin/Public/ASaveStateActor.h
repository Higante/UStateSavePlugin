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

	/**
	 *	Delegate Function which is called if RosCalls the Save Function.
	 *
	 *	@param InFileName A String holding the named FileName.
	 */
	UFUNCTION()
	void RosCallSave(FString InFileName);
	
	/**
	 *	Delegate Function which is called if RosCalls the Load Function
	 *
	 *	@param InFileName A String holding the named FileName
	 */
	UFUNCTION()
	void RosCallLoad(FString InFileName);

	UFUNCTION()
	TArray<FString> ListAllSaveFilesAtLocation() const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	FString SaveSlotName = "Foobar";
	const FString SaveFilePath = FPaths::ProjectSavedDir() + "UStateSavePlugin/";
	
	UPROPERTY()
	USaveState* SavedState;

	/** Used as a workaround for physics */
	bool bHasLoadedLastTick = false;

	/** Array of Actors which will be refreshed for a Workaround. */
	UPROPERTY()
	TArray<AActor*> ActorsToRefreshOnTick = TArray<AActor*>();
	
	/**
	 * Saves the current State of the World into a File.
	 *
	 * @param FileName Name of the File on which to save on
	 * @param FilePath Path to the File
	 */
	UFUNCTION()
	void SaveStateCurrentWorld(FString FileName, FString FilePath);
	
	/**
	 * Function responsible loading stated File back into a usable state within the Unreal Engine.
	 *
	 * @param FileName Name of the File on which to load from
	 * @param FilePath Path ot the File
	 */
	UFUNCTION()
	void LoadStateOntoCurrentLevel(FString FileName, FString FilePath);
};