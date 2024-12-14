// Fill out your copyright notice in the Description page of Project Settings.


#include "Parser.h"
#include "Miniply.h"
#include "HAL/PlatformFileManager.h" // Core
#include "Misc/FileHelper.h" // Core
#include "Misc/Paths.h" // Core

FString UParser::ParseFilePLY(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage, TArray<FGaussianSplatData>& GaussianSplatData) {
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
	
	// ---- File ----

	FString AbsolutePath = FPaths::ProjectContentDir() + "Models/" + FilePath;
	FString HeaderLog = "";
	FString Output = "---- Parsing PLY File ----\n\n";
	
	// ---- PLY Parsing ----
	TMap<FString, float*> vertexData;
	uint32_t numVertices = 0;
	
	miniply::PLYReader reader(TCHAR_TO_ANSI(*AbsolutePath));
	
	if (!reader.valid()) {
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Parsing PLY failed - Not a valid PLY file - %s"), *AbsolutePath);
		Output = OutInfoMessage;
		return Output;
	}

	// -- Content: Parse Header of file --
	HeaderLog += "ply\n";
	HeaderLog += FString::Printf(TEXT("format %s %d.%d\n"), ANSI_TO_TCHAR(kFileTypes[int(reader.file_type())]),
		reader.version_major(), reader.version_minor());
	for (uint32_t i = 0, endI = reader.num_elements(); i < endI; i++) {
		const miniply::PLYElement* elem = reader.get_element(i);
		HeaderLog += FString::Printf(TEXT("element %s %u\n"), ANSI_TO_TCHAR(elem->name.c_str()), elem->count);

		// Properties
		uint32_t numProperties = 0;
		for (const miniply::PLYProperty& prop : elem->properties) {
			numProperties++;
			if (prop.countType != miniply::PLYPropertyType::None) {
				HeaderLog += FString::Printf(TEXT("property list %s %s %s\n"), ANSI_TO_TCHAR(kPropertyTypes[uint32_t(prop.countType)]),
					ANSI_TO_TCHAR(kPropertyTypes[uint32_t(prop.type)]), ANSI_TO_TCHAR(prop.name.c_str()));
			}
			else {
				HeaderLog += FString::Printf(TEXT("property %s %s\n"), ANSI_TO_TCHAR(kPropertyTypes[uint32_t(prop.type)]), ANSI_TO_TCHAR(prop.name.c_str()));
			}
		}

		// Extract Data from vertices

		if (reader.element_is(miniply::kPLYVertexElement) && reader.load_element()) {
			numVertices = reader.num_rows();
			uint32_t y = 0;
			for (const miniply::PLYProperty& prop : elem->properties) {
				uint32_t indexes[] = { y };
				float* columnData = new float[numVertices];
				reader.extract_properties(indexes, 1, miniply::PLYPropertyType::Float, columnData);
				vertexData.Add(prop.name.c_str(), columnData);
				y++;
			}
		}
	}

	// Only for debugging: Print Values
	/**
	uint32_t max_debug_vertices = 0;
	for (auto& Elem : vertexData) {
		uint32_t x = 0;
		HeaderLog += Elem.Key + "\n";
		for (uint32_t n = 0; n < numVertices; n++) {
			if (x >= max_debug_vertices) {
				break;
			}
			HeaderLog += FString::FromInt(int(Elem.Value[n])) + " ";
			x++;
		}
		HeaderLog += "\n";
	}*/

	HeaderLog += "end_header\n\n";

	// ---- Create TArray Representation ----
	// TODO: Add Error handling for other ply formats: This code assumes all these properties exist.
	// Maybe I can delete this loop for performance.
	TArray<FGaussianSplatData> splats;

	for (uint32_t i = 0; i < numVertices; i++) {
		FGaussianSplatData splat;
		splat.Position = FVector(vertexData["x"][i], vertexData["y"][i], vertexData["z"][i]);
		splat.Normal = FVector(vertexData["nx"][i], vertexData["ny"][i], vertexData["nz"][i]);
		splat.Orientation = FQuat(vertexData["rot_0"][i], vertexData["rot_1"][i], vertexData["rot_2"][i], vertexData["rot_3"][i]);
		splat.Scale = FVector(vertexData["scale_0"][i], vertexData["scale_1"][i], vertexData["scale_2"][i]);
		splat.Opacity = vertexData["opacity"][i];
		splat.ZeroOrderHarmonicsCoefficients = FVector(vertexData["f_dc_0"][i], vertexData["f_dc_1"][i], vertexData["f_dc_2"][i]);
		
		TArray<FVector> higherOrderHarmonics;
		for (uint32_t y = 0; y < 45; y += 3) {
			FString index1 = "f_rest_" + FString::FromInt(y);
			FString index2 = "f_rest_" + FString::FromInt(y+1);
			FString index3 = "f_rest_" + FString::FromInt(y+2);

			higherOrderHarmonics.Add(FVector(vertexData[index1][i], vertexData[index2][i], vertexData[index3][i]));
		}
		splat.HighOrderHarmonicsCoefficients = higherOrderHarmonics;

		splats.Add(splat);
	}

	GaussianSplatData = splats;

	// ---- Finish up ----

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Successfully parsed PLY File - %s\n\n-- PLY Header --\n\n"), *AbsolutePath);
	Output += OutInfoMessage;
	Output += HeaderLog;
	Output += "-- End of PLY Header --\n\n";
	Output += "-- PLY Body --\n\n";
	Output += "-- End of PLY Body --\n\n";
	Output += "";
	Output += "---- Finished Parsing PLY File ----";

	return Output;
}

