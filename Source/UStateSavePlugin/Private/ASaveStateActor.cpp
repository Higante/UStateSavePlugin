#include "ASaveStateActor.h"

#include "Engine/StaticMeshActor.h"
#include "FileManagerGeneric.h"
#include "ROSBridgeGameInstance.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "USaveState.h"

ASaveStateActor::ASaveStateActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Standard Actor to add and serialize
	ClassesToSave.Add(AStaticMeshActor::StaticClass());
}

void ASaveStateActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if WITH_EDITOR
	// Debug Section
	if (!bDebug)
		return;
	
	if (bSave)
	{
		UE_LOG(LogTemp, Warning, TEXT("Debug: Saving Test"));
		RosCallSave();
		bSave = false;
	}

	if (bLoad)
	{
		UE_LOG(LogTemp, Warning, TEXT("Debug: Loading Test"));
		RosCallLoad();
		bLoad = false;
	}
#endif
}

void ASaveStateActor::BeginPlay()
{
	Super::BeginPlay();

	// Make ROS Game Instance
	const UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetGameInstance());
	
	// Add new Services to the ROSHandler
	SaveService = MakeShareable<FROSSaveStateLevel>(new FROSSaveStateLevel(SaveServiceTopic, TEXT("std_srvs/Trigger")));
	LoadService = MakeShareable<FROSLoadStateLevel>(new FROSLoadStateLevel(LoadServiceTopic, TEXT("std_srvs/Trigger")));

	ActiveGameInstance->ROSHandler->AddServiceServer(SaveService);
	ActiveGameInstance->ROSHandler->AddServiceServer(LoadService);
	ActiveGameInstance->ROSHandler->Process();

	// Bind Delegates
	SaveService->OnRosCallback.BindUObject(this, &ASaveStateActor::RosCallSave);
	LoadService->OnRosCallback.BindUObject(this, &ASaveStateActor::RosCallLoad);
}

/**
 * Function responsible saving the State of the current running instance, into a format to be saved.
 *
 * @param FileName FString Input saying which file to save.
 * @param FilePath Optional. FString Indicating the path to the Folder holding the Savesfiles.
 */
void ASaveStateActor::SaveState(FString FileName, FString FilePath)
{
	TArray<uint8> DataToSave = TArray<uint8>();
	int32 AmountOfItemsSaved = 0;

	SavedState = NewObject<USaveState>();
	SavedState->Save(GetWorld(), ClassesToSave);
	DataToSave = SavedState->SerializeState(AmountOfItemsSaved);

	FBufferArchive SaveData(true);
	FName WorldName = GetWorld()->GetFName();
	SaveData << WorldName;
	SaveData << AmountOfItemsSaved;
	SaveData << DataToSave;
	const FString FileToSaveOn = FilePath + GetWorld()->GetName() + "_" + FileName + ".sav";
	FFileHelper::SaveArrayToFile(SaveData, *FileToSaveOn);

	SaveData.FlushCache();
	SaveData.Empty();
}

/**
 * Function responsible loading stated File back into a usable state within the Unreal Engine.
 *
 * @param FileName FString Input saying which file to load.
 * @param FilePath Optional. FString Indicating the path to the Folder holding the Savesfiles.
 */
void ASaveStateActor::LoadState(FString FileName, FString FilePath)
{
	UE_LOG(LogTemp, Warning, TEXT("%s: Begin loading State!"), TEXT(__FUNCTION__));
	SavedState = NewObject<USaveState>();

	const FString FileToLoadPath = FilePath + GetWorld()->GetName() + "_" + FileName + ".sav";
	TArray<uint8> SavedData;
	UE_LOG(LogTemp, Warning, TEXT("%s: Attempt to load..."), TEXT(__FUNCTION__));
	FFileHelper::LoadFileToArray(SavedData, *FileToLoadPath);

	FName WorldName = FName();
	int ItemsSaved = 0;
	TArray<uint8> SaveData = TArray<uint8>();

	FMemoryReader MemoryReader(SavedData, true);
	MemoryReader << WorldName;
	MemoryReader << ItemsSaved;
	MemoryReader << SaveData;

	if (GetWorld()->GetFName() == WorldName)
	{
		// TODO: Add Security. More.
		SavedState->ApplySerializeOnState(SaveData, ItemsSaved);
		SavedState->Load(GetWorld());
		UE_LOG(LogTemp, Warning, TEXT("%s: Save loaded."), TEXT(__FUNCTION__));
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("%s: No save file found compatible with this world."), TEXT(__FUNCTION__));
}

TArray<FString> ASaveStateActor::ListAllSaveFilesAtLocation() const
{
	TArray<FString> OutputArray = TArray<FString>();
	IFileManager::Get().FindFiles(OutputArray, *SaveFilePath, *FString("sav"));

	return OutputArray;
}

/**
 * Delegate Function to be executed if ROS calls for the Level to be saved.
 */
void ASaveStateActor::RosCallSave()
{
	SaveState(SaveSlotName, SaveFilePath);
}

/**
 * Delegate Function to be executed if ROS calls for the level to be saved.
 */
void ASaveStateActor::RosCallLoad()
{
	LoadState(SaveSlotName, SaveFilePath);
}

