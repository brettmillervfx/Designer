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
#include "UObject/GCObject.h"

// Local Includes
#include "Tools/DesignerTool.h"

// Forward Declares
class AActor;
class UDesignerSettings;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

/**
 * Tool for spawning assets from the content browser.
 */
class FSpawnAssetTool : public FDesignerTool
{

public:
	FSpawnAssetTool(UDesignerSettings* InDesignerSettings);

	//~ Begin FDesignerTool interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	/** Returns the name that gets reported to the editor. */
	virtual FString GetName() const override;

	/** Called by the designer ed mode when switching to this tool */
	virtual void EnterTool() override;

	/** Called by the designer ed mode when switching to another tool from this tool */
	virtual void ExitTool() override;

	/** Check to see if an actor can be selected in this mode - no side effects */
	virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const override;
	//~ End FDesignerTool interface

	// User input

	//~ Begin FModeTool interface
	virtual bool MouseEnter(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;

	virtual bool MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport) override;

	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;

	virtual bool ReceivedFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport) override;

	virtual bool LostFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport) override;

	virtual bool CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;

	virtual bool InputAxis(FEditorViewportClient* InViewportClient, FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime) override;

	virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;

	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;

	virtual void Render(const FSceneView* View,FViewport* Viewport,FPrimitiveDrawInterface* PDI) override;

	virtual bool StartModify() override;
	virtual bool EndModify() override;

	// Tick
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;

	virtual bool BoxSelect(FBox& InBox, bool InSelect = true) override;

	virtual bool FrustumSelect(const FConvexVolume& InFrustum, FEditorViewportClient* InViewportClient, bool InSelect = true) override;
	//~ End FModeTool interface

	UDesignerSettings* GetDesignerSettings() const;

	AActor* GetControlledActor() const;

private:
	/** Update the material parameters for the spawn visualizer component. Returns true if it was successful */
	bool UpdateSpawnVisualizerMaterialParameters();

	/** Calculate the world transform for the mouse and store it in MouseDownWorldTransform. Returns true if it was successful */
	bool RecalculateSpawnTransform(FEditorViewportClient* ViewportClient, FViewport* Viewport);
	
	/** Recalculate the world transform of the mouse and store it in the CurrentMouseWorldTransform. Returns true if it was successful */
	void RecalculateMousePlaneIntersectionWorldLocation(FEditorViewportClient* ViewportClient, FViewport* Viewport);

	/** Updates the designer actor transform so it matches with all the changes made to DesignerActorTransformExcludingOffset */
	void UpdateDesignerActorTransform();

	/** Generate new random rotation offset */
	void RegenerateRandomRotationOffset();

	/** Get the random rotation applied to the designer actor */
	FRotator GetRandomRotationOffset() const;

	/** Generate new random scale */
	void RegenerateRandomScale();

	/** The random scale applied to the designer actor */
	FVector GetRandomScale() const;

	/** Get the designer actor rotation with all settings applied to it */
	FRotator GetDesignerActorRotation();

private:
	/** The static mesh of the Spawn visualizer component */
	UStaticMeshComponent* SpawnVisualizerComponent;

	/** The material instance dynamic of the Spawn visualizer component */
	UMaterialInstanceDynamic* SpawnVisualizerMID;

	/** The plane we trace against when transforming the placed actor */
	FPlane SpawnTracePlane;

	/** The world transform stored on mouse click down */
	FTransform SpawnWorldTransform;

	/** When spawning an object the mouse traces with a plane to determine the size and rotation. This is the world space hit location on that plane */
	FVector CursorPlaneIntersectionWorldLocation;

	/** The settings available to the user */
	UDesignerSettings* DesignerSettings;

	/** The actor currently controlled by the designer editor mode */
	AActor* SpawnedActor;

	/** The local box extent of the selected designer actor in cm when scale is uniform 1 */
	FVector DefaultDesignerActorExtent;
};
