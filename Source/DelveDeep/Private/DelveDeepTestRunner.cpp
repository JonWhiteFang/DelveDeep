// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestRunner.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "Misc/AutomationTest.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepTestRunner, Log, All);

void SDelveDeepTestRunner::Construct(const FArguments& InArgs)
{
	bIsRunning = false;
	Progress = 0.0f;
	StatusText = FText::FromString(TEXT("Ready"));

	// Build initial test tree
	BuildTestTree();

	ChildSlot
	[
		SNew(SVerticalBox)

		// Toolbar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SHorizontalBox)

			// Run button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Run Selected")))
				.OnClicked(this, &SDelveDeepTestRunner::OnRunClicked)
				.IsEnabled_Lambda([this]() { return !bIsRunning && SelectedItems.Num() > 0; })
			]

			// Stop button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Stop")))
				.OnClicked(this, &SDelveDeepTestRunner::OnStopClicked)
				.IsEnabled_Lambda([this]() { return bIsRunning; })
			]

			// Refresh button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Refresh")))
				.OnClicked(this, &SDelveDeepTestRunner::OnRefreshClicked)
				.IsEnabled_Lambda([this]() { return !bIsRunning; })
			]

			// Spacer
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNullWidget::NullWidget
			]

			// Search box
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SBox)
				.WidthOverride(200.0f)
				[
					SNew(SSearchBox)
					.OnTextChanged(this, &SDelveDeepTestRunner::OnFilterTextChanged)
				]
			]
		]

		// Status bar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f))
			.Padding(4.0f)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return StatusText; })
			]
		]

		// Main content area
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(4.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			// Test tree view
			+ SSplitter::Slot()
			.Value(0.6f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f))
				.Padding(4.0f)
				[
					SAssignNew(TestTreeView, STreeView<TSharedPtr<FDelveDeepTestTreeItem>>)
					.TreeItemsSource(&RootItems)
					.OnGenerateRow(this, &SDelveDeepTestRunner::OnGenerateRow)
					.OnGetChildren(this, &SDelveDeepTestRunner::OnGetChildren)
					.OnSelectionChanged(this, &SDelveDeepTestRunner::OnSelectionChanged)
					.SelectionMode(ESelectionMode::Multi)
				]
			]

			// Details panel
			+ SSplitter::Slot()
			.Value(0.4f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f))
				.Padding(4.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Test Details\n\nSelect a test to view details.")))
				]
			]
		]

		// Progress bar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f))
			.Padding(4.0f)
			[
				SNew(SBox)
				.HeightOverride(20.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.0f, 0.5f, 0.0f))
					.Padding(0.0f)
					[
						SNew(SBox)
						.WidthOverride_Lambda([this]() { return Progress * 400.0f; })
					]
				]
			]
		]
	];
}

void SDelveDeepTestRunner::RefreshTestList()
{
	BuildTestTree();
	
	if (TestTreeView.IsValid())
	{
		TestTreeView->RequestTreeRefresh();
	}

	StatusText = FText::FromString(FString::Printf(TEXT("Refreshed. Found %d tests."), AllItems.Num()));
}

void SDelveDeepTestRunner::RunSelectedTests()
{
	if (SelectedItems.Num() == 0)
	{
		StatusText = FText::FromString(TEXT("No tests selected"));
		return;
	}

	TArray<FString> TestNames;
	for (const TSharedPtr<FDelveDeepTestTreeItem>& Item : SelectedItems)
	{
		if (Item->IsTest())
		{
			TestNames.Add(Item->Name);
		}
	}

	ExecuteTests(TestNames);
}

void SDelveDeepTestRunner::RunAllTests()
{
	TArray<FString> TestNames;
	for (const TSharedPtr<FDelveDeepTestTreeItem>& Item : AllItems)
	{
		if (Item->IsTest())
		{
			TestNames.Add(Item->Name);
		}
	}

	ExecuteTests(TestNames);
}

void SDelveDeepTestRunner::StopTests()
{
	bIsRunning = false;
	StatusText = FText::FromString(TEXT("Tests stopped"));
	UE_LOG(LogDelveDeepTestRunner, Display, TEXT("Test execution stopped by user"));
}

void SDelveDeepTestRunner::FilterTests(const FString& InFilterText)
{
	FilterText = InFilterText;

	// Rebuild tree with filter
	BuildTestTree();

	if (TestTreeView.IsValid())
	{
		TestTreeView->RequestTreeRefresh();
	}
}

TSharedRef<ITableRow> SDelveDeepTestRunner::OnGenerateRow(
	TSharedPtr<FDelveDeepTestTreeItem> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SDelveDeepTestRunnerRow, OwnerTable, Item);
}

