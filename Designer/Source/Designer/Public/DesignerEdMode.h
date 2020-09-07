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

// Engine includes
#include "CoreMinimal.h"
#include "EdMode.h"

// Local includes
#include "Tools/SpawnAssetTool.h"

// Forward Declares
class UDesignerSettings;
class FSpawnAssetTool;

class FDesignerEdMode : public FEdMode
{

public:
	FDesignerEdMode();

	//~ Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject interface

	//~ Begin FEdMode interface
	TSharedPtr<class FModeToolkit> GetToolkit();

	virtual void Enter() override;
	virtual void Exit() override;
		
	bool LostFocus(FEditorViewportClient * ViewportClient, FViewport * Viewport);
	bool InputKey(FEditorViewportClient * ViewportClient, FViewport * Viewport, FKey Key, EInputEvent Event);

	/** If the Edmode is handling its own mouse deltas, it can disable the MouseDeltaTacker */
	virtual bool DisallowMouseDeltaTracking() const;

	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click);

	/** Check to see if an actor can be selected in this mode - no side effects */
	virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const;
	
	/** True if this mode uses a toolkit mode (eventually they all should) */
	bool UsesToolkits() const override;
	//~ End FEdMode interface


	/** Set the current tool to the new designer tool while also calling ExitTool on the previous DesignerTool and EnterTool on the NewDesignerTool */
	void SwitchTool(FDesignerTool* NewDesignerTool);

	/** The settings available to the user */
	UDesignerSettings* GetDesignerSettings() const 
	{ 
		return DesignerSettings; 
	}

public:
	const static FEditorModeID EM_DesignerEdModeId;

private:
	UDesignerSettings* DesignerSettings;
	FSpawnAssetTool* SpawnAssetTool;
};
