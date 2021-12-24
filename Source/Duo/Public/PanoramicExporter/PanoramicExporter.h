// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "PanoramicExporter.generated.h"

DECLARE_STATS_GROUP(TEXT("Panoramic"), STATGROUP_PANORAMIC, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("ReadRenderTarget"), STAT_ReadRenderTarget, STATGROUP_PANORAMIC);
DECLARE_CYCLE_STAT(TEXT("ExportPanoramicFrame"), STAT_ExportPanoramicFrame, STATGROUP_PANORAMIC);


UENUM()
enum class EPanormaicExportFormat : uint8
{
	Hdr,
	Jpeg,
	Png,
	Exr,
	Bmp
};

inline const FString ToExentsion(EPanormaicExportFormat exportFormat)
{
	switch (exportFormat)
	{
	case EPanormaicExportFormat::Hdr:
		return TEXT("hdr");
	case EPanormaicExportFormat::Jpeg:
		return TEXT("jpg");
	case EPanormaicExportFormat::Png:
		return TEXT("png");
	case EPanormaicExportFormat::Exr:
		return TEXT("exr");
	case EPanormaicExportFormat::Bmp:
		return TEXT("bmp");
	}
	return TEXT("error");
}

class UTextureRenderTargetCube;
class UStaticMeshComponent;
class USceneCaptureComponentCube;

UCLASS()
class DUO_API APanoramicExporter : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APanoramicExporter();

	//APanoramicExporter(FVTableHelper& helper);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type endPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capture|Setting")
	FString m_captureName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capture|Setting")
		EPanormaicExportFormat m_exportFormat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capture|Frame")
		int32 m_framesToCapture;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capture|Frame")
		int32 m_captureHeight;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Captrue|Setting")
		FString m_timeStamp;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient, Category = "Capture|Components")
		UTextureRenderTargetCube* m_renderTargetCube;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		UStaticMeshComponent* m_meshComp;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		USceneCaptureComponentCube* m_cubeCapture;

	UFUNCTION(BlueprintCallable, Category = "PanoramicExporter")
	void Export();

private:
	IImageWrapperModule* m_imageWrapperModule;
};
