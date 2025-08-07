// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Editor.h" // Provides access to the global GEditor object, essential for editor subsystems [6]
#include "EditorAssetLibrary.h" // Declares UEditorAssetLibrary, used for loading assets by path [4, 6]
#include "EditorUtilitySubsystem.h" // Declares UEditorUtilitySubsystem, the primary interface for spawning EUWs [4, 6, 9]
#include "EditorUtilityWidgetBlueprint.h" // Declares UEditorUtilityWidgetBlueprint, the class representing your EUW asset [4, 6]

class SMyPluginButtonWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMyPluginButtonWidget) {}
	SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        ChildSlot
            [
                SNew(SButton) // Wrap your content in a button
                    .OnClicked_Lambda([]() -> FReply // Use a lambda to handle the click
                        {
                            // The log message confirms the button click is working.
                            UE_LOG(LogTemp, Log, TEXT("Button clicked. Attempting to open widget."));

                            // Use StaticLoadObject to reliably load the asset by path.
                            const FString WidgetPath = TEXT("/UnrealSplat/Widgets/UnrealSplat.UnrealSplat");
                            UEditorUtilityWidgetBlueprint* MyWidgetBlueprint = Cast<UEditorUtilityWidgetBlueprint>(
                                StaticLoadObject(UEditorUtilityWidgetBlueprint::StaticClass(), nullptr, *WidgetPath)
                            );

                            if (!MyWidgetBlueprint)
                            {
                                UE_LOG(LogTemp, Error, TEXT("Failed to load widget blueprint from path: %s"), *WidgetPath);
                                return FReply::Handled();
                            }

                            // Get the Editor Utility Subsystem.
                            UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
                            if (!EditorUtilitySubsystem)
                            {
                                UE_LOG(LogTemp, Error, TEXT("Failed to get Editor Utility Subsystem."));
                                return FReply::Handled();
                            }

                            // Spawn the widget.
                            EditorUtilitySubsystem->SpawnAndRegisterTab(MyWidgetBlueprint);
                            UE_LOG(LogTemp, Log, TEXT("Widget spawned successfully!"));

                            return FReply::Handled();
                        })
                    .ContentPadding(FMargin(5.0f)) // Add some padding inside the button
                    [
                        SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .VAlign(VAlign_Center)
                            .Padding(FMargin(0.0f, 0.0f, 5.0f, 0.0f))
                            [
                                SNew(SImage)
                                    .Image(FAppStyle::Get().GetBrush("LevelEditor.MeshPaintMode"))
                            ]
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .VAlign(VAlign_Center)
                            [
                                SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("UnrealSplat")))
                                    .Font(FCoreStyle::Get().GetFontStyle("SmallText"))
                            ]
                    ]
            ];
    }
};

class FUnrealSplatModule : public IModuleInterface
{
private:
	
	void RegisterMenuExtensions();

public:

	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;


};
