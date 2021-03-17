#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "USaveStateInterface.h"
#include "USaveState.generated.h"

// Struct to help save Objects.
USTRUCT(BlueprintType)
struct FSavedObjectInfo
{
	GENERATED_BODY()

public:
	FVector ActorLocation;
	FRotator ActorRotation;
	TArray<FName> Tags;

	FSavedObjectInfo()
	{
		ActorLocation = FVector();
		ActorRotation = FRotator();
		Tags.Init(FName(), 0);
	}

	FSavedObjectInfo(AActor* InputActor)
	{
		ActorLocation = InputActor->GetActorLocation();
		ActorRotation = InputActor->GetActorRotation();
		Tags = InputActor->Tags;
	}

	void SaveObject(AActor* InputActor)
	{
		ActorLocation = InputActor->GetActorLocation();
		ActorRotation = InputActor->GetActorRotation();
		Tags = InputActor->Tags;
	}
};

UCLASS()
class USTATESAVEPLUGIN_API USaveState : public UObject, public ISaveStateInterface
{
	GENERATED_BODY()

public:
	USaveState();

	TArray<UClass*> SavedClasses;
	TMap<FString, FSavedObjectInfo> SavedState;

	TMap<FString, FSavedObjectInfo> GetSavedState();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	void ClearContents();
	void ClearContents_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	bool Save(UWorld* World, TArray<UClass*>& ToSave);
	bool Save_Implementation(UWorld* World, TArray<UClass*>& ToSave) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveState")
	bool Load(UWorld* World);
	bool Load_Implementation(UWorld* World) override;
};