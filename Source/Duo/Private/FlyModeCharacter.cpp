// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyModeCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"


// Save last actor location.
static FVector s_lastActorLocation = FVector(.0f, .0f, .0f);

// Save hit result.
static FHitResult s_hitResult;

// Save lerp value.
static float s_lerpValue;
// Save current location of camera and actor.
static FVector s_originalCamRelativeLocation;
static FVector s_targetCamRelativeLocation;
static FVector s_originalActorLocation;
static FVector s_targetActorLocation;

const float DEFAULT_ZOOM_SPEED = 10.0f;
const float MAX_ZOOM_SPEED = DEFAULT_ZOOM_SPEED * 10.0f;
const float MIN_ZOOM_SPEED = DEFAULT_ZOOM_SPEED / 10.0f;
// Control zoom speed.
const float ZOOM_FACTOR = 35.0f;
// Save scroll wheel input.
static float s_scrollWheelInputValue;
static float s_smoothedDolly;
// Save zoom direction.
static FVector s_zoomDir;

// Sets default values
AFlyModeCharacter::AFlyModeCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Get root component.
	UCapsuleComponent* capsuleComp = GetCapsuleComponent();
	// Disable collision.
	capsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Setup camera
	m_cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("FlyModeCameraComponent"));
	check(m_cameraComp);
	m_cameraComp->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(capsuleComp));
	m_cameraComp->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	m_cameraComp->bUsePawnControlRotation = true;

	// Setup character movement component.
	m_characterMovementComp = GetCharacterMovement();
	m_characterMovementComp->GravityScale = 0.0f;
	m_characterMovementComp->MaxAcceleration = 4096.0f;
	m_characterMovementComp->BrakingFrictionFactor = 40.0f;
	m_characterMovementComp->Mass = 0.0f;
	m_characterMovementComp->DefaultLandMovementMode = EMovementMode::MOVE_Flying;
	m_characterMovementComp->MaxFlySpeed = 1200.0f;

	// Setup character properties
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;

	// Init anchorPoint.
	// Auto find WBP_Pov.
	static ConstructorHelpers::FClassFinder<UUserWidget> anchorPointWidgetClass(TEXT("/Game/Duo/UI/POV/WBP_Pov"));
	m_anchorPointWidgetClass = anchorPointWidgetClass.Class;

	// Setup TimeLine
	m_timelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimelineComponent"));
}

// Called when the game starts or when spawned
void AFlyModeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Setup playerController.
	m_playerController = UGameplayStatics::GetPlayerController(this, 0);
	if (m_playerController)
	{
		m_playerController->bShowMouseCursor = 1;
	}

	m_anchorPointWidget = CreateWidget<UUserWidget>(GetWorld(), m_anchorPointWidgetClass);
	m_anchorPointWidget->SetVisibility(ESlateVisibility::Hidden);
	m_anchorPointWidget->AddToViewport();

	// Float curve track.
	FOnTimelineFloat floatCurveTrack;
	FOnTimelineEventStatic timelineFinishedTrack;
	// Bind function with track.
	//s_floatCurveTrack.BindUFunction(this, TEXT("GetLerpValueFromCurveFloat"));
	floatCurveTrack.BindDynamic(this, &AFlyModeCharacter::GetLerpValueFromCurveFloat);
	timelineFinishedTrack.BindUFunction(this, TEXT("HandleTimelineFinishedTrack"));
	// Bind curve with TimeLine
	if (m_curveFloatComp)
	{
		m_timelineComponent->AddInterpFloat(m_curveFloatComp, floatCurveTrack);
		m_timelineComponent->SetTimelineFinishedFunc(timelineFinishedTrack);
	}
}

// Called every frame
void AFlyModeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If left mouse button click, m_timelinecomponent is playing.
	if (m_timelineComponent->IsPlaying())
	{
		FlyToActor();
	}
	// Zoom in/out
	Zoom(DeltaTime);
}

