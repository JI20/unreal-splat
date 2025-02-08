// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Parser.generated.h"

USTRUCT(BlueprintType)
struct FHighOrderHarmonicsCoefficientsStruct {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> Values;

	bool operator==(const FHighOrderHarmonicsCoefficientsStruct& Other) const
	{
		return Values == Other.Values;
	}

	FHighOrderHarmonicsCoefficientsStruct()
		: Values()
	{
	}
};

/**
 * Represents parsed data for a single splat, loaded from a regular PLY file.
 */
USTRUCT(BlueprintType)
struct FGaussianSplatData
{

	GENERATED_BODY()

	// Splat position (x, y, z)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> Positions;

	// Normal vectors [optional] (nx, ny, nz)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> Normals;

	// Splat orientation coming as wxyz from PLY (rot_0, rot_1, rot_2, rot_3)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FQuat> Orientations;

	// Splat scale (scale_0, scale_1, scale_2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> Scales;

	// Splat opacity (opacity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> Opacity;

	// Spherical Harmonics coefficients - Zero order (f_dc_0, f_dc_1, f_dc_2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> ZeroOrderHarmonicsCoefficients;

	// Spherical Harmonics coefficients - High order (f_rest_0, ..., f_rest_44)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FHighOrderHarmonicsCoefficientsStruct> HighOrderHarmonicsCoefficients;

	bool operator==(const FGaussianSplatData& Other) const
	{
		return Positions == Other.Positions
			&& Normals == Other.Normals
			&& Orientations == Other.Orientations
			&& Scales == Other.Scales
			&& Opacity == Other.Opacity
			&& ZeroOrderHarmonicsCoefficients == Other.ZeroOrderHarmonicsCoefficients
			&& HighOrderHarmonicsCoefficients == Other.HighOrderHarmonicsCoefficients;
	}

	FGaussianSplatData()
		: Positions()
		, Normals()
		, Orientations()
		, Scales()
		, Opacity()
	{
	}
};

/**
 * Class for reading and parsing 3D Splat models into Unreal
 */
UCLASS()
class UNREALSPLAT_API UParser : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "JI20/Parser")
	static FGaussianSplatData ParseFilePLY(FString FilePath, bool& bOutSuccess, FString& OutputString);

};
