// Fill out your copyright notice in the Description page of Project Settings.


#include "PanoramicExporter/PanoramicExporter.h"
#include "Components/SceneCaptureComponentCube.h"
#include "Components/StaticMeshComponent.h"
#include "CubemapUnwrapUtils.h"
#include "Engine/TextureRenderTargetCube.h"
#include "RenderingThread.h"
#include "ImageUtils.h"
#include "Tools/UtilsFunctionLibrary.h"

DECLARE_LOG_CATEGORY_CLASS(PanormicExporter, Warning, All);

FString APanoramicExporter::s_jpgFileName = "";
FDelegateHandle APanoramicExporter::s_delegateHandle;

// Sets default values
APanoramicExporter::APanoramicExporter()
	: m_exportFormat(EPanormaicExportFormat::Jpeg)
	, m_framesToCapture(0)
	, m_captureHeight(4096)
	, m_renderTargetCube(nullptr)
	, m_imageWrapperModule(&FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper")))
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CameraStaticMeshComponent"));
	m_meshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	m_meshComp->bHiddenInGame = true;
	m_meshComp->CastShadow = false;
	m_meshComp->SetComponentTickEnabled(false);

	RootComponent = m_meshComp;

	m_cubeCapture = CreateDefaultSubobject<USceneCaptureComponentCube>(TEXT("CubeCapture"));
	m_cubeCapture->bCaptureEveryFrame = false;
	m_cubeCapture->bAlwaysPersistRenderingState = true;
	m_cubeCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	m_cubeCapture->SetupAttachment(RootComponent);
	m_cubeCapture->TextureTarget = m_renderTargetCube;
}

// Called when the game starts or when spawned
void APanoramicExporter::BeginPlay()
{
	Super::BeginPlay();

	m_timeStamp = FDateTime::Now().ToString();
	if (m_renderTargetCube == nullptr)
	{
		m_renderTargetCube = NewObject<UTextureRenderTargetCube>(this, TEXT("PanoramicRenderTargetCube"), RF_Transient);
		m_renderTargetCube->SizeX = m_captureHeight;
		m_renderTargetCube->bHDR = (m_exportFormat == EPanormaicExportFormat::Hdr);
		m_renderTargetCube->bNeedsTwoCopies = true;
		//m_renderTargetCube->InitAutoFormat(m_captureHeight);
		m_renderTargetCube->Init(m_captureHeight, PF_B8G8R8A8);
		m_renderTargetCube->UpdateResource();
		m_cubeCapture->TextureTarget = m_renderTargetCube;
	}

	m_cubeCapture->CaptureScene();
	FlushRenderingCommands();
}

void APanoramicExporter::EndPlay(EEndPlayReason::Type endPlayReason)
{
	// Release
	m_renderTargetCube = nullptr;
	m_cubeCapture->TextureTarget = nullptr;

	Super::EndPlay(endPlayReason);
}

