// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MinimalGASHUD.generated.h"

UCLASS()
class AMinimalGASHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMinimalGASHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

