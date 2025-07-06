// Fill out your copyright notice in the Description page of Project Settings.


#include "Parser.h"
#include "Miniply.h"
#include "HAL/PlatformFileManager.h" // Core
#include "Misc/FileHelper.h" // Core
#include "Misc/Paths.h" // Core
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "Kismet/GameplayStatics.h" // Include for accessing editor utilities
#include "Engine/Texture2D.h"
#include "PixelFormat.h" // For EPixelFormat
#include "Engine/TextureDefines.h" // For TextureMipGenSettings
#include "ImageUtils.h" // Not strictly needed for FLinearColor, but good for general image utilities.
#include "Math/UnrealMathUtility.h" // For FMath::Memcpy

// ---------- Constants ----------

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

const float C0 = 0.28209479177387814;

struct FGaussianSplattingTextureData {
	TArray<FLinearColor> PositionTextureData;
	TArray<FLinearColor> ScaleTextureData;
	TArray<FLinearColor> RotationTextureData;
	TArray<FLinearColor> ColorTextureData;
	TArray<FLinearColor> harmonicsL1TextureData;
	TArray<FLinearColor> harmonicsL2TextureData;
	TArray<FLinearColor> harmonicsL31TextureData;
	TArray<FLinearColor> harmonicsL32TextureData;

	FGaussianSplattingTextureData()
		: PositionTextureData()
		, ScaleTextureData()
		, RotationTextureData()
		, ColorTextureData()
		, harmonicsL1TextureData()
		, harmonicsL2TextureData()
		, harmonicsL31TextureData()
		, harmonicsL32TextureData()
	{
	}
};

// ---------- Private Helper Functions ----------

static FString CreateAndSaveTexture(
	const FString& InPackagePath,
	const FString& InTextureName,
	int32 Width,
	int32 Height,
	TArray<FLinearColor>& InPixelData
	) {
	
	// --- 2. Determine Package and Asset Paths ---
	FString PackagePath = FPaths::Combine(FPackageName::FilenameToLongPackageName(InPackagePath), InTextureName);
	// Ensure the package path ends with the asset name for proper asset naming conventions
	// Example: /Game/MyTextures/MyGeneratedTexture.MyGeneratedTexture
	FString FullAssetPath = PackagePath + "." + InTextureName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create package: %s"), *PackagePath);
		return "";
	}

	UTexture2D* NewTexture = NewObject<UTexture2D>(Package, FName(*InTextureName), RF_Public | RF_Standalone | RF_MarkAsNative);
	if (!NewTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create UTexture2D object: %s"), *InTextureName);
		return "";
	}

	// Texture Properties

	NewTexture->SRGB = false;
	NewTexture->MipGenSettings = TMGS_NoMipmaps;
	NewTexture->NeverStream = false;
	NewTexture->CompressionNone = true;
	NewTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
	NewTexture->Filter = TF_Nearest;
	
	// Persistent Texture is stored into Source

	NewTexture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_RGBA32F);
	void* MipData = NewTexture->Source.LockMip(0);
	FMemory::Memcpy(MipData, InPixelData.GetData(), InPixelData.Num() * sizeof(FLinearColor));
	NewTexture->Source.UnlockMip(0);

	// Transient Texture

	/*NewTexture->SetPlatformData(new FTexturePlatformData());
	NewTexture->GetPlatformData()->SizeX = Width;
	NewTexture->GetPlatformData()->SizeY = Height;
	//NewTexture->GetPlatformData()->SetNumSlices(1);
	NewTexture->GetPlatformData()->PixelFormat = EPixelFormat::PF_A32B32G32R32F;

	// Mip Map Level 0
	FTexture2DMipMap* Mip = new FTexture2DMipMap(Width, Height,1);
	NewTexture->GetPlatformData()->Mips.Add(Mip);

	// Allocate first mipmap.

	// Bulk-Daten zum Schreiben sperren.
	Mip->BulkData.Lock(LOCK_READ_WRITE);

	void* DestPixels = Mip->BulkData.Realloc(InPixelData.Num() * sizeof(FLinearColor));


	// Überprüfen, ob die Speicherzuweisung erfolgreich war.

	// Daten in die Textur kopieren
	FMemory::Memcpy(DestPixels, InPixelData.GetData(), InPixelData.Num() * sizeof(FLinearColor));

	Mip -> BulkData.Unlock();
	// Set Platform Meta Data of Texture*/
	
	NewTexture->UpdateResource();

	NewTexture->PostEditChange();

	// Saving to Disk
	Package->MarkPackageDirty();

	//const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FString(TEXT(".uasset")));

	//bool bSuccess = UPackage::SavePackage(Package, NewTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GLog);
	bool bSuccess = UEditorAssetLibrary::SaveLoadedAsset(NewTexture, true);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully created and saved texture asset: %s"), *PackagePath);
		return NewTexture->GetPathName();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save texture asset: %s"), *PackagePath);
		// Clean up partially created asset if save failed to prevent stale references.
		NewTexture->MarkAsGarbage();
		return "";
	}

	// Notify Asset Registry
	//FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	//AssetRegistryModule.Get().AssetCreated(NewTexture);
	//UEditorAssetLibrary::SaveAsset(FullAssetPath, true);
	//return PackagePath;
}

