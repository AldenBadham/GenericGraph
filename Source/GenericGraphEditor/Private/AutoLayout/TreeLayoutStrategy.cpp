#include "AutoLayout/TreeLayoutStrategy.h"
#include "GenericGraphAssetEditor/SGraphEditorNode.h"
#include "GenericGraphEditorPCH.h"

UTreeLayoutStrategy::UTreeLayoutStrategy() {}

UTreeLayoutStrategy::~UTreeLayoutStrategy() {}

void UTreeLayoutStrategy::Layout(UEdGraph* _EdGraph)
{
	EdGraph = Cast<UEdGraph_GenericGraph>(_EdGraph);
	check(EdGraph != nullptr);

	EdGraph->RebuildGenericGraph();
	Graph = EdGraph->GetGenericGraph();
	check(Graph != nullptr);

	bool bFirstPassOnly = false;

	if (Settings != nullptr)
	{
		OptimalDistance = Settings->OptimalDistance;
		MaxIteration = Settings->MaxIteration;
		bFirstPassOnly = Settings->bFirstPassOnly;
	}

	FVector2D Anchor(0.f, 0.f);
	for (int32 i = 0; i < Graph->RootNodes.Num(); ++i)
	{
		UGraphNodeDefinitionBase* RootNode = Graph->RootNodes[i];
		InitPass(RootNode, Anchor);

		if (!bFirstPassOnly)
		{
			for (int32 j = 0; j < MaxIteration; ++j)
			{
				bool HasConflict = ResolveConflictPass(RootNode);
				if (!HasConflict)
				{
					break;
				}
			}
		}
	}

	for (int32 i = 0; i < Graph->RootNodes.Num(); ++i)
	{
		for (int32 j = 0; j < i; ++j)
		{
			ResolveConflict(Graph->RootNodes[j], Graph->RootNodes[i]);
		}
	}
}

void UTreeLayoutStrategy::InitPass(UGraphNodeDefinitionBase* RootNode, const FVector2D& Anchor)
{
	UGraphEditorEdNodeBase* EdNode_RootNode = EdGraph->NodeMap[RootNode];

	FVector2D ChildAnchor(FVector2D(0.f, GetNodeHeight(EdNode_RootNode) + OptimalDistance + Anchor.Y));
	for (int32 i = 0; i < RootNode->ChildrenNodes.Num(); ++i)
	{
		UGraphNodeDefinitionBase* Child = RootNode->ChildrenNodes[i];
		UGraphEditorEdNodeBase* EdNode_ChildNode = EdGraph->NodeMap[Child];
		if (i > 0)
		{
			UGraphNodeDefinitionBase* PreChild = RootNode->ChildrenNodes[i - 1];
			UGraphEditorEdNodeBase* EdNode_PreChildNode = EdGraph->NodeMap[PreChild];
			ChildAnchor.X += OptimalDistance + GetNodeWidth(EdNode_PreChildNode) / 2;
		}
		ChildAnchor.X += GetNodeWidth(EdNode_ChildNode) / 2;
		InitPass(Child, ChildAnchor);
	}

	float NodeWidth = GetNodeWidth(EdNode_RootNode);

	EdNode_RootNode->NodePosY = Anchor.Y;
	if (RootNode->ChildrenNodes.Num() == 0)
	{
		EdNode_RootNode->NodePosX = Anchor.X - NodeWidth / 2;
	}
	else
	{
		UpdateParentNodePosition(RootNode);
	}
}

bool UTreeLayoutStrategy::ResolveConflictPass(UGraphNodeDefinitionBase* Node)
{
	bool HasConflict = false;
	for (int32 i = 0; i < Node->ChildrenNodes.Num(); ++i)
	{
		UGraphNodeDefinitionBase* Child = Node->ChildrenNodes[i];
		if (ResolveConflictPass(Child))
		{
			HasConflict = true;
		}
	}

	for (int32 i = 0; i < Node->ParentNodes.Num(); ++i)
	{
		UGraphNodeDefinitionBase* ParentNode = Node->ParentNodes[i];
		for (int32 j = 0; j < ParentNode->ChildrenNodes.Num(); ++j)
		{
			UGraphNodeDefinitionBase* LeftSibling = ParentNode->ChildrenNodes[j];
			if (LeftSibling == Node)
			{
				break;
			}
			if (ResolveConflict(LeftSibling, Node))
			{
				HasConflict = true;
			}
		}
	}

	return HasConflict;
}