// Called every frame
void APanoramicExporter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APanoramicExporter::Export()
{
	TArray<FString> outFileNames;

	static int32 frameNumber = -1;
	frameNumber++;
	FString projDir = FPaths::ProjectSavedDir() + TEXT("Panoramas");
	FString folderName = m_captureName + (m_captureName.IsEmpty() ? TEXT("") : TEXT("_")) + m_timeStamp;
	FString fileName = FString::Printf(TEXT("Frame_%d.%s"), frameNumber, *ToExentsion(m_exportFormat));
	const FString framePath = FPaths::Combine(projDir, folderName, fileName);
	UE_LOG(PanormicExporter, Warning, TEXT("%s"), *projDir);
	UE_LOG(PanormicExporter, Warning, TEXT("%s"), *folderName);
	UE_LOG(PanormicExporter, Warning, TEXT("%s"), *fileName);
	UE_LOG(PanormicExporter, Warning, TEXT("%s"), *framePath);

	m_cubeCapture->CaptureScene();
	FlushRenderingCommands();

	// Export hdr.
	if (m_exportFormat == EPanormaicExportFormat::Hdr)
	{
		TUniquePtr<FArchive> fileWriter(IFileManager::Get().CreateFileWriter(*framePath));
		if (fileWriter)
		{
			FImageUtils::ExportRenderTargetCubeAsHDR(m_renderTargetCube, *fileWriter);
		}
		return;
	}
	if (m_cubeCapture->TextureTarget == nullptr)
		return;

	// Others.
	const int32 width = m_cubeCapture->TextureTarget->SizeX * 2;
	const int32 height = m_cubeCapture->TextureTarget->SizeX;

	TArray64<uint8> rawData;

	{
		SCOPE_CYCLE_COUNTER(STAT_ReadRenderTarget);
		FTextureRenderTargetResource* renderTarget = m_cubeCapture->TextureTarget->GameThread_GetRenderTargetResource();
		rawData.AddUninitialized(width * height);

		FIntPoint size;
		EPixelFormat pixelFormat;
		if (!CubemapHelpers::GenerateLongLatUnwrap(m_renderTargetCube, rawData, size, pixelFormat))
		{
			return;
		}

		for (int32 i = 3; i < rawData.Num(); i += 4)
		{
			rawData[i] = 255;
		}
	}

	/*FTextureRenderTargetResource* rtResource = m_cubeCapture->TextureTarget->GameThread_GetRenderTargetResource();
	FReadSurfaceDataFlags readPixelFlags(RCM_UNorm);
	TArray<FColor> outBMP;
	for (FColor& color : outBMP)
	{
		color.A = 255;
	}
	outBMP.AddUninitialized(width * height);
	rtResource->ReadPixels(outBMP, readPixelFlags);
	FIntPoint destSize(width, height);*/


	{
		SCOPE_CYCLE_COUNTER(STAT_ExportPanoramicFrame);
		TSharedPtr<IImageWrapper> imageWrapper;
		switch (m_exportFormat)
		{
		case EPanormaicExportFormat::Bmp:
			FFileHelper::CreateBitmap(*framePath, width, height, (FColor*)rawData.GetData());
			break;
		case EPanormaicExportFormat::Jpeg:
			imageWrapper = m_imageWrapperModule->CreateImageWrapper(EImageFormat::JPEG);
			if (imageWrapper->SetRaw(rawData.GetData(), rawData.GetAllocatedSize(), width, height, ERGBFormat::BGRA, 8))
			{
				FFileHelper::SaveArrayToFile(imageWrapper->GetCompressed(100), *framePath);
			}
			break;
		case EPanormaicExportFormat::Png:
			imageWrapper = m_imageWrapperModule->CreateImageWrapper(EImageFormat::PNG);
			if (imageWrapper->SetRaw(rawData.GetData(), rawData.GetAllocatedSize(), width, height, ERGBFormat::BGRA, 8))
			{
				FFileHelper::SaveArrayToFile(imageWrapper->GetCompressed(100), *framePath);
			}
			break;
		case EPanormaicExportFormat::Exr:
			imageWrapper = m_imageWrapperModule->CreateImageWrapper(EImageFormat::EXR);
			if (imageWrapper->SetRaw(rawData.GetData(), rawData.GetAllocatedSize(), width, height, ERGBFormat::BGRA, 8))
			{
				FFileHelper::SaveArrayToFile(imageWrapper->GetCompressed((int32)EImageCompressionQuality::Uncompressed), *framePath);
			}
		default:
			break;
		}
	}
}

void APanoramicExporter::SaveScreenShot(const FString& imagePath, int32 width, int32 height)
{
	if (!UGameViewportClient::OnScreenshotCaptured().IsBound())
	{
		s_jpgFileName = imagePath;
		GetHighResScreenshotConfig().SetResolution(width, height);
		s_delegateHandle = UGameViewportClient::OnScreenshotCaptured().AddStatic(&OnScreenshotCaptureInternal);
		FScreenshotRequest::RequestScreenshot(imagePath, false, false);
	}
}

void APanoramicExporter::OnScreenshotCaptureInternal(int32 width, int32 height, const TArray<FColor>& bitmap)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper;
	ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	ImageWrapper->SetRaw(bitmap.GetData(), bitmap.GetAllocatedSize(), width, height, ERGBFormat::BGRA, 8);
	const TArray64<uint8>& jpegData = ImageWrapper->GetCompressed();
	FFileHelper::SaveArrayToFile(jpegData, *s_jpgFileName);
	UGameViewportClient::OnScreenshotCaptured().Remove(s_delegateHandle);
	ImageWrapper.Reset();
}

