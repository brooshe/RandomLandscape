// Copyright 2021 IOLACORP STUDIO. All Rights Reserved
#include "FoliageSector.h"

FFoliageSector::FFoliageSector(DVector pPosition, double pSize) 
{
		Position = pPosition;
		Size = pSize;
		FoliageSpawned = false;
}

bool FFoliageSector::operator==(const FFoliageSector& sector) const
{
	return (sector.Position == this->Position && sector.Size == this->Size);
}

bool FFoliageSector::operator!=(const FFoliageSector& sector) const
{
	return (sector.Position != this->Position && sector.Size != this->Size);
}

FBox FFoliageSector::GetFBox(DVector Offset, double scale)
{
	return FBox((Position + Offset - DVector(Size * scale * 0.5)).ToFVector(), (Position + DVector(Size * scale * 0.5) + Offset).ToFVector());
}
