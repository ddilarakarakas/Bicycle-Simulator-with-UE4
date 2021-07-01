// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map2Hud.h"
#include "Map2Pawn.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

// Needed for VR Headset
#include "Engine.h"

#define LOCTEXT_NAMESPACE "VehicleHUD"

#ifndef HMD_MODULE_INCLUDED
#define HMD_MODULE_INCLUDED 0
#endif

AMap2Hud::AMap2Hud()
{
	static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
	HUDFont = Font.Object;
}

void AMap2Hud::DrawHUD()
{
	Super::DrawHUD();

	// Calculate ratio from 720p
	const float HUDXRatio = Canvas->SizeX / 1280.f;
	const float HUDYRatio = Canvas->SizeY / 720.f;

	bool bHMDDeviceActive = false;

	// We dont want the onscreen hud when using a HMD device	
#if HMD_MODULE_INCLUDED
	bHMDDeviceActive = GEngine->IsStereoscopic3D();
#endif // HMD_MODULE_INCLUDED
	if( bHMDDeviceActive == false )
	{
		// Get our vehicle so we can check if we are in car. If we are we don't want onscreen HUD
		AMap2Pawn* Vehicle = Cast<AMap2Pawn>(GetOwningPawn());
		if ((Vehicle != nullptr) && (Vehicle->bInCarCameraActive == false))
		{
			FVector2D ScaleVec(HUDYRatio * 1.4f, HUDYRatio * 1.4f);

			// Speed
			FCanvasTextItem SpeedTextItem(FVector2D(HUDXRatio * 950.f, HUDYRatio * 55), Vehicle->SpeedDisplayString, HUDFont, FLinearColor::White);
			SpeedTextItem.Scale = ScaleVec;
			Canvas->DrawItem(SpeedTextItem);

			//Nabiz
			FCanvasTextItem PulseTextItem(FVector2D(HUDXRatio * 950.f, HUDYRatio * 100), Vehicle->NabizString, HUDFont, FLinearColor::Yellow);
			PulseTextItem.Scale = ScaleVec;
			Canvas->DrawItem(PulseTextItem);

			// Gear
			/*FCanvasTextItem GearTextItem(FVector2D(HUDXRatio * 1000.f, HUDYRatio * 100.f), Vehicle->GearDisplayString, HUDFont, Vehicle->bInReverseGear == false ? Vehicle->GearDisplayColor : Vehicle->GearDisplayReverseColor);
			GearTextItem.Scale = ScaleVec;
			Canvas->DrawItem(GearTextItem);*/
		}
		else if ((Vehicle != nullptr) && (Vehicle->bInCarCameraActive == true)) {
			FVector2D ScaleVec(HUDYRatio * 1.4f, HUDYRatio * 1.4f);

			// Speed
			FCanvasTextItem SpeedTextItem(FVector2D(HUDXRatio * 950.f, HUDYRatio * 55), Vehicle->SpeedDisplayString, HUDFont, FLinearColor::White);
			SpeedTextItem.Scale = ScaleVec;
			Canvas->DrawItem(SpeedTextItem);

			//Nabiz
			FCanvasTextItem PulseTextItem(FVector2D(HUDXRatio * 950.f, HUDYRatio * 100), Vehicle->NabizString, HUDFont, FLinearColor::Yellow);
			PulseTextItem.Scale = ScaleVec;
			Canvas->DrawItem(PulseTextItem);
		}
	}
}

#undef LOCTEXT_NAMESPACE
