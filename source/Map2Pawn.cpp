// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map2Pawn.h"
#include "Map2WheelFront.h"
#include "Map2WheelRear.h"
#include "Map2Hud.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include <iostream>
#include <windows.h>
#include "USerial.h"
#include "MyTriggerBox.h"
#include <stdio.h>
#include <string.h>
#include <cstdio>
#include <stdlib.h>
using namespace std;

char portName[30] = "\\\\.\\COM";
#define MAX_DATA_LENGTH 255
const unsigned int BLINKING_DELAY = 1000;
char incomingData[MAX_DATA_LENGTH];
#define MAX_DIGITS 10
SerialPort* stm32;
char ledON1[] = "Yokus\n";
char ledOFF1[] = "OFF\n";
int isStart = 0;
int sa = 0;
int so = 0;
int32 nabiz_int = 0;
int port = 0;
#ifndef HMD_MODULE_INCLUDED
#define HMD_MODULE_INCLUDED 0
#endif

// Needed for VR Headset
#if HMD_MODULE_INCLUDED
#include "IXRTrackingSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#endif // HMD_MODULE_INCLUDED

const FName AMap2Pawn::LookUpBinding("LookUp");
const FName AMap2Pawn::LookRightBinding("LookRight");
const FName AMap2Pawn::EngineAudioRPM("RPM");

#define LOCTEXT_NAMESPACE "VehiclePawn"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

