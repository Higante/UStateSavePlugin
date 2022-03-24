#include "ASaveStateActor.h"

#include "FileHelper.h"
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

	SavedState = NewObject<USaveState>();
}

void ASaveStateActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHasLoadedLastTick)
	{
		for (AActor* RefreshingActor : ActorsToRefreshOnTick)
		{
			if (UPrimitiveComponent* RefRootC = Cast<UPrimitiveComponent>(RefreshingActor->GetRootComponent()))
			{
				RefRootC->SetSimulatePhysics(RefRootC->IsSimulatingPhysics());
			}
		}

		bHasLoadedLastTick = false;
		ActorsToRefreshOnTick.Empty();
	}

#if WITH_EDITOR
	// Debug Section
	if (!bDebug)
		return;
	
	if (bSave)
	{
		UE_LOG(LogTemp, Warning, TEXT("Debug: Saving Test"));
		RosCallSave(SaveSlotName);
		bSave = false;
	}

	if (bLoad)
	{
		UE_LOG(LogTemp, Warning, TEXT("Debug: Loading Test"));
		RosCallLoad(SaveSlotName);
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
	SaveService = MakeShareable<FROSSaveStateLevel>(new FROSSaveStateLevel(SaveServiceTopic, TEXT("world_control_msgs/DeleteModel")));
	LoadService = MakeShareable<FROSLoadStateLevel>(new FROSLoadStateLevel(LoadServiceTopic, TEXT("world_control_msgs/DeleteModel")));

	ActiveGameInstance->ROSHandler->AddServiceServer(SaveService);
	ActiveGameInstance->ROSHandler->AddServiceServer(LoadService);
	ActiveGameInstance->ROSHandler->Process();

	// Bind Delegates
	SaveService->OnRosCallback.BindUObject(this, &ASaveStateActor::RosCallSave);
	LoadService->OnRosCallback.BindUObject(this, &ASaveStateActor::RosCallLoad);
}

void ASaveStateActor::SaveStateCurrentWorld(const FString FileName, const FString FilePath)
{
	int32 AmountOfItemsSaved = 0;

	SavedState = NewObject<USaveState>();
	SavedState->SaveFromWorld(GetWorld(), ClassesToSave);
	TArray<uint8> ByteArray = SavedState->SerializeState(AmountOfItemsSaved);

	FBufferArchive SaveData(true);
	FName WorldName = GetWorld()->GetFName();
	SaveData << WorldName;
	SaveData << AmountOfItemsSaved;
	SaveData << ByteArray;
	
	const FString FileToSaveOn = FilePath + GetWorld()->GetName() + "_" + FileName + ".sav";
	FFileHelper::SaveArrayToFile(SaveData, *FileToSaveOn);

	SaveData.FlushCache();
	SaveData.Empty();
}

void ASaveStateActor::LoadStateOntoCurrentLevel(const FString FileName, const FString FilePath)
{
	SavedState = NewObject<USaveState>();

	const FString FileToLoadPath = FilePath + GetWorld()->GetName() + "_" + FileName + ".sav";
	TArray<uint8> SavedData;
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
		SavedState->ApplySerializeOnState(SaveData, ItemsSaved);
		ActorsToRefreshOnTick = SavedState->LoadOntoWorld(GetWorld());
		bHasLoadedLastTick = true;
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

void ASaveStateActor::RosCallSave(FString InFileName)
{
	FFunctionGraphTask::CreateAndDispatchWhenReady([this, InFileName]()
	{
		SaveStateCurrentWorld(InFileName, SaveFilePath);
	}, TStatId(), nullptr, ENamedThreads::GameThread);
}

void ASaveStateActor::RosCallLoad(FString InFileName)
{
	FFunctionGraphTask::CreateAndDispatchWhenReady([this, InFileName]()
	{
		LoadStateOntoCurrentLevel(InFileName, SaveFilePath);
	}, TStatId(), nullptr, ENamedThreads::GameThread);
}