static FString CreateDirectory(FString Path) {
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	UE_LOG(LogTemp, Log, TEXT("This is the Path: %s"), *Path);
	FString AbsoluteFilePath = Path;
	int i = 0;

	while (PlatformFile.DirectoryExists(*AbsoluteFilePath)) {
		AbsoluteFilePath = Path + FString::FromInt(i);
		i++;
	}

	// 1. Create Physical Folder
	if (PlatformFile.CreateDirectory(*AbsoluteFilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Created physical directory: %s"), *AbsoluteFilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create physical directory: %s"), *AbsoluteFilePath);
	}

	// 2. Notify Unreal Engine Asset Manager
	FString ContentBrowserPath = FPackageName::FilenameToLongPackageName(AbsoluteFilePath);
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().AddPath(ContentBrowserPath);

	return AbsoluteFilePath;
}

static UTexture2D* CreateTexture(int32 width, int32 height, const FString& Name) {
	UTexture2D* NewTexture = UTexture2D::CreateTransient(width, height, EPixelFormat::PF_A32B32G32R32F, FName(*Name));

	if (!NewTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create transient texture: %s"), *Name);
		return nullptr;
	}

	NewTexture->SRGB = false;
	NewTexture->MipGenSettings = TMGS_NoMipmaps;
	NewTexture->NeverStream = true;
	NewTexture->CompressionNone = true; 
	NewTexture->CompressionSettings = TextureCompressionSettings::TC_Default;

	// Ensure the texture resource is created on the rendering thread.
	NewTexture->UpdateResource();

	return NewTexture;
}

static int getGridIndex(FVector position, FVector boundsMin, FVector boundsMax, int cellsPerEdge) {
	if (cellsPerEdge == 1) {
		return 0;
	}

	// Calculate the grid cell index in 3d
	// 1. Calculate the size of the bounding box defined by boundsMin and boundsMax
	FVector BoxSize = boundsMax - boundsMin;

	// 2. Translate the 'position' relative to the 'boundsMin' corner
	//    This shifts the origin of our coordinate system to the min corner of the box.
	FVector RelativePosition = position - boundsMin;

	// 3. Normalize the relative position to a [0, 1] range for each axis
	//    Divide by the box size. Be careful with division by zero.
	FVector NormalizedPosition = FVector::ZeroVector;
	if (BoxSize.X > KINDA_SMALL_NUMBER) NormalizedPosition.X = RelativePosition.X / BoxSize.X;
	if (BoxSize.Y > KINDA_SMALL_NUMBER) NormalizedPosition.Y = RelativePosition.Y / BoxSize.Y;
	if (BoxSize.Z > KINDA_SMALL_NUMBER) NormalizedPosition.Z = RelativePosition.Z / BoxSize.Z;

	// 4. Scale the normalized position by cellsPerEdge to get floating-point cell coordinates
	FVector ScaledPosition = NormalizedPosition * (float)cellsPerEdge;

	// 5. Floor the scaled position to get the integer 3D grid cell index
	int32 CellX = FMath::FloorToInt(ScaledPosition.X);
	int32 CellY = FMath::FloorToInt(ScaledPosition.Y);
	int32 CellZ = FMath::FloorToInt(ScaledPosition.Z);

	// 6. Clamp the cell indices to be within the valid range [0, cellsPerEdge - 1]
	//    This is crucial for positions slightly outside the box or exactly on max boundaries.
	CellX = FMath::Clamp(CellX, 0, cellsPerEdge - 1);
	CellY = FMath::Clamp(CellY, 0, cellsPerEdge - 1);
	CellZ = FMath::Clamp(CellZ, 0, cellsPerEdge - 1);

	int LinearIndex = CellZ * (cellsPerEdge * cellsPerEdge) + CellY * cellsPerEdge + CellX;

	return LinearIndex;
}