AMap2Pawn::AMap2Pawn()
{
	
	// Car mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/VehicleAdv/Vehicle/Vehicle_SkelMesh.Vehicle_SkelMesh"));
	GetMesh()->SetSkeletalMesh(CarMesh.Object);
	
	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/VehicleAdv/Vehicle/VehicleAnimationBlueprint"));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);

	// Setup friction materials
	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> SlipperyMat(TEXT("/Game/VehicleAdv/PhysicsMaterials/Slippery.Slippery"));
	SlipperyMaterial = SlipperyMat.Object;
		
	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> NonSlipperyMat(TEXT("/Game/VehicleAdv/PhysicsMaterials/NonSlippery.NonSlippery"));
	NonSlipperyMaterial = NonSlipperyMat.Object;

	UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

	check(Vehicle4W->WheelSetups.Num() == 4);

	// Wheels/Tyres
	// Setup the wheels
	Vehicle4W->WheelSetups[0].WheelClass = UMap2WheelFront::StaticClass();
	Vehicle4W->WheelSetups[0].BoneName = FName("PhysWheel_FL");
	Vehicle4W->WheelSetups[0].AdditionalOffset = FVector(0.f, -8.f, 0.f);

	Vehicle4W->WheelSetups[1].WheelClass = UMap2WheelFront::StaticClass();
	Vehicle4W->WheelSetups[1].BoneName = FName("PhysWheel_FR");
	Vehicle4W->WheelSetups[1].AdditionalOffset = FVector(0.f, 8.f, 0.f);

	Vehicle4W->WheelSetups[2].WheelClass = UMap2WheelRear::StaticClass();
	Vehicle4W->WheelSetups[2].BoneName = FName("PhysWheel_BL");
	Vehicle4W->WheelSetups[2].AdditionalOffset = FVector(0.f, -8.f, 0.f);

	Vehicle4W->WheelSetups[3].WheelClass = UMap2WheelRear::StaticClass();
	Vehicle4W->WheelSetups[3].BoneName = FName("PhysWheel_BR");
	Vehicle4W->WheelSetups[3].AdditionalOffset = FVector(0.f, 8.f, 0.f);

	// Adjust the tire loading
	Vehicle4W->MinNormalizedTireLoad = 0.0f;
	Vehicle4W->MinNormalizedTireLoadFiltered = 0.2f;
	Vehicle4W->MaxNormalizedTireLoad = 2.0f;
	Vehicle4W->MaxNormalizedTireLoadFiltered = 2.0f;

	// Engine 
	// Torque setup
	//Vehicle4W->MaxEngineRPM = 5700.0f;
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5730.0f, 400.0f);
 
	// Adjust the steering 
	Vehicle4W->SteeringCurve.GetRichCurve()->Reset();
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);
			
 	// Transmission	
	// We want 4wd
	Vehicle4W->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_4W;
	
	// Drive the front wheels a little more than the rear
	Vehicle4W->DifferentialSetup.FrontRearSplit = 0.65;

	//Vehicle4W->MaxEngineRPM = 17029.577637f;
	Vehicle4W->EngineSetup.MaxRPM = 1029.577;

	// Automatic gearbox
	Vehicle4W->TransmissionSetup.bUseGearAutoBox = true;
	Vehicle4W->TransmissionSetup.GearSwitchTime = 0.15f;
	Vehicle4W->TransmissionSetup.GearAutoBoxLatency = 1.0f;
	

	// Physics settings
	// Adjust the center of mass - the buggy is quite low
	UPrimitiveComponent* UpdatedPrimitive = Cast<UPrimitiveComponent>(Vehicle4W->UpdatedComponent);
	if (UpdatedPrimitive)
	{
		UpdatedPrimitive->BodyInstance.COMNudge = FVector(8.0f, 0.0f, 0.0f);
	}

	// Set the inertia scale. This controls how the mass of the vehicle is distributed.
	Vehicle4W->InertiaTensorScale = FVector(1.0f, 1.333f, 1.2f);

	// Create a spring arm component for our chase camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 34.0f));
	SpringArm->SetWorldRotation(FRotator(-20.0f, 0.0f, 0.0f));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 125.0f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;

	// Create the chase camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->SetRelativeLocation(FVector(-125.0, 0.0f, 0.0f));
	Camera->SetRelativeRotation(FRotator(10.0f, 0.0f, 0.0f));
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	// Create In-Car camera component 
	InternalCameraOrigin = FVector(-34.0f, -10.0f, 50.0f);
	InternalCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("InternalCameraBase"));
	InternalCameraBase->SetRelativeLocation(InternalCameraOrigin);
	InternalCameraBase->SetupAttachment(GetMesh());

	InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
	InternalCamera->bUsePawnControlRotation = false;
	InternalCamera->FieldOfView = 90.f;
	InternalCamera->SetupAttachment(InternalCameraBase);

	// In car HUD
	// Create text render component for in car speed display
	InCarSpeed = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarSpeed"));
	InCarSpeed->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	InCarSpeed->SetRelativeLocation(FVector(35.0f, -6.0f, 20.0f));
	InCarSpeed->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	InCarSpeed->SetupAttachment(GetMesh());

	// Create text render component for in car gear display
	InCarGear = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
	InCarGear->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	InCarGear->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
	InCarGear->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	InCarGear->SetupAttachment(GetMesh());
	
	// Setup the audio component and allocate it a sound cue
	//static ConstructorHelpers::FObjectFinder<USoundCue> SoundCue(TEXT("/Game/VehicleAdv/Sound/Engine_Loop_Cue.Engine_Loop_Cue"));
	//EngineSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
	//EngineSoundComponent->SetSound(SoundCue.Object);
	//EngineSoundComponent->SetupAttachment(GetMesh());

	// Colors for the in-car gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	bIsLowFriction = false;
	bInReverseGear = false;
}

void AMap2Pawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	//PlayerInputComponent->BindAxis("MoveForward", this, &AMap2Pawn::MoveForward);
	//PlayerInputComponent->BindAxis("MoveRight", this, &AMap2Pawn::MoveRight);
	PlayerInputComponent->BindAxis(LookUpBinding);
	PlayerInputComponent->BindAxis(LookRightBinding);

	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &AMap2Pawn::OnHandbrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &AMap2Pawn::OnHandbrakeReleased);
	PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &AMap2Pawn::OnToggleCamera);
	PlayerInputComponent->BindAction("open", IE_Pressed, this, &AMap2Pawn::openCom);
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMap2Pawn::OnResetVR); 
}

