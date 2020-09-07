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
#include "SpawnAssetTool.h"

// Engine Includes
#include "Editor.h"

#include "UObject/Class.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/CollisionProfile.h"

#include "Editor/EditorEngine.h"
#include "Engine/Selection.h"
#include "AssetSelection.h"
#include "EditorViewportClient.h"
#include "SnappingUtils.h"
#include "Editor/UnrealEd/Private/Editor/ActorPositioning.h"

// Local Includes
#include "DesignerModule.h"
#include "DesignerSettings.h"


FSpawnAssetTool::FSpawnAssetTool(UDesignerSettings* InDesignerSettings)
{
	DesignerSettings = InDesignerSettings;

	UStaticMesh* StaticMesh = nullptr;
	if (!IsRunningCommandlet())
	{
		UMaterialInterface* SpawnVisualizerMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Designer/MI_SpawnVisualizer.MI_SpawnVisualizer"), nullptr, LOAD_None, nullptr);
		check(SpawnVisualizerMaterial != nullptr);
		SpawnVisualizerMID = UMaterialInstanceDynamic::Create(SpawnVisualizerMaterial, GetTransientPackage());
		check(SpawnVisualizerMID != nullptr);
		StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Designer/SM_SpawnVisualizer.SM_SpawnVisualizer"), nullptr, LOAD_None, nullptr);
		check(StaticMesh != nullptr);
	}

	SpawnVisualizerComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("SpawnVisualizerComponent"));
	SpawnVisualizerComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SpawnVisualizerComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SpawnVisualizerComponent->SetStaticMesh(StaticMesh);
	SpawnVisualizerComponent->SetMaterial(0, SpawnVisualizerMID);
	SpawnVisualizerComponent->SetAbsolute(true, true, true);
	SpawnVisualizerComponent->CastShadow = false;
}

void FSpawnAssetTool::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(DesignerSettings);
	Collector.AddReferencedObject(SpawnVisualizerComponent);
}

FString FSpawnAssetTool::GetName() const
{
	return TEXT("SpawnAssetTool");
}

void FSpawnAssetTool::EnterTool()
{
	SpawnedActor = nullptr;

	SpawnVisualizerComponent->SetVisibility(true);
}

void FSpawnAssetTool::ExitTool()
{
	SpawnedActor = nullptr;

	if (SpawnVisualizerComponent->IsRegistered())
	{
		SpawnVisualizerComponent->UnregisterComponent();
	}
}

bool FSpawnAssetTool::IsSelectionAllowed(AActor* InActor, bool bInSelection) const
{
	if (SpawnedActor)
		return InActor == SpawnedActor && !FMath::IsNearlyZero(SpawnedActor->GetActorScale3D().Size());

	// Make sure select none works when spawning actor.
	if (InActor != SpawnedActor && !bInSelection)
		return true;

	// While it's nice to control selectibility in this mode, we require selection to 
	// remain compatible with the Tableau workflow.
	//return false;
	return true;
}

bool FSpawnAssetTool::MouseEnter(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	return false;
}

bool FSpawnAssetTool::MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	return false;
}

bool FSpawnAssetTool::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	return false;
}

bool FSpawnAssetTool::ReceivedFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	return false;
}

bool FSpawnAssetTool::LostFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	return false;
}

bool FSpawnAssetTool::CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	bool bHandled = false;

	if (SpawnedActor == nullptr)
	{
		return bHandled;
	}
	else
	{
		RecalculateMousePlaneIntersectionWorldLocation(InViewportClient, InViewport);
		UpdateDesignerActorTransform();
		UpdateSpawnVisualizerMaterialParameters();

		bHandled = true;
	}

	return bHandled;
}

bool FSpawnAssetTool::InputAxis(FEditorViewportClient* InViewportClient, FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime)
{
	return false;
}

bool FSpawnAssetTool::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	return false;
}

