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
#include "DesignerEdMode.h"

// Engine Includes
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

// Local Includes
#include "DesignerEdModeToolkit.h"
#include "DesignerModule.h"
#include "DesignerSettings.h"

#include "Tools/DesignerTool.h"
#include "Tools/SpawnAssetTool.h"

const FEditorModeID FDesignerEdMode::EM_DesignerEdModeId = TEXT("EM_DesignerEdMode");

FDesignerEdMode::FDesignerEdMode()
{
	DesignerSettings = NewObject<UDesignerSettings>(GetTransientPackage(), TEXT("DesignerEdModeSettings"), RF_Transactional);
	DesignerSettings->SetParent(this);

	SpawnAssetTool = new FSpawnAssetTool(DesignerSettings);
}

void FDesignerEdMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	// Call parent implementation
	FEdMode::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(DesignerSettings);
	
}

TSharedPtr<class FModeToolkit> FDesignerEdMode::GetToolkit()
{
	return Toolkit;
}

void FDesignerEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FDesignerEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}

	SwitchTool(nullptr);
}

void FDesignerEdMode::Exit()
{
	SwitchTool(nullptr);

	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FDesignerEdMode::LostFocus(FEditorViewportClient * ViewportClient, FViewport * Viewport)
{
	bool bHandled = FEdMode::LostFocus(ViewportClient, Viewport);
	SwitchTool(nullptr);

	return bHandled;
}

bool FDesignerEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	bool bHandled = false;

	if (Key == EKeys::LeftControl || Key == EKeys::RightControl)
	{
		if (Event == IE_Pressed)
		{
			SwitchTool(SpawnAssetTool);
			bHandled = true;
		}
		else if (Event == IE_Released)
		{
			SwitchTool(nullptr);
			bHandled = true;
		}
	}

	bool bHandledInSuper = FEdMode::InputKey(ViewportClient, Viewport, Key, Event);

	return bHandled || bHandledInSuper;
}

bool FDesignerEdMode::DisallowMouseDeltaTracking() const
{
	return CurrentTool != nullptr;
}

bool FDesignerEdMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	return false;
}

bool FDesignerEdMode::IsSelectionAllowed(AActor* InActor, bool bInSelection) const
{
	bool bResult = true;
	if (FDesignerTool * CurrentDesignerTool = static_cast<FDesignerTool*>(CurrentTool))
	{
		bResult = CurrentDesignerTool->IsSelectionAllowed(InActor, bInSelection);
	}
	return bResult;
}

bool FDesignerEdMode::UsesToolkits() const
{
	return true;
}

void FDesignerEdMode::SwitchTool(FDesignerTool* NewDesignerTool)
{
	FDesignerTool* CurrentDesignerTool = static_cast<FDesignerTool*>(CurrentTool);
	if (CurrentDesignerTool != nullptr)
	{
		CurrentDesignerTool->ExitTool();
	}

	if (NewDesignerTool != nullptr)
	{
		NewDesignerTool->EnterTool();
		SetCurrentTool(NewDesignerTool);
	}
	else
	{
		CurrentTool = nullptr;
	}	
}