void AMap2Pawn::MoveForward(float Val)
{
	/*float KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
	int32 KPH_int = FMath::FloorToInt(KPH);
	if (KPH_int > 25)
	{
		GetVehicleMovementComponent()->SetThrottleInput(25 * 0.036f);
		
	}
	else {
		GetVehicleMovementComponent()->SetThrottleInput(Val);
	}*/
	//GetVehicleMovementComponent()->SetGearUp(true);
	FString Fs = FString::SanitizeFloat(Val);
	UE_LOG(LogTemp, Log, TEXT("%s"), *Fs);
	GetVehicleMovementComponent()->SetThrottleInput(Val);

}

void AMap2Pawn::MoveRight(float Val)
{
	GetVehicleMovementComponent()->SetSteeringInput(Val);
}

void AMap2Pawn::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void AMap2Pawn::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void AMap2Pawn::OnToggleCamera()
{
	EnableIncarView(!bInCarCameraActive);
}

void AMap2Pawn::EnableIncarView(const bool bState)
{
	if (bState != bInCarCameraActive)
	{
		bInCarCameraActive = bState;
		
		if (bState == true)
		{
			OnResetVR();
			Camera->Deactivate();
			InternalCamera->Activate();
		}
		else
		{
			InternalCamera->Deactivate();
			Camera->Activate();
		}
		
		InCarSpeed->SetVisibility(bInCarCameraActive);
		InCarGear->SetVisibility(bInCarCameraActive);
	}
}

void AMap2Pawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// Setup the flag to say we are in reverse gear
	bInReverseGear = GetVehicleMovement()->GetCurrentGear() < 0;
	//FString Fs5 = FString::SanitizeFloat((float)comPort);
	//UE_LOG(LogTemp, Log, TEXT("gear: %s"), *Fs5);
	// Update phsyics material
	UpdatePhysicsMaterial();

	// Update the strings used in the hud (incar and onscreen)
	UpdateHUDStrings();

	// Set the string in the incar hud
	SetupInCarHUD();

	//MoveForward(25.0);

	bool bHMDActive = false;
#if HMD_MODULE_INCLUDED
	if ((GEngine->XRSystem.IsValid() == true ) && ( (GEngine->XRSystem->IsHeadTrackingAllowed() == true) || (GEngine->IsStereoscopic3D() == true)))
	{
		bHMDActive = true;
	}
#endif // HMD_MODULE_INCLUDED
	if( bHMDActive == false )
	{
		if ( (InputComponent) && (bInCarCameraActive == true ))
		{
			FRotator HeadRotation = InternalCamera->GetRelativeRotation();
			HeadRotation.Pitch += InputComponent->GetAxisValue(LookUpBinding);
			HeadRotation.Yaw += InputComponent->GetAxisValue(LookRightBinding);
			InternalCamera->SetRelativeRotation(HeadRotation);
		}
	}	

	// Pass the engine RPM to the sound component
	//float RPMToAudioScale = 2500.0f / GetVehicleMovement()->GetEngineMaxRotationSpeed();
	//EngineSoundComponent->SetFloatParameter(EngineAudioRPM, GetVehicleMovement()->GetEngineRotationSpeed()*RPMToAudioScale);

	if (comPort != 0) {
		/*char com[50];
		sprintf(com, "\\\\.\\COM%d", comPort);
		//strcat(portName, com);
		FString Fs = FString(ANSI_TO_TCHAR(com));
		UE_LOG(LogTemp, Log, TEXT("port:%s"),*Fs);
		stm32 = new SerialPort(com);

		if (stm32->isConnected()) {
			UE_LOG(LogTemp, Log, TEXT("Connected!!"));
			isStart = 1;
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("NOT Connected!!"));
		}

		if (isStart)
		{
			for (TObjectIterator<AMyTriggerBox> It; It; ++It)
			{
				It->card = stm32;
			}
		}*/
		port = comPort;
		comPort = 0;

	}
	
	if (isStart){
		exampleReceiveData(Delta);
	}
	
}

