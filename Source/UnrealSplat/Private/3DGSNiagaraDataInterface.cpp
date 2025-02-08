#include "3DGSNiagaraDataInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "VectorVM.h"
using namespace NGaussianSplatBufferNames;

U3DGSNiagaraDataInterface::U3DGSNiagaraDataInterface()
{
    // Initialize with some default Gaussian splats
    FGaussianSplatData splat1;

    Splats.Add(splat1);
}

void U3DGSNiagaraDataInterface::GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)
{
    FNiagaraFunctionSignature Sig;
    Sig.Name = TEXT("GetSplatPosition");
    Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("3DGSNiagaraDataInterface")));
    Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
    Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Position")));
    Sig.bMemberFunction = true;
    Sig.bRequiresContext = false;

    OutFunctions.Add(Sig);
}

DEFINE_NDI_DIRECT_FUNC_BINDER(U3DGSNiagaraDataInterface, GetSplatPosition);

void U3DGSNiagaraDataInterface::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
    if (BindingInfo.Name == TEXT("GetSplatPosition"))
    {
        NDI_FUNC_BINDER(U3DGSNiagaraDataInterface, GetSplatPosition)::Bind(this, OutFunc);
    }
}

void U3DGSNiagaraDataInterface::GetSplatPosition(FVectorVMExternalFunctionContext& Context) const
{
    VectorVM::FUserPtrHandler<U3DGSNiagaraDataInterface> InstData(Context);
    FNDIInputParam<int32> IndexParam(Context);
    FNDIOutputParam<float> OutPosX(Context);
    FNDIOutputParam<float> OutPosY(Context);
    FNDIOutputParam<float> OutPosZ(Context);

    const int32 NumInstances = Context.GetNumInstances();

    for (int32 i = 0; i < NumInstances; ++i)
    {
        const int32 Index = IndexParam.GetAndAdvance();

        if (Splats.IsValidIndex(Index))
        {
            const FGaussianSplatData& Splat = Splats[Index];

        }
        else
        {
            OutPosX.SetAndAdvance(0.0f);
            OutPosY.SetAndAdvance(0.0f);
            OutPosZ.SetAndAdvance(0.0f);
        }
    }
}

void U3DGSNiagaraDataInterface::UpdateSplats(const TArray<FGaussianSplatData>& NewSplats)
{
    Splats = NewSplats;
}

bool U3DGSNiagaraDataInterface::CopyToInternal(UNiagaraDataInterface* Destination) const
{
    // Ensure the destination is of the correct type
    if (U3DGSNiagaraDataInterface* DestinationNDI = Cast<U3DGSNiagaraDataInterface>(Destination))
    {
        // Copy the splats array to the destination
        DestinationNDI->Splats = Splats;
        return true;
    }
    return false;
}

bool U3DGSNiagaraDataInterface::Equals(const UNiagaraDataInterface* Other) const
{
    // Ensure the other NDI is of the correct type
    if (const U3DGSNiagaraDataInterface* OtherNDI = Cast<U3DGSNiagaraDataInterface>(Other))
    {
        // Compare the splats arrays for equality
        return Splats == OtherNDI->Splats;
    }
    return false;
}

void U3DGSNiagaraDataInterface::GetParameterDefinitionHLSL(
    const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
    Super::GetParameterDefinitionHLSL(ParamInfo, OutHLSL);

    OutHLSL.Appendf(TEXT("int %s%s;\n"),
        *ParamInfo.DataInterfaceHLSLSymbol, *SplatsCountParamName);
    OutHLSL.Appendf(TEXT("float3 %s%s;\n"),
        *ParamInfo.DataInterfaceHLSLSymbol, *GlobalTintParamName);
    OutHLSL.Appendf(TEXT("Buffer<float4> %s%s;\n"),
        *ParamInfo.DataInterfaceHLSLSymbol, *PositionsBufferName);
    OutHLSL.Appendf(TEXT("Buffer<float4> %s%s;\n"),
        *ParamInfo.DataInterfaceHLSLSymbol, *ScalesBufferName);
    OutHLSL.Appendf(TEXT("Buffer<float4> %s%s;\n"),
        *ParamInfo.DataInterfaceHLSLSymbol, *OrientationsBufferName);
    OutHLSL.Appendf(TEXT("Buffer<float4> %s%s;\n"),
        *ParamInfo.DataInterfaceHLSLSymbol, *SHZeroCoeffsBufferName);
}

bool U3DGSNiagaraDataInterface::GetFunctionHLSL(
    const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const
    FNiagaraDataInterfaceGeneratedFunction& FunctionInfo,
    int FunctionInstanceIndex, FString& OutHLSL)
{
    if (Super::GetFunctionHLSL(ParamInfo, FunctionInfo,
        FunctionInstanceIndex, OutHLSL))
    {
        // If the function is already defined on the Super class, do not
        // duplicate its definition.
        return true;
    }

    if (FunctionInfo.DefinitionName == *GetPositionFunctionName)
    {
        static const TCHAR* FormatBounds = TEXT(R"(
      void {FunctionName}(int Index, out float3 OutPosition)
      {
        OutPosition = {PositionsBuffer}[Index].xyz;
      }
    )");
        const TMap<FString, FStringFormatArg> ArgsBounds =
        {
         {TEXT("FunctionName"), FStringFormatArg(FunctionInfo.InstanceName)},
         {TEXT("PositionsBuffer"),
           FStringFormatArg(ParamInfo.DataInterfaceHLSLSymbol +
             PositionsBufferName)},
        };
        OutHLSL += FString::Format(FormatBounds, ArgsBounds);
    }
    // TODO: Add the other functions for Orientation Scale etc.
    else
    {
        // Return false if the function name does not match any expected.
        return false;
    }
    return true;
}