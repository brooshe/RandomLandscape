// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include "FoliageMaskVolume.h" 



void AFoliageMaskVolume::OnConstruction(const FTransform& Transform)
{
	if (PREVIOUS_Location != GetActorLocation()) 
	{
		NeedRefresh = true;
	}

	if (PREVIOUS_Rotation != GetActorRotation()) 
	{
		NeedRefresh = true;
	}

	if (PREVIOUS_Scale != GetActorScale()) 
	{
		NeedRefresh = true;
	}

	if (FoliageLayerMask.Num() != PREVIOUS_FoliageLayerMask.Num()) 
	{
		NeedRefresh = true;
	}
	else {
		for (int i = 0; i < FoliageLayerMask.Num(); i++) 
		{
			if (FoliageLayerMask.IsValidIndex(i) && PREVIOUS_FoliageLayerMask.IsValidIndex(i))
			{
				if (FoliageLayerMask[i] != PREVIOUS_FoliageLayerMask[i]) 
				{
					NeedRefresh = true;
					break;
				}
			}
			else 
			{
				NeedRefresh = true;
				break;
			}
		}
	}


	if (NeedRefresh)
	{
		PREVIOUS_Location = GetActorLocation();
		PREVIOUS_Rotation = GetActorRotation();
		PREVIOUS_Scale = GetActorScale();
		PREVIOUS_FoliageLayerMask = FoliageLayerMask;
		PREVIOUS_FoliageLayerMask = FoliageLayerMask;
	}
	

}





AFoliageMaskVolume::AFoliageMaskVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	FoliageLayerMask.Empty();
}