void AMap2Pawn::BeginPlay()
{
	FString Fs5 = FString::SanitizeFloat((float)port);
	UE_LOG(LogTemp, Log, TEXT("porrt: %s"), *Fs5);
	Super::BeginPlay();
	FString Fs6 = FString::SanitizeFloat((float)port);
	UE_LOG(LogTemp, Log, TEXT("porrt: %s"), *Fs6);
	bool bWantInCar = false;
	steerString = 0.0;
	// First disable both speed/gear displays 
	bInCarCameraActive = false;
	InCarSpeed->SetVisibility(bInCarCameraActive);
	InCarGear->SetVisibility(bInCarCameraActive);
	if (port != 0) {
		char com[50];
		sprintf(com, "\\\\.\\COM%d", port);
		//strcat(portName, com);
		FString Fs = FString(ANSI_TO_TCHAR(com));
		UE_LOG(LogTemp, Log, TEXT("port:%s"), *Fs);
		stm32 = new SerialPort(com);

		if (stm32->isConnected()) {
			UE_LOG(LogTemp, Log, TEXT("Connected!!"));
			isStart = 1;
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("NOT Connected!!"));
		}

		if (isStart)
		{
			for (TObjectIterator<AMyTriggerBox> It; It; ++It)
			{
				It->card = stm32;
			}
		}
	}

	
	// Enable in car view if HMD is attached
#if HMD_MODULE_INCLUDED
	bWantInCar = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
#endif // HMD_MODULE_INCLUDED

	EnableIncarView(bWantInCar);
	// Start an engine sound playing
	//EngineSoundComponent->Play();
	
	//GetOwner()->GetTimerManager().SetTimer(MemberTimer, this, &AMap2Pawn::MoveForward, 1.0f, true);
	//GetWorld()->GetTimerManager().SetTimer(MemberTimer, this, &AMap2Pawn::MoveForward, 3.0f, true);
}

void AMap2Pawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (isStart == 1) 
		stm32->closeSerial();
}

void AMap2Pawn::OnResetVR()
{
#if HMD_MODULE_INCLUDED
	if (GEngine->XRSystem.IsValid())
	{
		GEngine->XRSystem->ResetOrientationAndPosition();
		InternalCamera->SetRelativeLocation(InternalCameraOrigin);
		GetController()->SetControlRotation(FRotator());
	}
#endif // HMD_MODULE_INCLUDED
}

void AMap2Pawn::openCom() {
	char com[50];
	sprintf(com, "\\\\.\\COM%d", port);
	//strcat(portName, com);
	FString Fs = FString(ANSI_TO_TCHAR(com));
	UE_LOG(LogTemp, Log, TEXT("port:%s"), *Fs);
	stm32 = new SerialPort(com);

	if (stm32->isConnected()) {
		UE_LOG(LogTemp, Log, TEXT("Connected!!"));
		isStart = 1;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("NOT Connected!!"));
	}

	if (isStart)
	{
		for (TObjectIterator<AMyTriggerBox> It; It; ++It)
		{
			It->card = stm32;
		}
	}
	
}

