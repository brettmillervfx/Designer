/**
 * MIT License
 * 
 * Copyright(c) 2018 RoelBartstra
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// This Include
#include "DesignerSettings.h"

// Local Includes
#include "DesignerEdMode.h"

FRandomMinMaxFloat::FRandomMinMaxFloat()
{
	Min = 0.F;
	Max = 1.F;
	bRandomlyNegateValue = false;
	RandomValue = 0.5;
}

FRandomMinMaxFloat::FRandomMinMaxFloat(float Min, float Max, bool bRandomlyNegateValue = false)
{
	Min = Min;
	Max = Max;
	bRandomlyNegateValue = bRandomlyNegateValue;
	RandomValue = (Min + Max) / 2.0;

}

/** Get the random value currently stored in this struct */
float FRandomMinMaxFloat::GetCurrentRandomValue() const
{
	return RandomValue;
}

/** Regenerates the random value and returns it. The value can also be retrieved later as well using GetCurrentRandomValue */
float FRandomMinMaxFloat::RegenerateRandomValue()
{
	RandomValue = FMath::RandRange(Min, Max);
	if (FMath::RandBool() && bRandomlyNegateValue)
	{
		RandomValue *= -1.F;
	}
	return RandomValue;
}

UDesignerSettings::UDesignerSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RelativeLocationOffset(FVector::ZeroVector)
	, WorldLocationOffset(FVector::ZeroVector)
	, AxisToAlignWithNormal(EAxisType::Up)
	, AxisToAlignWithCursor(EAxisType::Forward)
	, bSnapToGridRotationX(false)
	, bSnapToGridRotationY(false)
	, bSnapToGridRotationZ(false)
	, bApplyRandomRotation(false)
	, RandomRotationX(FRandomMinMaxFloat(0.F, 360.F))
	, RandomRotationY(FRandomMinMaxFloat(0.F, 360.F))
	, RandomRotationZ(FRandomMinMaxFloat(0.F, 360.F))
	, bScaleBoundsTowardsCursor(false)
	, bApplyRandomScale(false)
	, RandomScaleX(FRandomMinMaxFloat(0.8F, 1.2F, true))
	, RandomScaleY(FRandomMinMaxFloat(0.8F, 1.2F, true))
	, RandomScaleZ(FRandomMinMaxFloat(0.8F, 1.2F, true))
{
}

EAxisType UDesignerSettings::GetPositiveAxisToAlignWithCursor() const
{
	return (EAxisType)(~1 & (int)AxisToAlignWithCursor);
}

void UDesignerSettings::SetParent(FDesignerEdMode* DesignerEdMode)
{
	ParentEdMode = DesignerEdMode;
}