// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealSplat.h"
#include "ToolMenus.h"

IMPLEMENT_MODULE(FUnrealSplatModule, UnrealSplat)

#define LOCTEXT_NAMESPACE "FUnrealSplatModule"
void FUnrealSplatModule::RegisterMenuExtensions()
{
    UE_LOG(LogTemp, Warning, TEXT("My UI Widget should spawn any second"));

    // Use the current object as the owner of the menus
    // This allows us to remove all our custom menus when the
    // module is unloaded (see ShutdownModule above)
    FToolMenuOwnerScoped OwnerScoped(this);
    UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(
        "LevelEditor.LevelEditorToolBar.PlayToolBar");

    // Extend the "File" section of the main toolbar
    if (ToolbarMenu)
    {
        UE_LOG(LogTemp, Warning, TEXT("ToolbarMenu is valid. Proceeding to add section."));

        // Use the exact section name from the ToolMenus.Edit command.
        FToolMenuSection& ToolbarSection = ToolbarMenu->AddSection("MyCustomSection");

        ToolbarSection.AddEntry(FToolMenuEntry::InitWidget(
            TEXT("MyCustomButtonName"),
            SNew(SMyPluginButtonWidget),
            INVTEXT("My custom button")
        ));
    }
    else
    {
        // This log will tell us the menu path is invalid.
        UE_LOG(LogTemp, Error, TEXT("Failed to extend menu! The menu path 'LevelEditor.LevelEditorToolBar.ModesToolBar' may be incorrect or unavailable."));
    }


    UE_LOG(LogTemp, Warning, TEXT("My UI Widget should have spawned now"));

}

void FUnrealSplatModule::StartupModule()
{
    UE_LOG(LogTemp, Warning, TEXT("MyEditorModule has started up!"));
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(
		this, &FUnrealSplatModule::RegisterMenuExtensions));
}

void FUnrealSplatModule::ShutdownModule()
{
	// Unregister the startup function
	UToolMenus::UnRegisterStartupCallback(this);

	// Unregister all our menu extensions
	UToolMenus::UnregisterOwner(this);
}

#undef LOCTEXT_NAMESPACE