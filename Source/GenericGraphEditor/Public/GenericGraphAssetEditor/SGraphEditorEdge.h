#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "SNodePanel.h"
#include "Styling/SlateColor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"

class SToolTip;
class UGraphEditorEdEdgeNodeBase;

class GENERICGRAPHEDITOR_API SGraphEditorEdge : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphEditorEdge) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UGraphEditorEdEdgeNodeBase* InNode);

	virtual bool RequiresSecondPassLayout() const override;
	virtual void PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& NodeToWidgetLookup) const override;

	virtual void UpdateGraphNode() override;

	// Calculate position for multiple nodes to be placed between a start and end
	// point, by providing this nodes index and max expected nodes
	void PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const;

	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);

protected:
	FSlateColor GetEdgeColor() const;

	const FSlateBrush* GetEdgeImage() const;

	EVisibility GetEdgeImageVisibility() const;
	EVisibility GetEdgeTitleVisbility() const;

private:
	TSharedPtr<STextEntryPopup> TextEntryWidget;
};