void AMap2Pawn::exampleReceiveData(float delta)
{
	char signal1[] = "hiz";
	char signal2[] = "geri";
	char signal3[] = "sag";
	char signal4[] = "sol";
	char signal5[] = "duz";
	char signal6[] = "pulse";
	char signal7[] = "";
	char signal8[] = ".";
	char signal9[] = "yon";
	char pulse[10];
	char yon[10];

	int readResult = stm32->readSerialPort(incomingData, MAX_DATA_LENGTH);
	//nabiz_int = atoi(incomingData);
	if (strstr(incomingData, signal1))
	{
		//UE_LOG(LogTemp, Log, TEXT("ileri"));
		GetVehicleMovementComponent()->SetThrottleInput(1.0);

	}
	if (strstr(incomingData, signal2))
	{
		UE_LOG(LogTemp, Log, TEXT("geri"));
		GetVehicleMovementComponent()->SetThrottleInput(-1.0);

	}
	if (strstr(incomingData, signal3) && !sa)
	{
		UE_LOG(LogTemp, Log, TEXT("sag"));
		GetVehicleMovementComponent()->SetSteeringInput(0.20);
		sa = 1;
		steerString = 1.0;

	}
	if (strstr(incomingData, signal4) && !so)
	{
		UE_LOG(LogTemp, Log, TEXT("sol"));
		GetVehicleMovementComponent()->SetSteeringInput(-0.20);
		so = 1;
		steerString = -1.0;

	}
	if (strstr(incomingData, signal5))
	{
		//UE_LOG(LogTemp, Log, TEXT("ileri"));
		//GetVehicleMovementComponent()->SetThrottleInput(1.0);
		sa = 0;
		so = 0;
		GetVehicleMovementComponent()->SetSteeringInput(0.0);
		steerString = 0.0;

	}
	if (strstr(incomingData, signal6) && strstr(incomingData, signal8))
	{
		
		char* pos = strstr(incomingData, "pulse");
		FString Fs2 = FString(ANSI_TO_TCHAR(pos));
		UE_LOG(LogTemp, Log, TEXT("pos:%s"),*Fs2);
		strncpy(pulse, pos + 5, 5);
		nabiz_int = atoi(pulse);
		/*FString Fs = FString::SanitizeFloat((float)nabiz_int);
		UE_LOG(LogTemp, Log, TEXT("nabiz: %s"), *Fs);
		if (nabiz_int < 200)
		{
			UE_LOG(LogTemp, Log, TEXT("sol"));

			//FString Fs = FString::SanitizeFloat((float)nabiz_int);
			//UE_LOG(LogTemp, Log, TEXT("nabiz: %s"), *Fs);
			GetVehicleMovementComponent()->SetSteeringInput(-0.20);
			so = 1;
			steerString = -1.0;
		}
		if (nabiz_int > 3000)
		{
			UE_LOG(LogTemp, Log, TEXT("sag"));
			GetVehicleMovementComponent()->SetSteeringInput(0.20);
			sa = 1;
			steerString = 1.0;
		}
		if(nabiz_int > 200 && nabiz_int < 3000)
		{
			UE_LOG(LogTemp, Log, TEXT("duz"));
			GetVehicleMovementComponent()->SetSteeringInput(0.0);
			sa = 0;
			so = 0;
			steerString = 0.0;
		}*/
	}
	if (strstr(incomingData, signal9) && strstr(incomingData, signal8))
	{
		char* pos = strstr(incomingData, "yon");
		FString Fs2 = FString(ANSI_TO_TCHAR(pos));
		UE_LOG(LogTemp, Log, TEXT("pos:%s"), *Fs2);
		strncpy(yon, pos + 3, 5);
		int value = atoi(yon);
		FString Fs = FString::SanitizeFloat((float)value);
		UE_LOG(LogTemp, Log, TEXT("nabiz: %s"), *Fs);
		if (value > 2100)
		{
			UE_LOG(LogTemp, Log, TEXT("sol"));

			//FString Fs = FString::SanitizeFloat((float)nabiz_int);
			//UE_LOG(LogTemp, Log, TEXT("nabiz: %s"), *Fs);
			float donus = (1100.0 * 90.0) / ((float)value * 100.0);
			FString Fs3 = FString::SanitizeFloat(donus);
			UE_LOG(LogTemp, Log, TEXT("donus: %s"), *Fs3);
			GetVehicleMovementComponent()->SetSteeringInput((donus-0.45));
			so = 1;
			steerString = donus-0.45;
		}
		if (value < 2100)
		{
			UE_LOG(LogTemp, Log, TEXT("sag"));
			float donus = (1100.0 * 90.0) / ((float)value * 100.0);
			FString Fs3 = FString::SanitizeFloat(donus);
			UE_LOG(LogTemp, Log, TEXT("donus: %s"), *Fs3);
			GetVehicleMovementComponent()->SetSteeringInput(donus-0.45);
			sa = 1;
			steerString = donus - 0.45;
		}
		/*if(value > 200 && value < 3000)
		{
			UE_LOG(LogTemp, Log, TEXT("duz"));
			GetVehicleMovementComponent()->SetSteeringInput(0.0);
			sa = 0;
			so = 0;
			steerString = 0.0;
		}*/

	}
	float KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
	int32 KPH_int = FMath::FloorToInt(KPH);
	/*if ((KPH_int > 3) && ((KPH_int % 5 == 0) || (KPH_int >= 16)) && strstr(incomingData, signal7))
	{
		//FString Fs = FString::SanitizeFloat(delta);
		//UE_LOG(LogTemp, Log, TEXT("delta: %s"), *Fs);
		UE_LOG(LogTemp, Log, TEXT("empty"));
		GetVehicleMovementComponent()->SetThrottleInput(0.0);
		GetVehicleMovementComponent()->SetSteeringInput(0.0);
	}*/
	if (KPH_int >= 16)
	{
		//FString Fs = FString::SanitizeFloat(delta);
		//UE_LOG(LogTemp, Log, TEXT("delta: %s"), *Fs);
		//UE_LOG(LogTemp, Log, TEXT("empty"));
		//GetVehicleMovementComponent()->SetThrottleInput(0.0);
	}


	//FString fs(incomingData);
	//FString Fs = FString(ANSI_TO_TCHAR(incomingData));
	//UE_LOG(LogTemp, Log, TEXT("%s"),*Fs);
}

