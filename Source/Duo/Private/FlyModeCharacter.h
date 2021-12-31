// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FlyModeCharacter.generated.h"


class APanoramicExporter;

UCLASS()
class AFlyModeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFlyModeCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	APlayerController* m_playerController;


	/******************MoveFunctions**********************/
	void MoveForward(float axisValue);
	void MoveRight(float axisValue);
	void MoveUp(float axisValue);
	void MoveRightByMouse(float axisValue);
	void MoveUpByMouse(float axisValue);
	void MoveImp(float axisValue, EAxis::Type axis);

	/******************POVFunctions***********************/
	virtual void AddControllerPitchInput(float axisValue) override;
	virtual void AddControllerYawInput(float axisValue) override;

	/***************** MouseFunctions*********************/
	void OnRightMouseButtonPressed();
	void OnRightMouseButtonReleased();
	void OnLeftMouseButtonPressed();
	void OnLeftMouseButtonReleased();
	void OnLeftMouseButtonDoubleClick();


	void OnMouseWheelUpPressed();
	void OnMouseWheelUpReleased();
	void OnMouseWheelDownPressed();
	void OnMouseWheelDownReleased();

	void OnCaptureButtonPressed();
	void RotateByAngle();

	UFUNCTION(BlueprintCallable, Category = "PanoramicExporter")
	APanoramicExporter* GetPanoramicExporter();

protected:
	UFUNCTION(BlueprintCallable, Category = "PanormaicExporter")
	void SaveScreenShot(FString fileName);

private:
	void SetPOV();
	void RevertPOV();


	// F keyboard.
	void Focus();
	/*************************MouseHitFunctions**********************/
	FHitResult GetHitResult() const;
	/**********************Fly to actor*******************/
	void FlyToActor();
	UFUNCTION()
		void GetLerpValueFromCurveFloat(float outputValue);
	UFUNCTION()
		void HandleTimelineFinishedTrack();

	void HandleMouseWheelStatusChanged();

	// Get mouse wheel input value.
	void GetMouseWheelInput(float axisValue);

	void Zoom(float deltaTime);

	UFUNCTION(Exec)
	void SetSensitivity(float sensitivity);

protected:
	// Camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCameraComponent* m_cameraComp = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> m_anchorPointWidgetClass;
	UUserWidget* m_anchorPointWidget;

	/**********************Interpolation**************************/
	UPROPERTY(EditAnywhere)
	UCurveFloat* m_curveFloatComp;
	UPROPERTY(EditAnywhere)
	class UTimelineComponent* m_timelineComponent;
	
public:
	/************************RotateProperies**********************/
	bool m_bIsRotatingView;
	UPROPERTY(EditAnywhere)
	float m_sensitivityFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageSetting")
	int32 m_width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageSetting")
	int32 m_height;

	// Pan Properties
	bool m_bIsPanning;

	/*********************MouseHitProperties*************/
	FHitResult m_hitResult;
	
	/*****************Debug properties*****************/
	// Move dir
	FVector m_forwardDir;
	FVector m_rightDir;
	FVector m_upDir;

	/*****************Zoom****************************/
	float scrollWheelInputValue;
	UPROPERTY(EditAnywhere)
	float m_zoomSpeed = 10.f;


private:
	APanoramicExporter* m_panoramicExporter;
};