bool PopulateGaussianTexture(UTexture2D* Texture, const TArray<FLinearColor>& DataArray, int32 InSizeX, int32 InSizeY)
{
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid texture or platform data for population."));
		return false;
	}
	// Get the first mipmap (since mipmaps are disabled, this is the only one)
	FTexture2DMipMap Mip = Texture->GetPlatformData()->Mips[0]; // [2, 3]
	// Calculate expected data size in bytes (4 floats per pixel, 4 bytes per float)
	const int32 ExpectedDataSize = InSizeX * InSizeY * sizeof(FLinearColor);
	// Lock the bulk data for writing
	void* LockedData = Mip.BulkData.Lock(LOCK_READ_WRITE); // [2, 3]
	if (!LockedData)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to lock bulk data for texture: %s"), *Texture->GetName());
		return false;
	}
	// Cast to FLinearColor* for direct access to 32-bit float RGBA pixels
	FLinearColor* DestPixels = static_cast<FLinearColor*>(LockedData);
	// Copy data from our prepared array to the texture's bulk data
	FMemory::Memcpy(DestPixels, DataArray.GetData(), ExpectedDataSize); // [4, 5]
	// Unlock the bulk data
	Mip.BulkData.Unlock(); // [2, 3]
	// Update the texture resource on the rendering thread to push changes to GPU
	Texture->UpdateResource(); //
	return true;
}

// ---------- Public Class Functions ----------