bool FSpawnAssetTool::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	bool bHandled = false;

	// Randomize the object again if right mouse button is pressed in this mode.
	if (Key == EKeys::RightMouseButton)
	{
		if (Event == IE_Pressed && SpawnedActor != nullptr)
		{
			RegenerateRandomRotationOffset();
			RegenerateRandomScale();
			UpdateDesignerActorTransform();
			UpdateSpawnVisualizerMaterialParameters();
		}

		bHandled = true;
	}

	if (Key == EKeys::LeftMouseButton)
	{
		if (Event == IE_Pressed)
		{
			GEditor->SelectNone(true, true, false);

			TArray<FAssetData> ContentBrowserSelections;
			GEditor->GetContentBrowserSelections(ContentBrowserSelections);

			UClass* SelectedClass = GEditor->GetSelectedObjects()->GetTop<UClass>();

			bool bPlaceable = true;
			TArray<FAssetData> SelectedAssets;
			AssetSelectionUtils::GetSelectedAssets(SelectedAssets);
			FAssetData TargetAssetData;

			if (TargetAssetData.GetClass() == UClass::StaticClass())
			{
				UClass* Class = Cast<UClass>(TargetAssetData.GetAsset());

				bPlaceable = AssetSelectionUtils::IsClassPlaceable(Class);
			}

			if (SelectedAssets.Num() > 0)
			{
				TargetAssetData = SelectedAssets[FMath::RandRange(0, SelectedAssets.Num() - 1)];// SelectedAssets.Top();
			}

			if (TargetAssetData.GetClass() == UClass::StaticClass())
			{
				UClass* Class = Cast<UClass>(TargetAssetData.GetAsset());

				bPlaceable = AssetSelectionUtils::IsClassPlaceable(Class);
			}
			else if (TargetAssetData.GetClass() == UBlueprint::StaticClass())
			{
				// For blueprints, attempt to determine placeability from its tag information

				const FName NativeParentClassTag = TEXT("NativeParentClass");
				const FName ClassFlagsTag = TEXT("ClassFlags");

				FString TagValue;

				if (TargetAssetData.GetTagValue(NativeParentClassTag, TagValue) && !TagValue.IsEmpty())
				{
					// If the native parent class can't be placed, neither can the blueprint.
					UObject* Outer = nullptr;
					ResolveName(Outer, TagValue, false, false);
					UClass* NativeParentClass = FindObject<UClass>(ANY_PACKAGE, *TagValue);

					bPlaceable = AssetSelectionUtils::IsClassPlaceable(NativeParentClass);
				}

				if (bPlaceable && TargetAssetData.GetTagValue(ClassFlagsTag, TagValue) && !TagValue.IsEmpty())
				{
					// Check to see if this class is placeable from its class flags
					const int32 NotPlaceableFlags = CLASS_NotPlaceable | CLASS_Deprecated | CLASS_Abstract;
					uint32 ClassFlags = FCString::Atoi(*TagValue);

					bPlaceable = (ClassFlags & NotPlaceableFlags) == CLASS_None;
				}
			}

			UObject* TargetAsset = TargetAssetData.GetAsset();

			if (bPlaceable && IsValid(TargetAsset))
			{
				UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(TargetAsset);
				if (ActorFactory)
				{
					// Recalculate mouse down, if it fails, return.
					if (!RecalculateSpawnTransform(ViewportClient, Viewport))
						return bHandled;

					SpawnedActor = GEditor->UseActorFactory(ActorFactory, TargetAssetData, &SpawnWorldTransform);

					DefaultDesignerActorExtent = SpawnedActor->CalculateComponentsBoundingBoxInLocalSpace(true).GetExtent();

					// Properly reset data.
					CursorPlaneIntersectionWorldLocation = SpawnWorldTransform.GetLocation();
					SpawnTracePlane = FPlane();

					FTransform SpawnVisualizerTransform = SpawnWorldTransform;
					SpawnVisualizerTransform.SetScale3D(FVector(10000));
					SpawnVisualizerComponent->SetRelativeTransform(SpawnVisualizerTransform);

					if (!SpawnVisualizerComponent->IsRegistered())
					{
						SpawnVisualizerComponent->RegisterComponentWithWorld(ViewportClient->GetWorld());
					}

					RegenerateRandomRotationOffset();
					RegenerateRandomScale();
					UpdateDesignerActorTransform();
					UpdateSpawnVisualizerMaterialParameters();

					bHandled = true;
				}
			}
		}
		/** Left mouse button released */
		else if (Event == IE_Released)
		{
			if (SpawnedActor != nullptr && FMath::IsNearlyZero(SpawnedActor->GetActorScale3D().Size()))
			{
				SpawnedActor->Destroy(false, false);
				GEditor->RedrawLevelEditingViewports();
			}
			else
			{
				GEditor->SelectActor(SpawnedActor, true, true, true, true);
			}

			SpawnedActor = nullptr;
			DefaultDesignerActorExtent = FVector::ZeroVector;

			if (SpawnVisualizerComponent->IsRegistered())
			{
				SpawnVisualizerComponent->UnregisterComponent();
			}

			bHandled = true;
		}
	}

	return bHandled;
}