// Called to bind functionality to input
void AFlyModeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFlyModeCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFlyModeCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &AFlyModeCharacter::MoveUp);
	PlayerInputComponent->BindAxis("MoveRightByMouse", this, &AFlyModeCharacter::MoveRightByMouse);
	PlayerInputComponent->BindAxis("MoveUpByMouse", this, &AFlyModeCharacter::MoveUpByMouse);
	PlayerInputComponent->BindAxis("Turn", this, &AFlyModeCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &AFlyModeCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("MouseWheelScroll", this, &AFlyModeCharacter::GetMouseWheelInput);

	PlayerInputComponent->BindAction("RightMouseBtn", IE_Pressed, this, &AFlyModeCharacter::OnRightMouseButtonPressed);
	PlayerInputComponent->BindAction("RightMouseBtn", IE_Released, this, &AFlyModeCharacter::OnRightMouseButtonReleased);
	PlayerInputComponent->BindAction("LeftMouseBtn", IE_Pressed, this, &AFlyModeCharacter::OnLeftMouseButtonPressed);
	PlayerInputComponent->BindAction("LeftMouseBtn", IE_Released, this, &AFlyModeCharacter::OnLeftMouseButtonReleased);
	PlayerInputComponent->BindAction("LeftMouseBtn", IE_DoubleClick, this, &AFlyModeCharacter::OnLeftMouseButtonDoubleClick);
	PlayerInputComponent->BindAction("Focus", IE_Pressed, this, &AFlyModeCharacter::Focus);
	PlayerInputComponent->BindAction("MouseWheelUp", IE_Pressed, this, &AFlyModeCharacter::OnMouseWheelUpPressed);
	PlayerInputComponent->BindAction("MouseWheelUp", IE_Released, this, &AFlyModeCharacter::OnMouseWheelUpReleased);
	PlayerInputComponent->BindAction("MouseWheelDown", IE_Pressed, this, &AFlyModeCharacter::OnMouseWheelDownPressed);
	PlayerInputComponent->BindAction("MouseWheelDown", IE_Released, this, &AFlyModeCharacter::OnMouseWheelDownReleased);
}

void AFlyModeCharacter::MoveForward(float axisValue)
{
	if (Controller)
	{
		MoveImp(axisValue, EAxis::X);
	}
}

void AFlyModeCharacter::MoveRight(float axisValue)
{
	if (Controller)
	{
		MoveImp(axisValue, EAxis::Y);
	}
}

void AFlyModeCharacter::MoveUp(float axisValue)
{
	if (Controller)
	{
		MoveImp(axisValue, EAxis::Z);
	}
}

void AFlyModeCharacter::MoveRightByMouse(float axisValue)
{
	if (Controller && m_bIsPanning)
	{
		MoveImp(axisValue, EAxis::Y);
	}
}

void AFlyModeCharacter::MoveUpByMouse(float axisValue)
{
	if (Controller && m_bIsPanning)
	{
		MoveImp(axisValue, EAxis::Z);
	}
}

void AFlyModeCharacter::MoveImp(float axisValue, EAxis::Type axis)
{
	AddMovementInput(FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(axis), axisValue);
}

void AFlyModeCharacter::AddControllerPitchInput(float axisValue)
{
	if (axisValue != .0f && Controller->IsLocalPlayerController() && m_bIsRotatingView)
	{
		APlayerController* const pc = CastChecked<APlayerController>(Controller);
		pc->AddPitchInput(axisValue * m_lookSpeed);
	}
}

void AFlyModeCharacter::AddControllerYawInput(float axisValue)
{
	if (axisValue != .0f && Controller->IsLocalPlayerController() && m_bIsRotatingView)
	{
		APlayerController* const pc = CastChecked<APlayerController>(Controller);
		pc->AddYawInput(axisValue * m_lookSpeed);
	}
}

// Whether need
void AFlyModeCharacter::GetMouseWheelInput(float axisValue)
{
	// Get MouseWheel Input.
	s_scrollWheelInputValue = axisValue;

	//UE_LOG(LogTemp, Warning, TEXT("Scroll Wheel input value is %f"), axisValue);
}

void AFlyModeCharacter::OnRightMouseButtonPressed()
{
	// Character can rotate.
	m_bIsRotatingView = true;
	// Setup mouse visibility.
	m_playerController->bShowMouseCursor = false;
	// GetHit
	m_hitResult = GetHitResult();
	SetPOV();

	// Save mouse position.
	FVector2D mousePos;
	// Handle anchorPoint widget.
	m_playerController->GetMousePosition(mousePos.X, mousePos.Y);
	m_anchorPointWidget->SetPositionInViewport(mousePos);
	m_anchorPointWidget->SetVisibility(ESlateVisibility::Visible);
}