int UParser::Preprocess3DGSModel(FString FilePath, bool& bOutSuccess, FString& OutputString, TArray<FTextureLocations>& TexLocations) {
	// ----- Prepare Parsing -----

	FString AbsolutePath = FPaths::ProjectContentDir() + "Models/" + FilePath;
	FString Output = "---- Parsing PLY File ----\n\n";
	TMap<FString, float*> vertexData;
	uint32_t numVertices = 0;

	// ----- Parsing -----
	// -- TODO: Determine File Type --
	// -- Check Validity --
	miniply::PLYReader reader(TCHAR_TO_ANSI(*AbsolutePath));

	if (!reader.valid()) {
		bOutSuccess = false;
		OutputString = FString::Printf(TEXT("Parsing PLY failed - Not a valid PLY file - %s"), *AbsolutePath);
		return -1;
	}

	// -- Content Parsing --
	FString HeaderLog = FString::Printf(TEXT("ply\nformat %s %d.%d\n"), ANSI_TO_TCHAR(kFileTypes[int(reader.file_type())]),
		reader.version_major(), reader.version_minor());

	for (; reader.has_element(); reader.next_element()) {
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
			for (const miniply::PLYProperty& prop : elem->properties) {
				uint32_t indexes[] = { y };
				float* columnData = new float[numVertices];
				reader.extract_properties(indexes, 1, miniply::PLYPropertyType::Float, columnData);
				vertexData.Add(prop.name.c_str(), columnData);
				y += 1;
			}
		}
	}

	HeaderLog += "end_header\n\n";

	// ---- Process Model Data ----

	// -- Check Model Validity --
	bool PositionExists = vertexData.Contains("x") && vertexData.Contains("y") && vertexData.Contains("z");
	bool OrientationExists = vertexData.Contains("rot_0") && vertexData.Contains("rot_1") && vertexData.Contains("rot_2") && vertexData.Contains("rot_3");
	bool ScaleExists = vertexData.Contains("scale_0") && vertexData.Contains("scale_1") && vertexData.Contains("scale_2");
	bool OpacityExists = vertexData.Contains("opacity");
	bool ZeroOrderHarmonicsExists = vertexData.Contains("f_dc_0") && vertexData.Contains("f_dc_1") && vertexData.Contains("f_dc_2");;
	bool higherOrderHarmonicsExists = true;
	for (uint32_t y = 0; y < 45; y += 3) {
		FString index = "f_rest_" + FString::FromInt(y);
		if (!vertexData.Contains(index)) {
			higherOrderHarmonicsExists = false;
		}
	}
	if (!(PositionExists && OrientationExists && ScaleExists && OpacityExists && ZeroOrderHarmonicsExists)) {
		return -1;
	}

	// -- Calculate Bounding Boxes --
	// Respect Unreal Engine Position Conversions for Position Values 100.0f * (x, -z, -y)
	float min_x = vertexData["x"][0];
	float min_y = -vertexData["z"][0];
	float min_z = -vertexData["y"][0];

	float max_x = vertexData["x"][0];
	float max_y = -vertexData["z"][0];
	float max_z = -vertexData["y"][0];

	for (uint32_t i = 0; i < numVertices; i++) {
		if (vertexData["x"][i] < min_x) {
			min_x = vertexData["x"][i];
		}
		else if (vertexData["x"][i] > max_x) {
			max_x = vertexData["x"][i];
		}

		if (-vertexData["z"][i] < min_y) {
			min_y = -vertexData["z"][i];
		}
		else if (-vertexData["z"][i] > max_y) {
			max_y = -vertexData["z"][i];
		}

		if (-vertexData["y"][i] < min_z) {
			min_z = -vertexData["y"][i];
		}
		else if (-vertexData["y"][i] > max_z) {
			max_z = -vertexData["y"][i];
		}
	}

	FVector BoundsMin = 100.0f * FVector(min_x, min_y, min_z);
	FVector BoundsMax = 100.0f * FVector(max_x, max_y, max_z);
	float BoxEdgeLengthX = 100.0f * (max_x - min_x);
	float BoxEdgeLengthY = 100.0f * (max_y - min_y);
	float BoxEdgeLengthZ = 100.0f * (max_z - min_y);

	// -- Divide Model into Emitter Grid --
	// Bounding box is divided into equally sized grid
	// Calculate how many cells per edge are needed
	int CellsPerEdge = 1;

	if (numVertices > 3500000) {
		CellsPerEdge = 4;

	}
	else if (numVertices > 1500000) {
		CellsPerEdge = 2;

	}

	int GridCells = CellsPerEdge * CellsPerEdge * CellsPerEdge;

	// -- Create Folder Structure in Game --
	FString ModelFolderPath = FPaths::ProjectContentDir() + "Models/" + FPaths::GetBaseFilename(FilePath);
	ModelFolderPath = CreateDirectory(ModelFolderPath);
	FString EmitterFolderPath = ModelFolderPath + "/Emitters";
	EmitterFolderPath = CreateDirectory(EmitterFolderPath);

	// Process Data and Prepare Pixel Values and sort them into the correct emitter
	TArray<FGaussianSplattingTextureData> TextureDataPerEmitter; // List of PixelData Arrays per Emitter
	for (int i = 0; i < GridCells; i++) {
		FGaussianSplattingTextureData textureData;
		TextureDataPerEmitter.Add(textureData);
	}

	// Process splats
	for (uint32_t i = 0; i < numVertices; i++) {
		// Map to Emitter Grid based on Position
		FVector Position = 100.0f * FVector(vertexData["x"][i], -vertexData["z"][i], -vertexData["y"][i]);
		int currentEmitterIndex = getGridIndex(Position, BoundsMin, BoundsMax, CellsPerEdge);

		// Positions
		FLinearColor PositionPixel = 100.0f * FLinearColor(vertexData["x"][i], -vertexData["z"][i], -vertexData["y"][i]);
		TextureDataPerEmitter[currentEmitterIndex].PositionTextureData.Add(PositionPixel);
		
		// Scales
		FLinearColor ScalePixel = 100.0f * FLinearColor(FMath::Exp(vertexData["scale_0"][i]), FMath::Exp(vertexData["scale_2"][i]), FMath::Exp(vertexData["scale_1"][i]));
		TextureDataPerEmitter[currentEmitterIndex].ScaleTextureData.Add(ScalePixel);

		// Rotation
		FQuat Rot = FQuat(vertexData["rot_1"][i], vertexData["rot_2"][i], vertexData["rot_3"][i], vertexData["rot_0"][i]); // Normalize Quaternion
		Rot.Normalize();
		FLinearColor RotationPixel = FLinearColor(Rot.X, -Rot.Z, -Rot.Y, Rot.W);
		TextureDataPerEmitter[currentEmitterIndex].RotationTextureData.Add(RotationPixel);

		// BaseColor and Opacity
		FVector ZeroOrderHarmonics = FVector(vertexData["f_dc_0"][i], vertexData["f_dc_1"][i], vertexData["f_dc_2"][i]);
		FLinearColor BaseColor = FLinearColor(0.5 + C0 * ZeroOrderHarmonics.X, 0.5 + C0 * ZeroOrderHarmonics.Y, 0.5 + C0 * ZeroOrderHarmonics.Z);
		
		float Opacity = FMath::Clamp(1.0f / (1.0f + FMath::Exp(-vertexData["opacity"][i])), 0.0f, 1.0f); // Apply Sigmoid Function
		
		FLinearColor ColorPixel = FLinearColor(BaseColor.R, BaseColor.G, BaseColor.B, Opacity);
		TextureDataPerEmitter[currentEmitterIndex].ColorTextureData.Add(ColorPixel);

		// Higher Order Harmonics
		// L1 - 3 Pixel per Gaussian
		if (higherOrderHarmonicsExists) {
			for (uint32_t y = 0; y < 9; y += 3) {
				FString index1 = "f_rest_" + FString::FromInt(y);
				FString index2 = "f_rest_" + FString::FromInt(y + 1);
				FString index3 = "f_rest_" + FString::FromInt(y + 2);
				TextureDataPerEmitter[currentEmitterIndex].harmonicsL1TextureData.Add(FLinearColor(vertexData[index1][i], vertexData[index2][i], vertexData[index3][i]));
			}
			// L2 - 5 Pixel per Gaussian
			for (uint32_t y = 9; y < 24; y += 3) {
				FString index1 = "f_rest_" + FString::FromInt(y);
				FString index2 = "f_rest_" + FString::FromInt(y + 1);
				FString index3 = "f_rest_" + FString::FromInt(y + 2);
				TextureDataPerEmitter[currentEmitterIndex].harmonicsL2TextureData.Add(FLinearColor(vertexData[index1][i], vertexData[index2][i], vertexData[index3][i]));
			}

			// L3 - 7 Pixel per Gaussian (divided into 4 and 3)
			for (uint32_t y = 24; y < 36; y += 3) {
				FString index1 = "f_rest_" + FString::FromInt(y);
				FString index2 = "f_rest_" + FString::FromInt(y + 1);
				FString index3 = "f_rest_" + FString::FromInt(y + 2);
				TextureDataPerEmitter[currentEmitterIndex].harmonicsL31TextureData.Add(FLinearColor(vertexData[index1][i], vertexData[index2][i], vertexData[index3][i]));
			}

			for (uint32_t y = 36; y < 45; y += 3) {
				FString index1 = "f_rest_" + FString::FromInt(y);
				FString index2 = "f_rest_" + FString::FromInt(y + 1);
				FString index3 = "f_rest_" + FString::FromInt(y + 2);
				TextureDataPerEmitter[currentEmitterIndex].harmonicsL32TextureData.Add(FLinearColor(vertexData[index1][i], vertexData[index2][i], vertexData[index3][i]));
			}
		}
	}

	// Create Emitter Folder and Textures per Emitter
	for (int i = 0; i < GridCells; i++) {
		FTextureLocations TextureLocations;
		// Create Emitter Folder
		FString NewEmitterFolder = EmitterFolderPath / FString::FromInt(i);
		NewEmitterFolder = CreateDirectory(NewEmitterFolder);
		// Create Emitter Description

		// TODO: Emitter Description File

		// Create, Populate and Store Textures per Emitter
		
		int numPixels = TextureDataPerEmitter[i].PositionTextureData.Num();
		if (numPixels == 0) {
			continue;
		}
		else if (numPixels <= 100) {
			continue;
		}
		float TextureWidth = ceil(sqrt(numPixels));
		float TextureHeight = ceil(numPixels / TextureWidth);
		FString PositionAssetPath = CreateAndSaveTexture(NewEmitterFolder, "positiontexture",TextureWidth, TextureHeight, TextureDataPerEmitter[i].PositionTextureData);
		TextureLocations.PositionTextureLocation = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(PositionAssetPath));

		FString ColorAssetPath = CreateAndSaveTexture(NewEmitterFolder, "colortexture", TextureWidth, TextureHeight, TextureDataPerEmitter[i].ColorTextureData);
		TextureLocations.ColorTextureLocation = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(ColorAssetPath));

		FString ScaleAssetPath = CreateAndSaveTexture(NewEmitterFolder, "scaletexture", TextureWidth, TextureHeight, TextureDataPerEmitter[i].ScaleTextureData);
		TextureLocations.ScaleTextureLocation = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(ScaleAssetPath));
		 
		FString RotationAssetPath = CreateAndSaveTexture(NewEmitterFolder, "rotationtexture", TextureWidth, TextureHeight, TextureDataPerEmitter[i].RotationTextureData);
		TextureLocations.RotationTextureLocation = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(RotationAssetPath));
		
		if (higherOrderHarmonicsExists) {
			int numPixelsHL1 = TextureDataPerEmitter[i].harmonicsL1TextureData.Num();
			float harmonicsL1Width = ceil(sqrt(numPixelsHL1));
			float harmonicsL1Height = ceil(numPixelsHL1 / harmonicsL1Width);
			FString HarmonicsL1AssetPath = CreateAndSaveTexture(NewEmitterFolder, "harmonicsl1texture", harmonicsL1Width, harmonicsL1Height, TextureDataPerEmitter[i].harmonicsL1TextureData);
			TextureLocations.HarmonicsL1TextureLocation = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(HarmonicsL1AssetPath));
		
			int numPixelsHL2 = TextureDataPerEmitter[i].harmonicsL2TextureData.Num();
			float harmonicsL2Width = ceil(sqrt(numPixelsHL2));
			float harmonicsL2Height = ceil(numPixelsHL2 / harmonicsL2Width);
			FString HarmonicsL2AssetPath = CreateAndSaveTexture(NewEmitterFolder, "harmonicsl2texture", harmonicsL2Width, harmonicsL2Height, TextureDataPerEmitter[i].harmonicsL2TextureData);
			TextureLocations.HarmonicsL2TextureLocation = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(HarmonicsL2AssetPath));

			int numPixelsHL31 = TextureDataPerEmitter[i].harmonicsL31TextureData.Num();
			float harmonicsL3Width1 = ceil(sqrt(numPixelsHL31));
			float harmonicsL3Height1 = ceil(numPixelsHL31 / harmonicsL3Width1);
			FString HarmonicsL31AssetPath = CreateAndSaveTexture(NewEmitterFolder, "harmonicsl31texture", harmonicsL3Width1, harmonicsL3Height1, TextureDataPerEmitter[i].harmonicsL31TextureData);
			TextureLocations.HarmonicsL31TextureLocation = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(HarmonicsL31AssetPath));

			int numPixelsHL32 = TextureDataPerEmitter[i].harmonicsL32TextureData.Num();
			float harmonicsL3Width2 = ceil(sqrt(numPixelsHL32));
			float harmonicsL3Height2 = ceil(numPixelsHL32 / harmonicsL3Width2);
			FString HarmonicsL32AssetPath = CreateAndSaveTexture(NewEmitterFolder, "harmonicsl32texture", harmonicsL3Width2, harmonicsL3Height2, TextureDataPerEmitter[i].harmonicsL32TextureData);
			TextureLocations.HarmonicsL32TextureLocation = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(HarmonicsL32AssetPath));
		}

		TexLocations.Add(TextureLocations);
	}

	bOutSuccess = true;
	Output += FString::Printf(TEXT("Successfully parsed PLY File - %s\n\n-- PLY Header --\n\n"), *AbsolutePath);
	Output += HeaderLog;
	Output += "-- End of PLY Header --\n\n";
	Output += "-- PLY Body --\n\n";
	Output += "-- End of PLY Body --\n\n";
	Output += "";
	Output += "---- Finished Parsing PLY File ----";
	OutputString = Output;

	return numVertices;
}