void FSpawnAssetTool::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{	
	if (SpawnedActor == nullptr)
	{
		DrawSphere(PDI, SpawnWorldTransform.GetLocation(), SpawnWorldTransform.GetRotation().Rotator(), FVector(5.F), 32, 32, GEngine->DebugEditorMaterial->GetRenderProxy(), SDPG_Foreground, false);
	}
}

bool FSpawnAssetTool::StartModify()
{
	return false;
}

bool FSpawnAssetTool::EndModify()
{
	return false;
}

void FSpawnAssetTool::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	if (SpawnedActor == nullptr)
		RecalculateSpawnTransform(ViewportClient, ViewportClient->Viewport);

}

bool FSpawnAssetTool::BoxSelect(FBox& InBox, bool InSelect)
{
	return false;
}

bool FSpawnAssetTool::FrustumSelect(const FConvexVolume& InFrustum, FEditorViewportClient* InViewportClient, bool InSelect)
{
	return false;
}

UDesignerSettings* FSpawnAssetTool::GetDesignerSettings() const 
{ 
	return DesignerSettings; 
}

AActor* FSpawnAssetTool::GetControlledActor() const 
{ 
	return SpawnedActor; 
}

bool FSpawnAssetTool::UpdateSpawnVisualizerMaterialParameters()
{
	if (SpawnVisualizerMID)
	{
		SpawnVisualizerMID->SetVectorParameterValue(FName("CursorInputDownWorldLocation"), FLinearColor(SpawnWorldTransform.GetLocation()));

		FVector Extent = DefaultDesignerActorExtent * SpawnedActor->GetActorScale3D();
		EAxisType PositiveAxis = DesignerSettings->GetPositiveAxisToAlignWithCursor();

		
		float ActorRadius = Extent.X;
		if (PositiveAxis == EAxisType::Right)
		{
			ActorRadius = Extent.Y;
		}
		else if (PositiveAxis == EAxisType::Up)
		{
			ActorRadius = Extent.Z;
		}
		ActorRadius = FMath::Abs(ActorRadius);

		SpawnVisualizerMID->SetVectorParameterValue(FName("CursorPlaneWorldLocation"), FLinearColor(CursorPlaneIntersectionWorldLocation.X, CursorPlaneIntersectionWorldLocation.Y, CursorPlaneIntersectionWorldLocation.Z, ActorRadius));

		FLinearColor ForwardVectorColor = FLinearColor::Red;
		if (PositiveAxis == EAxisType::Up)
		{
			ForwardVectorColor = FLinearColor::Blue;
		}
		else if (PositiveAxis == EAxisType::Right)
		{
			ForwardVectorColor = FLinearColor::Green;
		}


		SpawnVisualizerMID->SetVectorParameterValue(FName("ForwardAxisColor"), ForwardVectorColor);


		return true;
	}

	return false;
}

