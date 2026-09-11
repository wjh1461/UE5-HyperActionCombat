// Copyright Epic Games, Inc. All Rights Reserved.

#include "Customizations/HACF_ComboNodeCustomization.h"

#include "Animation/AnimMontage.h"
#include "Combat/Combo/HACF_ComboTypes.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "UObject/UnrealType.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "HACF_ComboNodeCustomization"

TSharedRef<IPropertyTypeCustomization> FHACF_ComboNodeCustomization::MakeInstance()
{
	return MakeShared<FHACF_ComboNodeCustomization>();
}

void FHACF_ComboNodeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const TSharedPtr<IPropertyHandle> NodeIdHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHACF_ComboNode, NodeId));

	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(130.0f)
	[
		SNew(STextBlock)
		.Text_Lambda([NodeIdHandle]()
		{
			if (!NodeIdHandle.IsValid())
			{
				return FText::GetEmpty();
			}

			FName NodeId = NAME_None;
			const FPropertyAccess::Result Result = NodeIdHandle->GetValue(NodeId);
			if (Result == FPropertyAccess::MultipleValues)
			{
				return LOCTEXT("MultipleNodeIds", "Multiple Values");
			}

			return Result == FPropertyAccess::Success ? FText::FromName(NodeId) : FText::GetEmpty();
		})
	];
}

void FHACF_ComboNodeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	MontageHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHACF_ComboNode, Montage));
	StartSectionHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHACF_ComboNode, StartSection));

	if (MontageHandle.IsValid())
	{
		MontageHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FHACF_ComboNodeCustomization::HandleMontageChanged));
	}

	RefreshSectionNames();

	uint32 NumChildren = 0;
	StructPropertyHandle->GetNumChildren(NumChildren);
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex);
		if (!ChildHandle.IsValid())
		{
			continue;
		}

		if (!StartSectionHandle.IsValid()
			|| ChildHandle->GetProperty() != StartSectionHandle->GetProperty())
		{
			StructBuilder.AddProperty(ChildHandle.ToSharedRef());
			continue;
		}

		IDetailPropertyRow& StartSectionRow = StructBuilder.AddProperty(ChildHandle.ToSharedRef());
		StartSectionRow.CustomWidget()
		.NameContent()
		[
			StartSectionHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(220.0f)
		[
			SAssignNew(SectionComboBox, SComboBox<TSharedPtr<FName>>)
			.OptionsSource(&SectionNames)
			.IsEnabled(this, &FHACF_ComboNodeCustomization::CanEditSection)
			.OnComboBoxOpening(this, &FHACF_ComboNodeCustomization::HandleSectionComboBoxOpening)
			.OnSelectionChanged(this, &FHACF_ComboNodeCustomization::HandleSectionSelected)
			.OnGenerateWidget_Lambda([](const TSharedPtr<FName>& Item)
			{
				return SNew(STextBlock)
					.Text(Item.IsValid() ? FText::FromName(*Item) : FText::GetEmpty());
			})
			[
				SNew(STextBlock)
				.Text(this, &FHACF_ComboNodeCustomization::GetCurrentSectionText)
			]
		];
	}
}

void FHACF_ComboNodeCustomization::RefreshSectionNames()
{
	SectionNames.Reset();
	SectionNames.Add(MakeShared<FName>(NAME_None));
	
	bHasSingleMontageValue = false;
	bHasMontage = false;

	UAnimMontage* Montage = nullptr;
	if (TryGetSingleMontage(Montage))
	{
		bHasSingleMontageValue = true;
		if (Montage)
		{
			bHasMontage = true;
			for (int32 SectionIndex = 0; SectionIndex < Montage->GetNumSections(); ++SectionIndex)
			{
				const FName SectionName = Montage->GetSectionName(SectionIndex);
				if (!SectionName.IsNone())
				{
					SectionNames.Add(MakeShared<FName>(SectionName));
				}
			}
		}
	}

	if (SectionComboBox.IsValid())
	{
		SectionComboBox->RefreshOptions();
	}
}