FGaussianSplatData UParser::ParseFilePLY(FString FilePath, bool& bOutSuccess, FString& OutputString) {
	
	// ---- Preparation ----

	FString AbsolutePath = FPaths::ProjectContentDir() + "Models/" + FilePath;
	FString Output = "---- Parsing PLY File ----\n\n";
	TMap<FString, float*> vertexData;
	uint32_t numVertices = 0;
	FGaussianSplatData SplatData;

	// ---- PLY Parsing ----
	
	miniply::PLYReader reader(TCHAR_TO_ANSI(*AbsolutePath));
	
	if (!reader.valid()) {
		bOutSuccess = false;
		OutputString = FString::Printf(TEXT("Parsing PLY failed - Not a valid PLY file - %s"), *AbsolutePath);
		return SplatData;
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
		if (PositionExists) {
			SplatData.Positions.Add(100.0f*FVector(vertexData["x"][i], -vertexData["z"][i], -vertexData["y"][i]));
		}
		if (NormalExists) {
			SplatData.Normals.Add(FVector(vertexData["nx"][i], vertexData["ny"][i], vertexData["nz"][i]));
		}
		if (OrientationExists) {
			FQuat Rot = FQuat(vertexData["rot_1"][i], vertexData["rot_2"][i], vertexData["rot_3"][i], vertexData["rot_0"][i]); // Normalize Quaternion
			Rot.Normalize();
			FQuat Rot2 = FQuat(Rot.X, -Rot.Z, -Rot.Y, Rot.W);
			SplatData.Orientations.Add(Rot2);
		}
		if (ScaleExists) {
			SplatData.Scales.Add(100.0f*FVector(FMath::Exp(vertexData["scale_0"][i]), FMath::Exp(vertexData["scale_2"][i]), FMath::Exp(vertexData["scale_1"][i]))); // Apply Exponential Function
		}
		if (OpacityExists) {
			SplatData.Opacity.Add(FMath::Clamp(1.0f / (1.0f + FMath::Exp(-vertexData["opacity"][i])), 0.0f, 1.0f)); // Apply Sigmoid Function
		}
		if (ZeroOrderHarmonicsExists) {
			SplatData.ZeroOrderHarmonicsCoefficients.Add(FVector(vertexData["f_dc_0"][i], vertexData["f_dc_1"][i], vertexData["f_dc_2"][i]));
		}
		if (higherOrderHarmonicsExists) {
			FHighOrderHarmonicsCoefficientsStruct higherOrderHarmonics;
			for (uint32_t y = 0; y < 45; y += 3) {
				FString index1 = "f_rest_" + FString::FromInt(y);
				FString index2 = "f_rest_" + FString::FromInt(y+1);
				FString index3 = "f_rest_" + FString::FromInt(y+2);

				higherOrderHarmonics.Values.Add(FVector(vertexData[index1][i], vertexData[index2][i], vertexData[index3][i]));
			}
			SplatData.HighOrderHarmonicsCoefficients.Add(higherOrderHarmonics);
		}
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

	return SplatData;
}

TArray<FLinearColor> UParser::SH2RGB(TArray<FVector> ZeroOrderHarmonics, TArray<FHighOrderHarmonicsCoefficientsStruct> HigherOrderHarmonics) {
	TArray<FLinearColor> result;
	for (int i = 0; i < ZeroOrderHarmonics.Num(); i++) {
		FLinearColor col = FLinearColor(0.5 + C0 * ZeroOrderHarmonics[i].X, 0.5 + C0 * ZeroOrderHarmonics[i].Y, 0.5 + C0 * ZeroOrderHarmonics[i].Z);
		result.Add(col);
	}
	return result;
}