void SDelveDeepTestRunner::OnGetChildren(
	TSharedPtr<FDelveDeepTestTreeItem> Item,
	TArray<TSharedPtr<FDelveDeepTestTreeItem>>& OutChildren)
{
	if (Item.IsValid())
	{
		OutChildren = Item->Children;
	}
}

void SDelveDeepTestRunner::OnSelectionChanged(
	TSharedPtr<FDelveDeepTestTreeItem> Item,
	ESelectInfo::Type SelectInfo)
{
	if (TestTreeView.IsValid())
	{
		SelectedItems = TestTreeView->GetSelectedItems();
	}
}

FReply SDelveDeepTestRunner::OnRunClicked()
{
	RunSelectedTests();
	return FReply::Handled();
}

FReply SDelveDeepTestRunner::OnStopClicked()
{
	StopTests();
	return FReply::Handled();
}

FReply SDelveDeepTestRunner::OnRefreshClicked()
{
	RefreshTestList();
	return FReply::Handled();
}

void SDelveDeepTestRunner::OnFilterTextChanged(const FText& InFilterText)
{
	FilterTests(InFilterText.ToString());
}

void SDelveDeepTestRunner::BuildTestTree()
{
	RootItems.Empty();
	AllItems.Empty();

	// Create mock test tree for demonstration
	// In a real implementation, this would query FAutomationTestFramework

	// Configuration tests
	TSharedPtr<FDelveDeepTestTreeItem> ConfigSuite = MakeShared<FDelveDeepTestTreeItem>(TEXT("Configuration"), false);
	ConfigSuite->AddChild(MakeShared<FDelveDeepTestTreeItem>(TEXT("DelveDeep.Configuration.CacheHitRate"), true));
	ConfigSuite->AddChild(MakeShared<FDelveDeepTestTreeItem>(TEXT("DelveDeep.Configuration.QueryPerformance"), true));
	ConfigSuite->AddChild(MakeShared<FDelveDeepTestTreeItem>(TEXT("DelveDeep.Configuration.Validation"), true));
	RootItems.Add(ConfigSuite);

	// Event tests
	TSharedPtr<FDelveDeepTestTreeItem> EventSuite = MakeShared<FDelveDeepTestTreeItem>(TEXT("Events"), false);
	EventSuite->AddChild(MakeShared<FDelveDeepTestTreeItem>(TEXT("DelveDeep.Events.Filtering"), true));
	EventSuite->AddChild(MakeShared<FDelveDeepTestTreeItem>(TEXT("DelveDeep.Events.Priority"), true));
	EventSuite->AddChild(MakeShared<FDelveDeepTestTreeItem>(TEXT("DelveDeep.Events.Performance"), true));
	RootItems.Add(EventSuite);

	// Telemetry tests
	TSharedPtr<FDelveDeepTestTreeItem> TelemetrySuite = MakeShared<FDelveDeepTestTreeItem>(TEXT("Telemetry"), false);
	TelemetrySuite->AddChild(MakeShared<FDelveDeepTestTreeItem>(TEXT("DelveDeep.Telemetry.FrameTracking"), true));
	TelemetrySuite->AddChild(MakeShared<FDelveDeepTestTreeItem>(TEXT("DelveDeep.Telemetry.BudgetTracking"), true));
	RootItems.Add(TelemetrySuite);

	// Collect all items
	for (const TSharedPtr<FDelveDeepTestTreeItem>& Suite : RootItems)
	{
		AllItems.Add(Suite);
		for (const TSharedPtr<FDelveDeepTestTreeItem>& Test : Suite->Children)
		{
			AllItems.Add(Test);
		}
	}

	// Apply filter if set
	if (!FilterText.IsEmpty())
	{
		TArray<TSharedPtr<FDelveDeepTestTreeItem>> FilteredRoots;
		
		for (const TSharedPtr<FDelveDeepTestTreeItem>& Suite : RootItems)
		{
			TArray<TSharedPtr<FDelveDeepTestTreeItem>> FilteredChildren;
			
			for (const TSharedPtr<FDelveDeepTestTreeItem>& Test : Suite->Children)
			{
				if (Test->Name.Contains(FilterText))
				{
					FilteredChildren.Add(Test);
				}
			}

			if (FilteredChildren.Num() > 0)
			{
				TSharedPtr<FDelveDeepTestTreeItem> FilteredSuite = MakeShared<FDelveDeepTestTreeItem>(Suite->Name, false);
				FilteredSuite->Children = FilteredChildren;
				FilteredRoots.Add(FilteredSuite);
			}
		}

		RootItems = FilteredRoots;
	}
}