void AFlyModeCharacter::OnRightMouseButtonReleased()
{
	// Character can't rotate.
	m_bIsRotatingView = false;
	// Setup mouse visibility
	m_playerController->bShowMouseCursor = true;

	m_anchorPointWidget->SetVisibility(ESlateVisibility::Hidden);
	RevertPOV();
}


void AFlyModeCharacter::OnLeftMouseButtonPressed()
{
	// Enable panning.
	m_bIsPanning = true;
	//m_playerController->GetMousePosition(mousePos.X, mousePos.Y);
	s_lastActorLocation = GetActorLocation();
}

void AFlyModeCharacter::OnLeftMouseButtonReleased()
{
	static UPrimitiveComponent* s_lastComp = nullptr;
	m_bIsPanning = false;

	FVector currLoc;
	currLoc = GetActorLocation();
	// 1.f is tolerance.
	if (!currLoc.Equals(s_lastActorLocation, 1.f))
	{
		return;
	}

	// Clear last selection.
	if (s_lastComp)
	{
		s_lastComp->SetRenderCustomDepth(false);
	}

	// Highlight selection.
	s_hitResult = GetHitResult();
	if (s_hitResult.GetComponent())
	{
		// Highlight actor
		s_hitResult.Component->SetRenderCustomDepth(true);
		s_lastComp = s_hitResult.GetComponent();
	}
	else
	{
		s_lastComp = nullptr;
	}
}

void AFlyModeCharacter::OnLeftMouseButtonDoubleClick()
{
	// Detects whether actor have been hit.
	s_hitResult = GetHitResult();
	if (s_hitResult.GetActor())
	{
		// Save a location.
		s_originalCamRelativeLocation = m_cameraComp->GetRelativeLocation();
		s_originalActorLocation = GetActorLocation();

		// Calculate fly to target value.
		FVector hitActorBoxExtent;
		// Get the properties of the actor that was hit.
		s_hitResult.GetActor()->GetActorBounds(false, s_targetActorLocation, hitActorBoxExtent);
		// Calculate actor diameter.
		float hitActorDiameter = FMath::Max3(hitActorBoxExtent.X, hitActorBoxExtent.Y, hitActorBoxExtent.Z);
		// Calculate the relative location of camera. -3.0 is three times the diameter.
		s_targetCamRelativeLocation = FVector(hitActorDiameter * -3.0f, .0f, .0f);

		// Play Timeline.
		m_timelineComponent->PlayFromStart();
	}
}


void AFlyModeCharacter::OnMouseWheelUpPressed()
{
	HandleMouseWheelStatusChanged();
}

void AFlyModeCharacter::OnMouseWheelUpReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("Mouse Wheel Up Released!"));
}

void AFlyModeCharacter::OnMouseWheelDownPressed()
{
	HandleMouseWheelStatusChanged();
}

void AFlyModeCharacter::OnMouseWheelDownReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("Mouse Wheel Down Released!"));
}

void AFlyModeCharacter::SetPOV()
{
	// If hit an actor.
	if (m_hitResult.GetActor())
	{
		FVector hitLocation = m_hitResult.Location;
		// Get world location of cameraComp.
		FVector cameraWorldLocation = m_cameraComp->GetComponentLocation();

		// Set root component location.
		SetActorLocation(hitLocation);
		m_cameraComp->SetWorldLocation(cameraWorldLocation);
	}
}

void AFlyModeCharacter::RevertPOV()
{
	// Get last camera pos.
	FVector cameraPos = m_cameraComp->GetComponentLocation();
	// Set Camera target.
	SetActorLocation(cameraPos);
	// Set current Camera pos.
	m_cameraComp->SetWorldLocation(cameraPos);
}

void AFlyModeCharacter::Focus()
{
	// Detects whether actor have been hit.
	s_hitResult = GetHitResult();
	if (s_hitResult.GetActor())
	{
		// Save a location.
		s_originalCamRelativeLocation = m_cameraComp->GetRelativeLocation();
		s_originalActorLocation = GetActorLocation();
		m_timelineComponent->PlayFromStart();
	}
}