bool UTreeLayoutStrategy::ResolveConflict(UGraphNodeDefinitionBase* LRoot, UGraphNodeDefinitionBase* RRoot)
{
	TArray<UGraphEditorEdNodeBase*> RightContour, LeftContour;

	GetRightContour(LRoot, 0, RightContour);
	GetLeftContour(RRoot, 0, LeftContour);

	int32 MaxOverlapDistance = 0;
	int32 Num = FMath::Min(LeftContour.Num(), RightContour.Num());
	for (int32 i = 0; i < Num; ++i)
	{
		if (RightContour.Contains(LeftContour[i]) || LeftContour.Contains(RightContour[i]))
		{
			break;
		}

		int32 RightBound = RightContour[i]->NodePosX + GetNodeWidth(RightContour[i]);
		int32 LeftBound = LeftContour[i]->NodePosX;
		int32 Distance = RightBound + OptimalDistance - LeftBound;
		if (Distance > MaxOverlapDistance)
		{
			MaxOverlapDistance = Distance;
		}
	}

	if (MaxOverlapDistance > 0)
	{
		ShiftSubTree(RRoot, FVector2D(MaxOverlapDistance, 0.f));

		TArray<UGraphNodeDefinitionBase*> ParentNodes = RRoot->ParentNodes;
		TArray<UGraphNodeDefinitionBase*> NextParentNodes;
		while (ParentNodes.Num() != 0)
		{
			for (int32 i = 0; i < ParentNodes.Num(); ++i)
			{
				UpdateParentNodePosition(ParentNodes[i]);

				NextParentNodes.Append(ParentNodes[i]->ParentNodes);
			}

			ParentNodes = NextParentNodes;
			NextParentNodes.Reset();
		}

		return true;
	}
	return false;
}

void UTreeLayoutStrategy::GetLeftContour(UGraphNodeDefinitionBase* RootNode, int32 Level, TArray<UGraphEditorEdNodeBase*>& Contour)
{
	UGraphEditorEdNodeBase* EdNode_Node = EdGraph->NodeMap[RootNode];
	if (Level >= Contour.Num())
	{
		Contour.Add(EdNode_Node);
	}
	else if (EdNode_Node->NodePosX < Contour[Level]->NodePosX)
	{
		Contour[Level] = EdNode_Node;
	}

	for (int32 i = 0; i < RootNode->ChildrenNodes.Num(); ++i)
	{
		GetLeftContour(RootNode->ChildrenNodes[i], Level + 1, Contour);
	}
}

void UTreeLayoutStrategy::GetRightContour(UGraphNodeDefinitionBase* RootNode, int32 Level, TArray<UGraphEditorEdNodeBase*>& Contour)
{
	UGraphEditorEdNodeBase* EdNode_Node = EdGraph->NodeMap[RootNode];
	if (Level >= Contour.Num())
	{
		Contour.Add(EdNode_Node);
	}
	else if (EdNode_Node->NodePosX + GetNodeWidth(EdNode_Node) > Contour[Level]->NodePosX + GetNodeWidth(Contour[Level]))
	{
		Contour[Level] = EdNode_Node;
	}

	for (int32 i = 0; i < RootNode->ChildrenNodes.Num(); ++i)
	{
		GetRightContour(RootNode->ChildrenNodes[i], Level + 1, Contour);
	}
}

void UTreeLayoutStrategy::ShiftSubTree(UGraphNodeDefinitionBase* RootNode, const FVector2D& Offset)
{
	UGraphEditorEdNodeBase* EdNode_Node = EdGraph->NodeMap[RootNode];
	EdNode_Node->NodePosX += Offset.X;
	EdNode_Node->NodePosY += Offset.Y;

	for (int32 i = 0; i < RootNode->ChildrenNodes.Num(); ++i)
	{
		UGraphNodeDefinitionBase* Child = RootNode->ChildrenNodes[i];

		if (Child->ParentNodes[0] == RootNode)
		{
			ShiftSubTree(RootNode->ChildrenNodes[i], Offset);
		}
	}
}

void UTreeLayoutStrategy::UpdateParentNodePosition(UGraphNodeDefinitionBase* ParentNode)
{
	UGraphEditorEdNodeBase* EdNode_ParentNode = EdGraph->NodeMap[ParentNode];
	if (ParentNode->ChildrenNodes.Num() % 2 == 0)
	{
		UGraphEditorEdNodeBase* FirstChild = EdGraph->NodeMap[ParentNode->ChildrenNodes[0]];
		UGraphEditorEdNodeBase* LastChild = EdGraph->NodeMap[ParentNode->ChildrenNodes.Last()];
		float LeftBound = FirstChild->NodePosX;
		float RightBound = LastChild->NodePosX + GetNodeWidth(LastChild);
		EdNode_ParentNode->NodePosX = (LeftBound + RightBound) / 2 - GetNodeWidth(EdNode_ParentNode) / 2;
	}
	else
	{
		UGraphEditorEdNodeBase* MidChild = EdGraph->NodeMap[ParentNode->ChildrenNodes[ParentNode->ChildrenNodes.Num() / 2]];
		EdNode_ParentNode->NodePosX = MidChild->NodePosX + GetNodeWidth(MidChild) / 2 - GetNodeWidth(EdNode_ParentNode) / 2;
	}
}