bool FSpawnAssetTool::RecalculateSpawnTransform(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	FTransform NewSpawnTransform = FTransform(FQuat::Identity, FVector::ZeroVector, FVector::OneVector);

	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		Viewport,
		ViewportClient->GetScene(),
		ViewportClient->EngineShowFlags)
		.SetRealtimeUpdate(ViewportClient->IsRealtime()));
	// SceneView is deleted with the ViewFamily
	FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily);

	FViewportCursorLocation MouseViewportRay(SceneView, ViewportClient, Viewport->GetMouseX(), Viewport->GetMouseY());

	FVector TraceStartLocation = MouseViewportRay.GetOrigin();
	FVector TraceDirection = MouseViewportRay.GetDirection();
	FVector TraceEndLocation = TraceStartLocation + TraceDirection * WORLD_MAX;
	if (ViewportClient->IsOrtho())
	{
		TraceStartLocation += -WORLD_MAX * TraceDirection;
	}

	FActorPositionTraceResult ActorPositionTraceResult = FActorPositioning::TraceWorldForPositionWithDefault(MouseViewportRay, *SceneView);

	if (ActorPositionTraceResult.HitActor == nullptr)
	{
		return false;
	}

	NewSpawnTransform.SetLocation(ActorPositionTraceResult.Location);

	FVector ZAxis = ActorPositionTraceResult.SurfaceNormal;
	if (GetDesignerSettings()->AxisToAlignWithNormal == EAxisType::None)
	{
		ZAxis = FVector::UpVector;
	}
	FVector XAxis = FVector::ForwardVector;

	FRotator CursorWorldRotation = FRotationMatrix::MakeFromZX(ZAxis, XAxis).Rotator();

	FRotator SpawnRotationSnapped = CursorWorldRotation;
	FSnappingUtils::SnapRotatorToGrid(SpawnRotationSnapped);

	if (GetDesignerSettings()->bSnapToGridRotationX)
	{
		CursorWorldRotation.Roll = SpawnRotationSnapped.Roll;
	}
	else
	{
		CursorWorldRotation.Roll = CursorWorldRotation.Roll;
	}

	if (GetDesignerSettings()->bSnapToGridRotationY)
	{
		CursorWorldRotation.Pitch = SpawnRotationSnapped.Pitch;
	}
	else
	{
		CursorWorldRotation.Pitch = CursorWorldRotation.Pitch;
	}

	if (GetDesignerSettings()->bSnapToGridRotationZ)
	{
		CursorWorldRotation.Yaw = SpawnRotationSnapped.Yaw;
	}
	else
	{
		CursorWorldRotation.Yaw = CursorWorldRotation.Yaw;
	}

	NewSpawnTransform.SetRotation(CursorWorldRotation.Quaternion());

	SpawnWorldTransform = NewSpawnTransform;

	return true;
}

void FSpawnAssetTool::RecalculateMousePlaneIntersectionWorldLocation(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		Viewport,
		ViewportClient->GetScene(),
		ViewportClient->EngineShowFlags)
		.SetRealtimeUpdate(ViewportClient->IsRealtime()));
	// SceneView is deleted with the ViewFamily
	FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily);
	FViewportCursorLocation MouseViewportRay(SceneView, ViewportClient, Viewport->GetMouseX(), Viewport->GetMouseY());

	FViewportCursorLocation ViewportCenterRay(SceneView, ViewportClient, Viewport->GetSizeXY().X * 0.5F, Viewport->GetSizeXY().Y * 0.5F);

	FVector TraceStartLocation = MouseViewportRay.GetOrigin();
	FVector TraceDirection = MouseViewportRay.GetDirection();
	FVector TraceEndLocation = TraceStartLocation + TraceDirection * WORLD_MAX;

	SpawnTracePlane = FPlane(SpawnWorldTransform.GetLocation(), SpawnWorldTransform.GetRotation().GetUpVector());
	CursorPlaneIntersectionWorldLocation = FMath::LinePlaneIntersection(TraceStartLocation, TraceEndLocation, SpawnTracePlane);
}

