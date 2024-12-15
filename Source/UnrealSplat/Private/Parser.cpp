// Fill out your copyright notice in the Description page of Project Settings.


#include "Parser.h"
#include "Miniply.h"
#include "HAL/PlatformFileManager.h" // Core
#include "Misc/FileHelper.h" // Core
#include "Misc/Paths.h" // Core

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

TArray<FGaussianSplatData> UParser::ParseFilePLY(FString FilePath, bool& bOutSuccess, FString& OutputString) {
	
	// ---- Preparation ----

	FString AbsolutePath = FPaths::ProjectContentDir() + "Models/" + FilePath;
	FString Output = "---- Parsing PLY File ----\n\n";
	TMap<FString, float*> vertexData;
	uint32_t numVertices = 0;
	TArray<FGaussianSplatData> splats;

	// ---- PLY Parsing ----
	
	miniply::PLYReader reader(TCHAR_TO_ANSI(*AbsolutePath));
	
	if (!reader.valid()) {
		bOutSuccess = false;
		OutputString = FString::Printf(TEXT("Parsing PLY failed - Not a valid PLY file - %s"), *AbsolutePath);
		return splats;
	}
	
	FString HeaderLog = FString::Printf(TEXT("ply\nformat %s %d.%d\n"), ANSI_TO_TCHAR(kFileTypes[int(reader.file_type())]),
		reader.version_major(), reader.version_minor());

	// -- Content Parsing --

	for (; reader.has_element() ;reader.next_element()) {
		// - Element (Set of Vertices, Faces, etc.)
		const miniply::PLYElement* elem = reader.element();
		HeaderLog += FString::Printf(TEXT("element %s %u\n"), ANSI_TO_TCHAR(elem->name.c_str()), elem->count);

		// - Read PLY Header
		for (const miniply::PLYProperty& prop : elem->properties) {
			if (prop.countType != miniply::PLYPropertyType::None) {
				HeaderLog += FString::Printf(TEXT("property list %s %s %s\n"), ANSI_TO_TCHAR(kPropertyTypes[uint32_t(prop.countType)]),
					ANSI_TO_TCHAR(kPropertyTypes[uint32_t(prop.type)]), ANSI_TO_TCHAR(prop.name.c_str()));
			}
			else {
				HeaderLog += FString::Printf(TEXT("property %s %s\n"), ANSI_TO_TCHAR(kPropertyTypes[uint32_t(prop.type)]), ANSI_TO_TCHAR(prop.name.c_str()));
			}
		}

		// - Extract Data from Vertices
		if (reader.element_is(miniply::kPLYVertexElement) && reader.load_element()) {
			numVertices = reader.num_rows();
			uint32_t y = 0;
			HeaderLog += "Props Read for Vertices\n";
			for (const miniply::PLYProperty& prop : elem->properties) {
				HeaderLog += FString::Printf(TEXT("Property: %s "), ANSI_TO_TCHAR(prop.name.c_str()));
				// Hier was falsch?
				uint32_t indexes[] = { y };
				float* columnData = new float[numVertices];
				reader.extract_properties(indexes, 1, miniply::PLYPropertyType::Float, columnData);
				vertexData.Add(prop.name.c_str(), columnData);
				y += 1;
			}
		}
	}

	// Only for debugging: Print Values
	uint32_t max_debug_vertices = 10;
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
	}

	HeaderLog += "end_header\n\n";

	// ---- Conversion to Unreal's TArray Representation ----
	
	// TODO: Add Error handling for other ply formats: This code assumes all these properties exist.
	// TODO: Maybe I can delete this loop for performance.
	bool PositionExists = vertexData.Contains("x") && vertexData.Contains("y") && vertexData.Contains("z");
	bool NormalExists = vertexData.Contains("nx") && vertexData.Contains("ny") && vertexData.Contains("nz");
	bool OrientationExists = vertexData.Contains("rot_0") && vertexData.Contains("rot_1") && vertexData.Contains("rot_2") && vertexData.Contains("rot_3");
	bool ScaleExists = vertexData.Contains("scale_0") && vertexData.Contains("scale_1") && vertexData.Contains("scale_2");
	bool OpacityExists = vertexData.Contains("opacity");
	bool ZeroOrderHarmonicsExists= vertexData.Contains("f_dc_0") && vertexData.Contains("f_dc_1") && vertexData.Contains("f_dc_2");;
	bool higherOrderHarmonicsExists = true;
	for (uint32_t y = 0; y < 45; y += 3) {
		FString index = "f_rest_" + FString::FromInt(y);
		if (!vertexData.Contains(index)) {
			higherOrderHarmonicsExists = false;
		}
	}

	for (uint32_t i = 0; i < numVertices; i++) {
		FGaussianSplatData splat;
		if (PositionExists) {
			splat.Position = FVector(vertexData["x"][i], vertexData["y"][i], vertexData["z"][i]);
		}
		if (NormalExists) {
			splat.Normal = FVector(vertexData["nx"][i], vertexData["ny"][i], vertexData["nz"][i]);
		}
		if (OrientationExists) {
			splat.Orientation = FQuat(vertexData["rot_0"][i], vertexData["rot_1"][i], vertexData["rot_2"][i], vertexData["rot_3"][i]);
		}
		if (ScaleExists) {
			splat.Scale = FVector(vertexData["scale_0"][i], vertexData["scale_1"][i], vertexData["scale_2"][i]);
		}
		if (OpacityExists) {
			splat.Opacity = vertexData["opacity"][i];
		}
		if (ZeroOrderHarmonicsExists) {
			splat.ZeroOrderHarmonicsCoefficients = FVector(vertexData["f_dc_0"][i], vertexData["f_dc_1"][i], vertexData["f_dc_2"][i]);
		}
		if (higherOrderHarmonicsExists) {
			TArray<FVector> higherOrderHarmonics;
			for (uint32_t y = 0; y < 45; y += 3) {
				FString index1 = "f_rest_" + FString::FromInt(y);
				FString index2 = "f_rest_" + FString::FromInt(y+1);
				FString index3 = "f_rest_" + FString::FromInt(y+2);

				higherOrderHarmonics.Add(FVector(vertexData[index1][i], vertexData[index2][i], vertexData[index3][i]));
			}
			splat.HighOrderHarmonicsCoefficients = higherOrderHarmonics;
		}

		splats.Add(splat);
	}

	// ---- Finishing up ----

	bOutSuccess = true;

	Output += FString::Printf(TEXT("Successfully parsed PLY File - %s\n\n-- PLY Header --\n\n"), *AbsolutePath);
	Output += HeaderLog;
	Output += "-- End of PLY Header --\n\n";
	Output += "-- PLY Body --\n\n";
	Output += "-- End of PLY Body --\n\n";
	Output += "";
	Output += "---- Finished Parsing PLY File ----";
	OutputString = Output;

	return splats;
}

