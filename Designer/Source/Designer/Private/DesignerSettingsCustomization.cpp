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
#include "DesignerSettingsCustomization.h"

// Engine Includes
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "PropertyHandle.h"
#include "Misc/Optional.h"
#include "Fonts/SlateFontInfo.h"

#include "EditorModeManager.h"

// Local Includes
#include "DesignerModule.h"

#include "DesignerEdMode.h"
#include "DesignerSettings.h"


TSharedRef<IDetailCustomization> FDesignerSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FDesignerSettingsCustomization());
}

void FDesignerSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{	
	FDesignerEdMode* DesignerEdMode = static_cast<FDesignerEdMode*>(GLevelEditorModeTools().GetActiveMode(FDesignerEdMode::EM_DesignerEdModeId));

	IDetailCategoryBuilder& SpawnSettingsCategory = DetailBuilder.EditCategory("SpawnSettings");

	TSharedRef<IPropertyHandle> PropertyHandle_SpawnLocationOffsetRelative = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, RelativeLocationOffset));
	TSharedRef<IPropertyHandle> PropertyHandle_SpawnLocationOffsetRelative_X = PropertyHandle_SpawnLocationOffsetRelative->GetChildHandle("X").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_SpawnLocationOffsetRelative_Y = PropertyHandle_SpawnLocationOffsetRelative->GetChildHandle("Y").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_SpawnLocationOffsetRelative_Z = PropertyHandle_SpawnLocationOffsetRelative->GetChildHandle("Z").ToSharedRef();
	
	SpawnSettingsCategory.AddProperty(PropertyHandle_SpawnLocationOffsetRelative)
	.CustomWidget()
	.NameContent()
	[
		PropertyHandle_SpawnLocationOffsetRelative->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(125.0f * 3.0f) // copied from FComponentTransformDetails
	.MaxDesiredWidth(125.0f * 3.0f)
	[
		SNew(SVectorInputBox)
		.bColorAxisLabels(true)
		.AllowSpin(false)
		.Font(DetailBuilder.GetDetailFont())
		.X_Lambda([=]() -> float { return DesignerEdMode->GetDesignerSettings()->RelativeLocationOffset.X; })
		.Y_Lambda([=]() -> float { return DesignerEdMode->GetDesignerSettings()->RelativeLocationOffset.Y; })
		.Z_Lambda([=]() -> float { return DesignerEdMode->GetDesignerSettings()->RelativeLocationOffset.Z; })
		.OnXChanged_Lambda([=](float Value) { DesignerEdMode->GetDesignerSettings()->RelativeLocationOffset.X = Value; })
		.OnYChanged_Lambda([=](float Value) { DesignerEdMode->GetDesignerSettings()->RelativeLocationOffset.Y = Value; })
		.OnZChanged_Lambda([=](float Value) { DesignerEdMode->GetDesignerSettings()->RelativeLocationOffset.Z = Value; })
	];

	TSharedRef<IPropertyHandle> PropertyHandle_SpawnLocationOffsetWorld = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDesignerSettings, WorldLocationOffset));
	TSharedRef<IPropertyHandle> PropertyHandle_SpawnLocationOffsetWorld_X = PropertyHandle_SpawnLocationOffsetWorld->GetChildHandle("X").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_SpawnLocationOffsetWorld_Y = PropertyHandle_SpawnLocationOffsetWorld->GetChildHandle("Y").ToSharedRef();
	TSharedRef<IPropertyHandle> PropertyHandle_SpawnLocationOffsetWorld_Z = PropertyHandle_SpawnLocationOffsetWorld->GetChildHandle("Z").ToSharedRef();

	SpawnSettingsCategory.AddProperty(PropertyHandle_SpawnLocationOffsetWorld)
		.CustomWidget()
		.NameContent()
		[
			PropertyHandle_SpawnLocationOffsetWorld->CreatePropertyNameWidget()
		]
	.ValueContent()
		.MinDesiredWidth(125.0f * 3.0f) // copied from FComponentTransformDetails
		.MaxDesiredWidth(125.0f * 3.0f)
		[
			SNew(SVectorInputBox)
			.bColorAxisLabels(true)
			.AllowSpin(false)
			.Font(DetailBuilder.GetDetailFont())
			.X_Lambda([=]() -> float { return DesignerEdMode->GetDesignerSettings()->WorldLocationOffset.X; })
			.Y_Lambda([=]() -> float { return DesignerEdMode->GetDesignerSettings()->WorldLocationOffset.Y; })
			.Z_Lambda([=]() -> float { return DesignerEdMode->GetDesignerSettings()->WorldLocationOffset.Z; })
			.OnXChanged_Lambda([=](float Value) { DesignerEdMode->GetDesignerSettings()->WorldLocationOffset.X = Value; })
			.OnYChanged_Lambda([=](float Value) { DesignerEdMode->GetDesignerSettings()->WorldLocationOffset.Y = Value; })
			.OnZChanged_Lambda([=](float Value) { DesignerEdMode->GetDesignerSettings()->WorldLocationOffset.Z = Value; })
		];
}

void FDesignerSettingsCustomization::OnPaintTypeChanged(IDetailLayoutBuilder* LayoutBuilder)
{
	LayoutBuilder->ForceRefreshDetails();
}

TSharedPtr<SWidget> FDesignerSettingsRootObjectCustomization::CustomizeObjectHeader(const UObject* InRootObject)
{
	return SNullWidget::NullWidget;
}

bool FDesignerSettingsRootObjectCustomization::IsObjectVisible(const UObject* InRootObject) const
{ 
	return true; 
}

bool FDesignerSettingsRootObjectCustomization::ShouldDisplayHeader(const UObject* InRootObject) const
{ 
	return false; 
}