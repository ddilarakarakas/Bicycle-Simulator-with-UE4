// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTriggerBox.h"
#include "DrawDebugHelpers.h"
#include "USerial.h"
#include "Map2Pawn.h"
//#include <Map2\Map2Pawn.cpp>
int isIn = 0;
char ledON[] = "Yokus\n";
char ledOFF[] = "Normal\n";
AMyTriggerBox::AMyTriggerBox()
{
    //Register Events
    OnActorBeginOverlap.AddDynamic(this, &AMyTriggerBox::OnOverlapBegin);
    OnActorEndOverlap.AddDynamic(this, &AMyTriggerBox::OnOverlapEnd);
}

// Called when the game starts or when spawned
void AMyTriggerBox::BeginPlay()
{
    Super::BeginPlay();

   // DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Green, true, 999, 0, 5);
}

void AMyTriggerBox::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
    //if the overlapping actor is the specific actor we identified in the editor
    if (OtherActor && OtherActor != this && OtherActor == SpecificActor)
    {
        if (GEngine)
        {
            //char ledON[] = "Yokus\n";
            //char ledOFF[] = "Normal\n";
            if (!isIn)
            {
               // stm32->writeSerialPort(ledON, strlen(ledON));
                for (TObjectIterator<AMap2Pawn> It; It; ++It)
                {
                    // Grab whatever data.
                    ((SerialPort*)It->getSerial())->writeSerialPort(ledON, strlen(ledON));
                }
                isIn = 1;
            }
            else {
                //stm32->writeSerialPort(ledOFF, strlen(ledOFF));
                for (TObjectIterator<AMap2Pawn> It; It; ++It)
                {
                    // Grab whatever data.
                    ((SerialPort*)It->getSerial())->writeSerialPort(ledOFF, strlen(ledOFF));
                }
                isIn = 0;
            }
        }
    }
}

void AMyTriggerBox::OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor)
{
    //if the overlapping actor is the specific actor we identified in the editor
    if (OtherActor && OtherActor != this && OtherActor == SpecificActor)
    {
        if (GEngine)
        {
            //stm32->writeSerialPort(ledOFF, strlen(ledOFF));
            for (TObjectIterator<AMap2Pawn> It; It; ++It)
            {
                
                // Grab whatever data.
                ((SerialPort*)It->getSerial())->writeSerialPort(ledOFF, strlen(ledOFF));
            }
            //GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green, TEXT("Overlap Ended"));
            //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta, FString::Printf(TEXT("%s has left the Trigger Box"), *OtherActor->GetName()));
        }
    }
}