void FHACF_ComboNodeCustomization::RefreshAndValidateSection()
{
	RefreshSectionNames();
	ValidateCurrentSections();
}

void FHACF_ComboNodeCustomization::ValidateCurrentSections()
{
	if (!bHasSingleMontageValue || !StartSectionHandle.IsValid())
	{
		return;
	}

	TArray<FString> PerObjectValues;
	if (StartSectionHandle->GetPerObjectValues(PerObjectValues) != FPropertyAccess::Success)
	{
		return;
	}

	bool bHasInvalidSection = false;
	for (FString& PerObjectValue : PerObjectValues)
	{
		const FName SectionName(*PerObjectValue.TrimQuotes());
		if (!IsSectionNameValid(SectionName))
		{
			PerObjectValue = FName(NAME_None).ToString();
			bHasInvalidSection = true;
		}
	}

	if (bHasInvalidSection)
	{
		// PropertyHandle 경로를 사용해 다중 객체의 유효한 값은 보존하면서
		// 변경 알림과 transaction/undo를 유지한다.
		StartSectionHandle->SetPerObjectValues(PerObjectValues);
	}
}

void FHACF_ComboNodeCustomization::HandleMontageChanged()
{
	RefreshAndValidateSection();
}

void FHACF_ComboNodeCustomization::HandleSectionComboBoxOpening()
{
	// Montage 에셋이 외부 에디터에서 수정됐을 수 있으므로 열 때마다 다시 조회한다.
	RefreshAndValidateSection();
}

void FHACF_ComboNodeCustomization::HandleSectionSelected(TSharedPtr<FName> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (bHasSingleMontageValue && StartSectionHandle.IsValid() && SelectedItem.IsValid())
	{
		StartSectionHandle->SetValue(*SelectedItem);
	}
}

FText FHACF_ComboNodeCustomization::GetCurrentSectionText() const
{
	if (!StartSectionHandle.IsValid())
	{
		return FText::GetEmpty();
	}

	FName CurrentSection = NAME_None;
	const FPropertyAccess::Result Result = StartSectionHandle->GetValue(CurrentSection);
	if (Result == FPropertyAccess::MultipleValues)
	{
		return LOCTEXT("MultipleValues", "Multiple Values");
	}

	return Result == FPropertyAccess::Success ? FText::FromName(CurrentSection) : FText::GetEmpty();
}

bool FHACF_ComboNodeCustomization::CanEditSection() const
{
	return bHasSingleMontageValue && bHasMontage
		&& StartSectionHandle.IsValid() && StartSectionHandle->IsEditable();
}

bool FHACF_ComboNodeCustomization::IsSectionNameValid(FName SectionName) const
{
	return SectionName.IsNone() || SectionNames.ContainsByPredicate(
		[SectionName](const TSharedPtr<FName>& AvailableSectionName)
		{
			return AvailableSectionName.IsValid() && *AvailableSectionName == SectionName;
		});
}

bool FHACF_ComboNodeCustomization::TryGetSingleMontage(UAnimMontage*& OutMontage) const
{
	OutMontage = nullptr;

	if (!MontageHandle.IsValid())
	{
		return false;
	}

	const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(MontageHandle->GetProperty());
	if (!ObjectProperty)
	{
		return false;
	}

	TArray<void*> RawData;
	MontageHandle->AccessRawData(RawData);
	if (RawData.IsEmpty() || !RawData[0])
	{
		return false;
	}

	UObject* FirstValue = ObjectProperty->GetObjectPropertyValue(RawData[0]);
	for (int32 Index = 1; Index < RawData.Num(); ++Index)
	{
		if (!RawData[Index] || ObjectProperty->GetObjectPropertyValue(RawData[Index]) != FirstValue)
		{
			return false;
		}
	}

	OutMontage = Cast<UAnimMontage>(FirstValue);
	return FirstValue == nullptr || OutMontage != nullptr;
}

#undef LOCTEXT_NAMESPACE
