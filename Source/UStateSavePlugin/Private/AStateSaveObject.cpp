#include "AStateSaveObject.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

#include "FileManagerGeneric.h"
#include "ROSBridgeGameInstance.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "USaveState.h"

AStateSaveObject::AStateSaveObject()
{
	PrimaryActorTick.bCanEverTick = true;

	// Standard Actor to add and serialize
	ClassesToSave.Add(AStaticMeshActor::StaticClass());
}

void AStateSaveObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug Section
	if (!bDebug)
		return;
	
	if (bSave)
	{
		UE_LOG(LogTemp, Warning, TEXT("Debug: Saving Test"));
		SaveState(SaveSlotName, SaveFilePath);
		bSave = false;
	}

	if (bLoad)
	{
		UE_LOG(LogTemp, Warning, TEXT("Debug: Loading Test"));
		LoadState(SaveSlotName, SaveFilePath);
		bLoad = false;
	}
}

void AStateSaveObject::BeginPlay()
{
	Super::BeginPlay();

	// Make ROS Services
	UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetGameInstance());
	
	SaveService = MakeShareable<FROSSaveStateLevel>(new FROSSaveStateLevel(SaveServiceTopic, TEXT("std_srvs/Trigger")));
	LoadService = MakeShareable<FROSLoadStateLevel>(new FROSLoadStateLevel(LoadServiceTopic, TEXT("std_srvs/Trigger")));

	ActiveGameInstance->ROSHandler->AddServiceServer(SaveService);
	ActiveGameInstance->ROSHandler->AddServiceServer(LoadService);
	ActiveGameInstance->ROSHandler->Process();

	// Bind Delegates
	SaveService->OnRosCallsSave.BindUObject(this, &AStateSaveObject::CallSave);
	LoadService->OnRosCallsLoad.BindUObject(this, &AStateSaveObject::CallLoad);
	ListDelegate.BindDynamic(this, &AStateSaveObject::ListAllSaveFilesAtLocation);
}

/**
 * Function responsible saving the State of the current running instance, into a format to be saved.
 *
 * @param FileName FString Input saying which file to save.
 * @param FilePath Optional. FString Indicating the path to the Folder holding the Savesfiles.
 */
void AStateSaveObject::SaveState(FString FileName, FString FilePath)
{
	TArray<uint8> DataToSave = TArray<uint8>();
	TArray<uint8> SaveGameData = TArray<uint8>();
	int AmountOfItemsSaved = 0;

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
void AStateSaveObject::LoadState(FString FileName, FString FilePath)
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

TArray<FString> AStateSaveObject::ListAllSaveFilesAtLocation()
{
	TArray<FString> OutputArray = TArray<FString>();
	IFileManager::Get().FindFiles(OutputArray, *SaveFilePath, *FString("sav"));

	return OutputArray;
}

void AStateSaveObject::CallSave()
{
	SaveState(SaveSlotName, SaveFilePath);
}

void AStateSaveObject::CallLoad()
{
	LoadState(SaveSlotName, SaveFilePath);
}

