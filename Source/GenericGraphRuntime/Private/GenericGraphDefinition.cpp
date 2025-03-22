#include "Engine/Engine.h"
#include "GenericGraphDefinition.h"
#include "GenericGraphRuntimePCH.h"

#define LOCTEXT_NAMESPACE "GenericGraph"

UGenericGraphDefinition::UGenericGraphDefinition(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.Get())
{
	NodeType = UGenericGraphNode::StaticClass();
	EdgeType = UGenericGraphEdge::StaticClass();

	bEdgeEnabled = true;

	#if WITH_EDITORONLY_DATA
	EditorGraph = nullptr;

	bCanRenameNode = true;
	#endif
}

UGenericGraphDefinition::~UGenericGraphDefinition() {}

void UGenericGraphDefinition::Print(bool ToConsole /*= true*/, bool ToScreen /*= true*/)
{
	int Level = 0;
	TArray<UGenericGraphNode*> CurrLevelNodes = RootNodes;
	TArray<UGenericGraphNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UGenericGraphNode* Node = CurrLevelNodes[i];
			check(Node != nullptr);

			FString Message = FString::Printf(TEXT("%s, Level %d"), *Node->GetDescription().ToString(), Level);

			if (ToConsole)
			{
				LOG_INFO(TEXT("%s"), *Message);
			}

			if (ToScreen && GEngine != nullptr)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, Message);
			}

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}
}

int UGenericGraphDefinition::GetLevelNum() const
{
	int Level = 0;
	TArray<UGenericGraphNode*> CurrLevelNodes = RootNodes;
	TArray<UGenericGraphNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UGenericGraphNode* Node = CurrLevelNodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}

	return Level;
}

void UGenericGraphDefinition::GetNodesByLevel(int Level, TArray<UGenericGraphNode*>& Nodes)
{
	int CurrLEvel = 0;
	TArray<UGenericGraphNode*> NextLevelNodes;

	Nodes = RootNodes;

	while (Nodes.Num() != 0)
	{
		if (CurrLEvel == Level)
		{
			break;
		}

		for (int i = 0; i < Nodes.Num(); ++i)
		{
			UGenericGraphNode* Node = Nodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		Nodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++CurrLEvel;
	}
}

void UGenericGraphDefinition::ClearGraph()
{
	for (int i = 0; i < AllNodes.Num(); ++i)
	{
		UGenericGraphNode* Node = AllNodes[i];
		if (Node)
		{
			Node->ParentNodes.Empty();
			Node->ChildrenNodes.Empty();
			Node->Edges.Empty();
		}
	}

	AllNodes.Empty();
	RootNodes.Empty();
}

#undef LOCTEXT_NAMESPACE