void FSpawnAssetTool::UpdateDesignerActorTransform()
{
	FTransform NewDesignerActorTransform = SpawnWorldTransform;

	FVector CursorDirection(0.F, 0.F, 0.F);
	float CursorDistance = 0.F;
	(CursorPlaneIntersectionWorldLocation - SpawnWorldTransform.GetLocation()).ToDirectionAndLength(CursorDirection, CursorDistance);

	FVector NewScale = FVector::OneVector;
	if (GetDesignerSettings()->bApplyRandomScale)
	{
		NewScale = GetRandomScale();

		// If the object also scales towards the mouse we use the randoms scale as a ratio
		if (GetDesignerSettings()->bScaleBoundsTowardsCursor)
		{
			NewScale /= FMath::Max(NewScale.X, FMath::Max(NewScale.Y, NewScale.Z));
		}
	}

	if (GetDesignerSettings()->bScaleBoundsTowardsCursor)
	{
		EAxisType PositiveAxis = GetDesignerSettings()->GetPositiveAxisToAlignWithCursor();
		float BoundsUsedForScale;
		if (PositiveAxis == EAxisType::Forward)
			BoundsUsedForScale = DefaultDesignerActorExtent.X;
		else if (PositiveAxis == EAxisType::Right)
			BoundsUsedForScale = DefaultDesignerActorExtent.Y;
		else if (PositiveAxis == EAxisType::Up)
			BoundsUsedForScale = DefaultDesignerActorExtent.Z;
		else
			BoundsUsedForScale = FMath::Max(DefaultDesignerActorExtent.X, DefaultDesignerActorExtent.Y);

		NewScale *= FVector(CursorDistance / BoundsUsedForScale);
	}

	if (NewScale.ContainsNaN())
	{
		NewScale = FVector::OneVector;
		UE_LOG(LogDesigner, Warning, TEXT("New scale contained NaN, so it is set to one. DefaultDesignerActorExtent = %s."), *DefaultDesignerActorExtent.ToString());
	}

	NewDesignerActorTransform.SetScale3D(NewScale);

	NewDesignerActorTransform.SetRotation(GetDesignerActorRotation().Quaternion());
	SpawnedActor->SetActorTransform(NewDesignerActorTransform);
	SpawnedActor->AddActorWorldOffset(GetDesignerSettings()->WorldLocationOffset);
	SpawnedActor->AddActorLocalOffset(GetDesignerSettings()->RelativeLocationOffset);
}
void FSpawnAssetTool::RegenerateRandomRotationOffset()
{
	GetDesignerSettings()->RandomRotationX.RegenerateRandomValue();
	GetDesignerSettings()->RandomRotationY.RegenerateRandomValue();
	GetDesignerSettings()->RandomRotationZ.RegenerateRandomValue();
}

FRotator FSpawnAssetTool::GetRandomRotationOffset() const
{
	return FRotator( // Pitch, Yaw, Roll = Y, Z, X.
		GetDesignerSettings()->RandomRotationY.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomRotationZ.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomRotationX.GetCurrentRandomValue()
	);
}

void FSpawnAssetTool::RegenerateRandomScale()
{
	GetDesignerSettings()->RandomScaleX.RegenerateRandomValue();
	GetDesignerSettings()->RandomScaleY.RegenerateRandomValue();
	GetDesignerSettings()->RandomScaleZ.RegenerateRandomValue();
}

FVector FSpawnAssetTool::GetRandomScale() const
{
	return FVector(
		GetDesignerSettings()->RandomScaleX.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomScaleY.GetCurrentRandomValue(),
		GetDesignerSettings()->RandomScaleZ.GetCurrentRandomValue()
	);
}

