// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Combo/HACF_ComboDefinition.h"

const FHACF_ComboNode* UHACF_ComboDefinition::FindNode(FName NodeId) const
{
	if (NodeId.IsNone())
	{
		return nullptr;
	}

	return Nodes.FindByPredicate(
		[NodeId](const FHACF_ComboNode& Node)
		{
			return Node.NodeId == NodeId;
		});
}

const FHACF_ComboNode* UHACF_ComboDefinition::GetEntryNode() const
{
	return FindNode(EntryNodeId);
}
