// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Parser.generated.h"

/**
 * 
 */
UCLASS()
class UNREALSPLAT_API UParser : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "JI20/Parser")
	static FString ParseFilePLY(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage);

};