void SDelveDeepTestRunner::ExecuteTests(const TArray<FString>& TestNames)
{
	if (TestNames.Num() == 0)
	{
		return;
	}

	bIsRunning = true;
	Progress = 0.0f;
	StatusText = FText::FromString(FString::Printf(TEXT("Running %d tests..."), TestNames.Num()));

	UE_LOG(LogDelveDeepTestRunner, Display, TEXT("Executing %d tests"), TestNames.Num());

	// In a real implementation, this would integrate with FAutomationTestFramework
	// For now, simulate test execution
	
	// Mark tests as running
	for (const TSharedPtr<FDelveDeepTestTreeItem>& Item : AllItems)
	{
		if (Item->IsTest() && TestNames.Contains(Item->Name))
		{
			Item->bRunning = true;
		}
	}

	// Simulate completion
	bIsRunning = false;
	Progress = 1.0f;
	StatusText = FText::FromString(FString::Printf(TEXT("Completed %d tests"), TestNames.Num()));

	UpdateResults();
}

void SDelveDeepTestRunner::UpdateResults()
{
	// Update test results
	// In a real implementation, this would query FAutomationTestFramework for results

	int32 PassedCount = 0;
	int32 FailedCount = 0;

	for (const TSharedPtr<FDelveDeepTestTreeItem>& Item : AllItems)
	{
		if (Item->IsTest() && Item->bRunning)
		{
			// Simulate results (90% pass rate)
			Item->bPassed = FMath::FRand() > 0.1f;
			Item->bRunning = false;
			Item->ExecutionTime = FMath::FRandRange(0.1f, 5.0f);

			if (Item->bPassed)
			{
				PassedCount++;
			}
			else
			{
				FailedCount++;
				Item->Errors.Add(TEXT("Simulated test failure"));
			}
		}
	}

	StatusText = FText::FromString(FString::Printf(
		TEXT("Tests completed: %d passed, %d failed"), PassedCount, FailedCount));

	if (TestTreeView.IsValid())
	{
		TestTreeView->RequestTreeRefresh();
	}
}

FSlateColor SDelveDeepTestRunner::GetTestItemColor(TSharedPtr<FDelveDeepTestTreeItem> Item) const
{
	if (!Item.IsValid())
	{
		return FSlateColor::UseForeground();
	}

	if (Item->bRunning)
	{
		return FLinearColor::Yellow;
	}

	if (Item->IsTest())
	{
		return Item->bPassed ? FLinearColor::Green : FLinearColor::Red;
	}

	return FSlateColor::UseForeground();
}

const FSlateBrush* SDelveDeepTestRunner::GetTestItemIcon(TSharedPtr<FDelveDeepTestTreeItem> Item) const
{
	if (!Item.IsValid())
	{
		return nullptr;
	}

	if (Item->IsSuite())
	{
		return FAppStyle::GetBrush("Icons.FolderClosed");
	}

	if (Item->bRunning)
	{
		return FAppStyle::GetBrush("Icons.CircleArrowRight");
	}

	if (Item->bPassed)
	{
		return FAppStyle::GetBrush("Icons.Check");
	}

	return FAppStyle::GetBrush("Icons.X");
}

// Test runner row implementation
void SDelveDeepTestRunnerRow::Construct(
	const FArguments& InArgs,
	const TSharedRef<STableViewBase>& InOwnerTable,
	TSharedPtr<FDelveDeepTestTreeItem> InItem)
{
	Item = InItem;

	STableRow<TSharedPtr<FDelveDeepTestTreeItem>>::Construct(
		STableRow<TSharedPtr<FDelveDeepTestTreeItem>>::FArguments()
		.Padding(2.0f)
		.Content()
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f, 0.0f)
			[
				SNew(SImage)
				.Image_Lambda([this]()
				{
					if (Item->IsSuite())
					{
						return FAppStyle::GetBrush("Icons.FolderClosed");
					}
					if (Item->bRunning)
					{
						return FAppStyle::GetBrush("Icons.CircleArrowRight");
					}
					if (Item->bPassed)
					{
						return FAppStyle::GetBrush("Icons.Check");
					}
					return FAppStyle::GetBrush("Icons.X");
				})
			]

			// Name
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->Name))
				.ColorAndOpacity_Lambda([this]()
				{
					if (Item->bRunning)
					{
						return FLinearColor::Yellow;
					}
					if (Item->IsTest())
					{
						return Item->bPassed ? FLinearColor::Green : FLinearColor::Red;
					}
					return FLinearColor::White;
				})
			]

			// Execution time
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (Item->IsTest() && Item->ExecutionTime > 0.0f)
					{
						return FText::FromString(FString::Printf(TEXT("%.2fs"), Item->ExecutionTime));
					}
					return FText::GetEmpty();
				})
			]
		],
		InOwnerTable
	);
}