FRotator FSpawnAssetTool::GetDesignerActorRotation()
{
	FVector MouseDirection(0.F,0.F,0.F);
	float MouseDistance = 0.F;
	(CursorPlaneIntersectionWorldLocation - SpawnWorldTransform.GetLocation()).ToDirectionAndLength(MouseDirection, MouseDistance);

	// If the mouse is exactly at the CursorInputDownWorldTransform, which happens on mouse click down.
	if (MouseDirection.IsNearlyZero())
		MouseDirection = SpawnWorldTransform.GetRotation().GetForwardVector();

	FVector ForwardVector = MouseDirection;
	if (GetDesignerSettings()->AxisToAlignWithCursor == EAxisType::None)
	{
		ForwardVector = SpawnWorldTransform.GetRotation().GetForwardVector();
	}
	FVector UpVector = SpawnWorldTransform.GetRotation().GetUpVector();

	// if they're almost same, we need to find arbitrary vector
	if (FMath::IsNearlyEqual(FMath::Abs(ForwardVector | UpVector), 1.f))
	{
		// make sure we don't ever pick the same as NewX
		if (FMath::Abs(ForwardVector.Z) < (1.f - KINDA_SMALL_NUMBER))
		{
			UpVector = FVector(0, 0, 1.f);
		}
		else
		{
			UpVector = FVector(1.f, 0, 0);
		}
	}

	FVector RightVector = (UpVector ^ ForwardVector).GetSafeNormal();
	UpVector = ForwardVector ^ RightVector;

	FVector SwizzledForwardVector = FVector::ZeroVector;
	FVector SwizzledRightVector = FVector::ZeroVector;
	FVector SwizzledUpVector = FVector::ZeroVector;

	switch (GetDesignerSettings()->AxisToAlignWithNormal)
	{
	case EAxisType::Forward:
		SwizzledForwardVector = UpVector;
		break;
	case EAxisType::Backward:
		SwizzledForwardVector = -UpVector;
		break;
	case EAxisType::Right:
		SwizzledRightVector = UpVector;
		break;
	case EAxisType::Left:
		SwizzledRightVector = -UpVector;
		break;
	case EAxisType::Down:
		SwizzledUpVector = -UpVector;
		break;
	default: // Axis type none or up
		SwizzledUpVector = UpVector;
		break;
	}

	switch (GetDesignerSettings()->AxisToAlignWithCursor)
	{
	case EAxisType::Backward:
		SwizzledForwardVector = -ForwardVector;
		break;
	case EAxisType::Right:
		SwizzledRightVector = ForwardVector;
		break;
	case EAxisType::Left:
		SwizzledRightVector = -ForwardVector;
		break;
	case EAxisType::Up:
		SwizzledUpVector = ForwardVector;
		break;
	case EAxisType::Down:
		SwizzledUpVector = -ForwardVector;
		break;
	default: // Axis type none or forward
		SwizzledForwardVector = ForwardVector;
		break;
	}

	bool bIsForwardVectorSet = !SwizzledForwardVector.IsNearlyZero();
	bool bIsRightVectorSet = !SwizzledRightVector.IsNearlyZero();
	bool bIsUpVectorSet = !SwizzledUpVector.IsNearlyZero();

	FRotator DesignerActorRotation;

	if (!bIsForwardVectorSet && bIsRightVectorSet && bIsUpVectorSet)
	{
		DesignerActorRotation = FRotationMatrix::MakeFromZY(SwizzledUpVector, SwizzledRightVector).Rotator();
	}
	else if (!bIsRightVectorSet && bIsForwardVectorSet && bIsUpVectorSet)
	{
		DesignerActorRotation = FRotationMatrix::MakeFromZX(SwizzledUpVector, SwizzledForwardVector).Rotator();
	}
	else if (!bIsUpVectorSet && bIsForwardVectorSet && bIsRightVectorSet)
	{
		DesignerActorRotation = FRotationMatrix::MakeFromXY(SwizzledForwardVector, SwizzledRightVector).Rotator();
	}
	else
	{
		// Default rotation of everything else fails
		DesignerActorRotation = FMatrix(ForwardVector, RightVector, UpVector, FVector::ZeroVector).Rotator();
		UE_LOG(LogDesigner, Warning, TEXT("Falling back to default rotation."));
	}

	// Apply the generated random rotation offset if the user has set the bApplyRandomRotation setting
	if (GetDesignerSettings()->bApplyRandomRotation)
	{
		DesignerActorRotation = FRotator(DesignerActorRotation.Quaternion() * GetRandomRotationOffset().Quaternion());
	}

	// Snap the axes to the grid if the user has set bSnapToGridRotation
	FRotator SpawnRotationSnapped = DesignerActorRotation;
	FSnappingUtils::SnapRotatorToGrid(SpawnRotationSnapped);
	DesignerActorRotation.Roll = GetDesignerSettings()->bSnapToGridRotationX ? SpawnRotationSnapped.Roll : DesignerActorRotation.Roll;
	DesignerActorRotation.Pitch = GetDesignerSettings()->bSnapToGridRotationY ? SpawnRotationSnapped.Pitch : DesignerActorRotation.Pitch;
	DesignerActorRotation.Yaw = GetDesignerSettings()->bSnapToGridRotationZ ? SpawnRotationSnapped.Yaw : DesignerActorRotation.Yaw;

	return DesignerActorRotation;
}