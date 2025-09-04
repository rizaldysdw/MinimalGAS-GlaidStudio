// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SimplePlayerController.generated.h"

UCLASS()
class MINIMALGAS_API ASimplePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ASimplePlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};