void AMap2Pawn::exampleWriteData(unsigned int delayTime)
{
	stm32->writeSerialPort(ledON1, strlen(ledON1));
}
void* AMap2Pawn::getSerial()
{
	return stm32;
}

void AMap2Pawn::UpdateHUDStrings()
{
	float KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
	int32 KPH_int = FMath::FloorToInt(KPH);
	int32 Gear = GetVehicleMovement()->GetCurrentGear();

	// Using FText because this is display text that should be localizable
	SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "Your Speed: {0} km/h"), FText::AsNumber(KPH_int));
	NabizString = FText::Format(LOCTEXT("SpeedFormat", "Your Pulse: {0}"), FText::AsNumber(nabiz_int));
	

	/*if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Some debug message!"));
	if (bInReverseGear == true)
	{
		GearDisplayString = FText(LOCTEXT("ReverseGear", "R"));
	}
	else
	{
		GearDisplayString = (Gear == 0) ? LOCTEXT("N", "N") : FText::AsNumber(Gear);
	}*/

}

void AMap2Pawn::SetupInCarHUD()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if ((PlayerController != nullptr) && (InCarSpeed != nullptr) && (InCarGear != nullptr))
	{
		// Setup the text render component strings
		InCarSpeed->SetText(SpeedDisplayString);
		InCarGear->SetText(GearDisplayString);
		
		if (bInReverseGear == false)
		{
			InCarGear->SetTextRenderColor(GearDisplayColor);
		}
		else
		{
			InCarGear->SetTextRenderColor(GearDisplayReverseColor);
		}
	}
}

void AMap2Pawn::UpdatePhysicsMaterial()
{
	if (GetActorUpVector().Z < 0)
	{
		if (bIsLowFriction == true)
		{
			GetMesh()->SetPhysMaterialOverride(NonSlipperyMaterial);
			bIsLowFriction = false;
		}
		else
		{
			GetMesh()->SetPhysMaterialOverride(SlipperyMaterial);
			bIsLowFriction = true;
		}
	}
}

#undef LOCTEXT_NAMESPACE

PRAGMA_ENABLE_DEPRECATION_WARNINGS
