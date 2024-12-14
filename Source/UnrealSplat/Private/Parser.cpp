// Fill out your copyright notice in the Description page of Project Settings.


#include "Parser.h"
#include "Miniply.h"
#include "HAL/PlatformFileManager.h" // Core
#include "Misc/FileHelper.h" // Core
#include "Misc/Paths.h" // Core

FString UParser::ParseFilePLY(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage) {
	const char* kFileTypes[] = {
	  "ascii",
	  "binary_little_endian",
	  "binary_big_endian",
	};

	const char* kPropertyTypes[] = {
	  "char",
	  "uchar",
	  "short",
	  "ushort",
	  "int",
	  "uint",
	  "float",
	  "double",
	};
	
	// ---- PLY Parsing ----

	FString AbsolutePath = FPaths::ProjectContentDir() + "Models/" + FilePath;
	FString HeaderLog = "";
	FString Output = "---- Parsing PLY File ----\n\n";
	
	miniply::PLYReader reader(TCHAR_TO_ANSI(*AbsolutePath));
	
	if (!reader.valid()) {
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Parsing PLY failed - Not a valid PLY file - %s"), *AbsolutePath);
		Output = OutInfoMessage;
		return Output;
	}

	// ---- Content: Parse Header of file ----
	HeaderLog += "ply\n";
	HeaderLog += FString::Printf(TEXT("format %s %d.%d\n"), ANSI_TO_TCHAR(kFileTypes[int(reader.file_type())]),
		reader.version_major(), reader.version_minor());
	for (uint32_t i = 0, endI = reader.num_elements(); i < endI; i++) {
		const miniply::PLYElement* elem = reader.get_element(i);
		HeaderLog += FString::Printf(TEXT("element %s %u\n"), ANSI_TO_TCHAR(elem->name.c_str()), elem->count);

		for (const miniply::PLYProperty& prop : elem->properties) {
			if (prop.countType != miniply::PLYPropertyType::None) {
				HeaderLog += FString::Printf(TEXT("property list %s %s %s\n"), ANSI_TO_TCHAR(kPropertyTypes[uint32_t(prop.countType)]),
					ANSI_TO_TCHAR(kPropertyTypes[uint32_t(prop.type)]), ANSI_TO_TCHAR(prop.name.c_str()));
			}
			else {
				HeaderLog += FString::Printf(TEXT("property %s %s\n"), ANSI_TO_TCHAR(kPropertyTypes[uint32_t(prop.type)]), ANSI_TO_TCHAR(prop.name.c_str()));
			}
		}
	}
	HeaderLog += "end_header\n\n";

	// ---- Finish up ----

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Successfully parsed PLY File - %s\n\n-- PLY Header --\n\n"), *AbsolutePath);
	Output += OutInfoMessage;
	Output += HeaderLog;
	Output += "-- End of PLY Header --\n\n";
	Output += "---- Finished Parsing PLY File ----";

	return Output;
}

