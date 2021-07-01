// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "USerial.h"
#include "Engine/TriggerBox.h"
#include "MyTriggerBox.generated.h"

UCLASS()
class MAP2_API AMyTriggerBox : public ATriggerBox
{
	GENERATED_BODY()
protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// constructor sets default values for this actor's properties
	AMyTriggerBox();

	// overlap begin function
	//send a signal to the stm32 card
	UFUNCTION()
		void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	// overlap end function
		//send a signal again to the stm32 card
	UFUNCTION()
		void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

	// specific actor for overlap
	//bicycle in simulation
	UPROPERTY(EditAnywhere)
		class AActor* SpecificActor;

	SerialPort* card;
	
	int times = 0;
};


