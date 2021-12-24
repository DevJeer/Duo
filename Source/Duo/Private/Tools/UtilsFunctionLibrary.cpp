// Fill out your copyright notice in the Description page of Project Settings.


#include "Tools/UtilsFunctionLibrary.h"

#if PLATFORM_WINDOWS
#include "HAL/FileManager.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/COMPointer.h"
#include <commdlg.h>
#include <shlobj.h>
#include <Winver.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

bool UUtilsFunctionLibrary::OpenFileDialog(const FString& dialogTitle, const FString& defaultPath, const FString& fileTypes, TArray<FString>& outFilenames, bool bAllowMultiSelect)
{
	bool success = false;
#if PLATFORM_WINDOWS
	TComPtr<IFileDialog> fileDialog;
	if (SUCCEEDED(::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, IID_PPV_ARGS_Helper(&fileDialog))))
	{
		// Set up common settings
		fileDialog->SetTitle(*dialogTitle);
		if (bAllowMultiSelect)
		{
			FILEOPENDIALOGOPTIONS openOptions;
			fileDialog->GetOptions(&openOptions);
			fileDialog->SetOptions(openOptions | FOS_ALLOWMULTISELECT);
		}
		if (!defaultPath.IsEmpty())
		{
			// SHCreateItemFromParsingName requires the given path be absolute and use \ rather than / as our normalized paths do
			FString defaultWindowsPath = FPaths::ConvertRelativePathToFull(defaultPath);
			defaultWindowsPath.ReplaceInline(TEXT("/"), TEXT("\\"), ESearchCase::CaseSensitive);

			TComPtr<IShellItem> defaultPathItem;
			if (SUCCEEDED(::SHCreateItemFromParsingName(*defaultWindowsPath, nullptr, IID_PPV_ARGS(&defaultPathItem))))
			{
				fileDialog->SetFolder(defaultPathItem);
			}
		}

		// Set-up the file type filters
		TArray<FString> unformattedExtensions;
		TArray<COMDLG_FILTERSPEC> fileDialogFilters;
		{
			//const FString DefaultFileTypes = FileTypes;//TEXT("Datasmith Scene (*.udatasmith)|*.udatasmith");
			fileTypes.ParseIntoArray(unformattedExtensions, TEXT("|"), true);

			if (unformattedExtensions.Num() % 2 == 0)
			{
				fileDialogFilters.Reserve(unformattedExtensions.Num() / 2);
				for (int32 ExtensionIndex = 0; ExtensionIndex < unformattedExtensions.Num();)
				{
					COMDLG_FILTERSPEC& NewFilterSpec = fileDialogFilters[fileDialogFilters.AddDefaulted()];
					NewFilterSpec.pszName = *unformattedExtensions[ExtensionIndex++];
					NewFilterSpec.pszSpec = *unformattedExtensions[ExtensionIndex++];
				}
			}
		}
		fileDialog->SetFileTypes(fileDialogFilters.Num(), fileDialogFilters.GetData());

		// Show the picker
		if (SUCCEEDED(fileDialog->Show(NULL)))
		{
			auto AddOutFilename = [&outFilenames](const FString& filename)
			{
				FString& outFilename = outFilenames[outFilenames.Add(filename)];
				outFilename = IFileManager::Get().ConvertToRelativePath(*outFilename);
				FPaths::NormalizeFilename(outFilename);
			};

			{
				IFileOpenDialog* fileOpenDialog = static_cast<IFileOpenDialog*>(fileDialog.Get());

				TComPtr<IShellItemArray> results;
				if (SUCCEEDED(fileOpenDialog->GetResults(&results)))
				{
					DWORD numResults = 0;
					results->GetCount(&numResults);
					for (DWORD resultIndex = 0; resultIndex < numResults; ++resultIndex)
					{
						TComPtr<IShellItem> result;
						if (SUCCEEDED(results->GetItemAt(resultIndex, &result)))
						{
							PWSTR filePath = nullptr;
							if (SUCCEEDED(result->GetDisplayName(SIGDN_FILESYSPATH, &filePath)))
							{
								AddOutFilename(filePath);
								::CoTaskMemFree(filePath);
							}

							result->Release();
						}
					}

					success = true;
				}
			}
		}

		fileDialog->Release();
	}

#else
	UE_LOG(LogTemp, Warning, TEXT("[UEUtilsFunctionLibrary]TODO: OpenFileDialog is not implemented in current OS!"));
#endif
	return success;
}
