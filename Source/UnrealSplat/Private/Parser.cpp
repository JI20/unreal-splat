// Fill out your copyright notice in the Description page of Project Settings.


#include "Parser.h"
#include "HAL/PlatformFileManager.h" // Core
#include "Misc/FileHelper.h" // Core
#include "Misc/Paths.h" // Core

FString UParser::ParseFilePLY(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage) {
	
	// ---- File Reading ----

	FString AbsolutePath = FPaths::ProjectContentDir() + FilePath;
	// 
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*AbsolutePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Reading String failed - File does not exist - %s"), *AbsolutePath);
		return "";
	}

	FString RetString = "";
	if (!FFileHelper::LoadFileToString(RetString, *AbsolutePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Reading String failed - Was not able to read the file - %s"), *AbsolutePath);
		return "";
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Reading String succeeded - %s"), *AbsolutePath);

	// ---- Content Parsing ----

	return RetString;
}

