#pragma once

#include "NiagaraDataInterface.h"
#include "NiagaraTypes.h"
#include "Parser.h"
#include "3DGSNiagaraDataInterface.generated.h"


BEGIN_SHADER_PARAMETER_STRUCT(FGaussianSplatShaderParameters, )
    SHADER_PARAMETER(int, SplatsCount)
    SHADER_PARAMETER(FVector3f, GlobalTint)
    SHADER_PARAMETER_SRV(Buffer<float4>, Positions)
    SHADER_PARAMETER_SRV(Buffer<float4>, Scales)
    SHADER_PARAMETER_SRV(Buffer<float4>, Orientations)
    SHADER_PARAMETER_SRV(Buffer<float4>, SHZeroCoeffsAndOpacity)
END_SHADER_PARAMETER_STRUCT()

namespace NGaussianSplatBufferNames
{
    static const FString SplatsCountParamName = TEXT("SplatsCountParam");
    static const FString GlobalTintParamName = TEXT("GlobalTintParam");
    static const FString PositionsBufferName = TEXT("PositionsBuffer");
    static const FString ScalesBufferName = TEXT("ScalesBuffer");
    static const FString OrientationsBufferName = TEXT("OrientationsBuffer");
    static const FString SHZeroCoeffsBufferName = TEXT("SHZeroCoeffsBuffer");
    static const FString GetPositionFunctionName = TEXT("GetSplatPosition");
}


UCLASS(EditInlineNew, Category = "Custom", meta = (DisplayName = "3D Gaussian Splat NDI"))
class UNREALSPLAT_API U3DGSNiagaraDataInterface : public UNiagaraDataInterface
{
    GENERATED_BODY()

public:
    U3DGSNiagaraDataInterface();

    // Override the base class functions
    virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions) override;
    virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;
    virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;
    virtual bool Equals(const UNiagaraDataInterface* Other) const override;
    virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override { return Target == ENiagaraSimTarget::GPUComputeSim; }

    // Overrides for GPU compatibility
    virtual void GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
    virtual bool GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL) override;
    
    //TODO: virtual void BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
    //TODO: virtual void SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;
    //Outdated in UE5.5: virtual bool UseLegacyShaderBindings() const override { return false; }

    // Custom function to retrieve splat position
    void GetSplatPosition(FVectorVMExternalFunctionContext& Context) const;

    // Function to update the splats array from Blueprints
    UFUNCTION(BlueprintCallable, Category = "3D Gaussian Splat")
    void UpdateSplats(const TArray<FGaussianSplatData>& NewSplats);

    // Data storage for Gaussian Splats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Gaussian Splats")
    TArray<FGaussianSplatData> Splats;
};