FHitResult AFlyModeCharacter::GetHitResult() const
{
	FHitResult hitResult;
	TArray<TEnumAsByte<EObjectTypeQuery>> arrays;
	arrays.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	arrays.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	m_playerController->GetHitResultUnderCursorForObjects(arrays, true, hitResult);
	return hitResult;
}

void AFlyModeCharacter::FlyToActor()
{

	/******Dont't use interploation.*****/
	//if (hitResult.GetActor())
	//{
	// FVector hitActorCenterPos;
	// FVector hitActorBoxExtent;
	//	m_timelineComponent->PlayFromStart();
	//	// If hit, Fly to target center.
	//	hitResult.GetActor()->GetActorBounds(false, hitActorCenterPos, hitActorBoxExtent);
	//	hitActorDiameter = FMath::Max3(hitActorBoxExtent.X, hitActorBoxExtent.Y, hitActorBoxExtent.Z);
	//	FVector targetCamLocation = FVector(hitActorDiameter * -3.0f, .0f, .0f);
	//	SetActorLocation(hitActorCenterPos);
	//	m_cameraComp->SetRelativeLocation(targetCamLocation);
	//}

	// Set actor location.
	SetActorLocation(FMath::Lerp(s_originalActorLocation, s_targetActorLocation, s_lerpValue));
	// Set relative location of camera.
	m_cameraComp->SetRelativeLocation(FMath::Lerp(s_originalCamRelativeLocation, s_targetCamRelativeLocation, s_lerpValue));
}

void AFlyModeCharacter::GetLerpValueFromCurveFloat(float outputValue)
{
	s_lerpValue = outputValue;
}

void AFlyModeCharacter::HandleTimelineFinishedTrack()
{
	RevertPOV();
	UE_LOG(LogTemp, Warning, TEXT("Timeline is finished!"));
}

void AFlyModeCharacter::HandleMouseWheelStatusChanged()
{
	// Detect Actor.
	s_hitResult = GetHitResult();
	if (s_hitResult.GetActor())
	{
		// Calculate zoom speed.
		//UE_LOG(LogTemp, Warning, TEXT("Distance : %f"), s_hitResult.Distance);
		m_zoomSpeed = s_hitResult.Distance / ZOOM_FACTOR;
		// Limit the maximum and minimum value.
		m_zoomSpeed = FMath::Clamp(m_zoomSpeed, MIN_ZOOM_SPEED, MAX_ZOOM_SPEED);
		//UE_LOG(LogTemp, Warning, TEXT("ZoomSpeed : %f"), m_zoomSpeed);
		s_zoomDir = s_hitResult.Location - this->GetActorLocation();
		// .0001f is the minimum of zoomAnchorDir.
		s_zoomDir.Normalize(.0001f);
	}
	else
	{
		// Reset zoom speed.
		m_zoomSpeed = DEFAULT_ZOOM_SPEED;
	}
}

void AFlyModeCharacter::CalculateZoomSpeed()
{

}

void AFlyModeCharacter::Zoom(float deltaTime)
{
	// Calculate smoothed dolly. 
	// 10.f controls the rate of change of the interpolation.
	s_smoothedDolly = FMath::FInterpTo(s_smoothedDolly, s_scrollWheelInputValue, deltaTime, 10.f);

	// Calculate zoom speed by distance.
	//FVector deltaLocation = m_forwardDir * s_smoothedDolly  * m_zoomSpeed;
	FVector deltaLocation = s_zoomDir * s_smoothedDolly * m_zoomSpeed;
	//if (deltaLocation.X != .0f)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("scrolWheelInputValue %f"), scrollWheelInputValue);
	//	UE_LOG(LogTemp, Warning, TEXT("smoothDolly %f"), s_smoothedDolly);
	//	UE_LOG(LogTemp, Warning, TEXT("--------------------------"));
	//	UE_LOG(LogTemp, Warning, TEXT("forwardDir %f, %f, %f"), m_forwardDir.X, m_forwardDir.Y, m_forwardDir.Z);
	//	UE_LOG(LogTemp, Warning, TEXT("deltaLocation %f, %f, %f"), deltaLocation.X, deltaLocation.Y, deltaLocation.Z);
	//	UE_LOG(LogTemp, Warning, TEXT("-----------------------"));
	//}
	this->AddActorWorldOffset(deltaLocation);
}

