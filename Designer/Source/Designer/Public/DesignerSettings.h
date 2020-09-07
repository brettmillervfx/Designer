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

#pragma once

// Engine Includes
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

// Generated Include
#include "DesignerSettings.generated.h"

// Forward Declares
class FDesignerEdMode;

UENUM()
enum class EAxisType : uint8
{
	/** None */
	None = 0x00 UMETA(DisplayName = "None"),

	/** The forward vector */
	Forward = 0x02 UMETA(DisplayName = "Forward (+X)"),

	/** The backward vector */
	Backward = 0x03 UMETA(DisplayName = "Backward (-X)"),

	/** The right vector */
	Right = 0x04 UMETA(DisplayName = "Right (+Y)"),

	/** The left vector */
	Left = 0x05 UMETA(DisplayName = "Left (-Y)"),

	/** The up vector */
	Up = 0x08 UMETA(DisplayName = "Up (+Z)"),

	/** The down vector */
	Down = 0x09 UMETA(DisplayName = "Down (-Z)")
};

/**
 * A random float within a min max range
 * Option for randomly negating the value
 */
USTRUCT(BlueprintType)
struct FRandomMinMaxFloat
{
	GENERATED_BODY()

public:
	FRandomMinMaxFloat();

	FRandomMinMaxFloat(float Min, float Max, bool bRandomlyNegateValue);

	/** Get the random value currently stored in this struct */
	float GetCurrentRandomValue() const;

	/** Regenerates the random value and returns it. The value can also be retrieved later as well using GetCurrentRandomValue */
	float RegenerateRandomValue();

public:
	/** The minimal value */
	UPROPERTY(EditAnywhere)
	float Min;

	/** The maximum value */
	UPROPERTY(EditAnywhere)
	float Max;

	/**
	 * Is this value allowed to randomly flip the sign of the generated value?
	 * i.e. if Min = 30 and Max = 30 the outcome can be either 30 or -30 when this is set to true
	 */
	UPROPERTY(EditAnywhere)
	bool bRandomlyNegateValue;

private:
	/** The randomly generated value */
	UPROPERTY(Transient)
	float RandomValue=0.F;

};

/**
 * The settings shown the in editor mode details panel
 */
UCLASS()
class DESIGNER_API UDesignerSettings : public UObject
{
	GENERATED_BODY()

public:
	UDesignerSettings(const FObjectInitializer& ObjectInitializer);

	/**
	* Always returns the positive axis of the current selected AxisToAlignWithCursor
	* i.e. Backward becomes Forward while Up stays Up.
	*/
	EAxisType GetPositiveAxisToAlignWithCursor() const;

	void SetParent(FDesignerEdMode* DesignerEdMode);

public:
	/** The spawn location offset in relative space */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	FVector RelativeLocationOffset;

	/** The spawn rotation offset in world space */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	FVector WorldLocationOffset;

	/** Actor axis vector to align with the hit surface direction */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	EAxisType AxisToAlignWithNormal;

	/** Actor axis vector to align with the cursor direction */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	EAxisType AxisToAlignWithCursor;

	/** Is the rotation x axis snapped to the grid set in the viewport grid settings */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	bool bSnapToGridRotationX;

	/** Is the rotation y axis snapped to the grid set in the viewport grid settings */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	bool bSnapToGridRotationY;

	/** Is the rotation z axis snapped to the grid set in the viewport grid settings */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	bool bSnapToGridRotationZ;
	
	/** Randomly rotates the mesh */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	bool bApplyRandomRotation;
	
	/** Random rotation offset applied to the x axis rotation matrix on spawn */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere, meta = (EditCondition = "bApplyRandomRotation"))
	FRandomMinMaxFloat RandomRotationX;

	/** Random rotation offset applied to the y axis rotation matrix on spawn */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere, meta = (EditCondition = "bApplyRandomRotation"))
	FRandomMinMaxFloat RandomRotationY;

	/** Random rotation offset applied to z axis the rotation matrix on spawn */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere, meta = (EditCondition = "bApplyRandomRotation"))
	FRandomMinMaxFloat RandomRotationZ;

	/** Scale the bounds of the mesh towards the cursor location */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	bool bScaleBoundsTowardsCursor;
	
	/** Randomly scale the mesh */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere)
	bool bApplyRandomScale;

	/** Random scale for x axis */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere, meta = (EditCondition = "bApplyRandomScale"))
	FRandomMinMaxFloat RandomScaleX;

	/** Random scale for y axis */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere, meta = (EditCondition = "bApplyRandomScale"))
	FRandomMinMaxFloat RandomScaleY;

	/** Random scale for z axis */
	UPROPERTY(Category = "SpawnSettings", NonTransactional, EditAnywhere, meta = (EditCondition = "bApplyRandomScale"))
	FRandomMinMaxFloat RandomScaleZ;

private:
	FDesignerEdMode* ParentEdMode;

};
