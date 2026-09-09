// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IPropertyTypeCustomization.h"

class IPropertyHandle;
class UAnimMontage;
template <typename OptionType>
class SComboBox;

class FHACF_ComboNodeCustomization final : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	void RefreshSectionNames();
	void RefreshAndValidateSection();
	
	void ValidateCurrentSections();
	
	void HandleMontageChanged();
	void HandleSectionComboBoxOpening();
	void HandleSectionSelected(TSharedPtr<FName> SelectedItem, ESelectInfo::Type SelectInfo);
	
	FText GetCurrentSectionText() const;
	
	bool CanEditSection() const;
	bool IsSectionNameValid(FName SectionName) const;
	
	bool TryGetSingleMontage(UAnimMontage*& OutMontage) const;

	TSharedPtr<IPropertyHandle> MontageHandle;
	TSharedPtr<IPropertyHandle> StartSectionHandle;
	TArray<TSharedPtr<FName>> SectionNames;
	
	TSharedPtr<SComboBox<TSharedPtr<FName>>> SectionComboBox;
	
	bool bHasSingleMontageValue = false;
	bool bHasMontage = false;
};
