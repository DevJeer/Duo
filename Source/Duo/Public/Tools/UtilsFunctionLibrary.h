// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtilsFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class DUO_API UUtilsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static bool OpenFileDialog(const FString& dialogTitle, const FString& defaultPath, const FString& fileTypes, TArray<FString>& outFilenames, bool bAllowMultiSelect);
};
