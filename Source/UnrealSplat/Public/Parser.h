// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Parser.generated.h"

/**
 * Represents parsed data for a single splat, loaded from a regular PLY file.
 */
USTRUCT(BlueprintType)
struct FGaussianSplatData
{

	GENERATED_BODY()

	// Splat position (x, y, z)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position;

	// Normal vectors [optional] (nx, ny, nz)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Normal;

	// Splat orientation coming as wxyz from PLY (rot_0, rot_1, rot_2, rot_3)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FQuat Orientation;

	// Splat scale (scale_0, scale_1, scale_2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale;

	// Splat opacity (opacity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Opacity;

	// Spherical Harmonics coefficients - Zero order (f_dc_0, f_dc_1, f_dc_2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ZeroOrderHarmonicsCoefficients;

	// Spherical Harmonics coefficients - High order (f_rest_0, ..., f_rest_44)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> HighOrderHarmonicsCoefficients;

	FGaussianSplatData()
		: Position(FVector::ZeroVector)
		, Normal(FVector::ZeroVector)
		, Orientation(FQuat::Identity)
		, Scale(FVector::OneVector)
		, Opacity(0)
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
	static FString ParseFilePLY(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage);

};
