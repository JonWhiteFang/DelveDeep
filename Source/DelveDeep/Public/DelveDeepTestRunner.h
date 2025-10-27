// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"
#include "DelveDeepTestReport.h"

/**
 * Visual Test Runner
 * 
 * Slate widget for running and viewing automated tests in the Unreal Editor.
 * Provides a tree view of tests, filtering, execution controls, and results display.
 * 
 * Features:
 * - Tree view of all tests organized by suite
 * - Run/stop buttons for test execution
 * - Real-time progress display
 * - Filtering by name or tag
 * - Color-coded results (pass/fail)
 * - Detailed failure information
 */

class FDelveDeepTestTreeItem;

/** Test tree item representing a test suite or individual test */
class FDelveDeepTestTreeItem : public TSharedFromThis<FDelveDeepTestTreeItem>
{
public:
	FDelveDeepTestTreeItem(const FString& InName, bool bInIsTest)
		: Name(InName)
		, bIsTest(bInIsTest)
		, bPassed(false)
		, bRunning(false)
		, ExecutionTime(0.0f)
	{}

	FString Name;
	bool bIsTest;
	bool bPassed;
	bool bRunning;
	float ExecutionTime;
	TArray<FString> Errors;
	TArray<TSharedPtr<FDelveDeepTestTreeItem>> Children;
	TWeakPtr<FDelveDeepTestTreeItem> Parent;

	void AddChild(TSharedPtr<FDelveDeepTestTreeItem> Child)
	{
		Child->Parent = AsShared();
		Children.Add(Child);
	}

	bool HasChildren() const { return Children.Num() > 0; }
	bool IsTest() const { return bIsTest; }
	bool IsSuite() const { return !bIsTest; }
};

/** Visual test runner widget */
class SDelveDeepTestRunner : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDelveDeepTestRunner) {}
	SLATE_END_ARGS()

	/** Construct the widget */
	void Construct(const FArguments& InArgs);

	/** Refresh the test list */
	void RefreshTestList();

	/** Run selected tests */
	void RunSelectedTests();

	/** Run all tests */
	void RunAllTests();

	/** Stop test execution */
	void StopTests();

	/** Filter tests by name */
	void FilterTests(const FString& FilterText);

	/** Get current test results */
	FDelveDeepTestReport GetTestResults() const { return CurrentReport; }

private:
	/** Generate tree view row */
	TSharedRef<ITableRow> OnGenerateRow(
		TSharedPtr<FDelveDeepTestTreeItem> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	/** Get children for tree item */
	void OnGetChildren(
		TSharedPtr<FDelveDeepTestTreeItem> Item,
		TArray<TSharedPtr<FDelveDeepTestTreeItem>>& OutChildren);

	/** Handle selection changed */
	void OnSelectionChanged(
		TSharedPtr<FDelveDeepTestTreeItem> Item,
		ESelectInfo::Type SelectInfo);

	/** Handle run button clicked */
	FReply OnRunClicked();

	/** Handle stop button clicked */
	FReply OnStopClicked();

	/** Handle refresh button clicked */
	FReply OnRefreshClicked();

	/** Handle filter text changed */
	void OnFilterTextChanged(const FText& InFilterText);

	/** Build test tree from automation system */
	void BuildTestTree();

	/** Execute tests */
	void ExecuteTests(const TArray<FString>& TestNames);

	/** Update test results */
	void UpdateResults();

	/** Get color for test item */
	FSlateColor GetTestItemColor(TSharedPtr<FDelveDeepTestTreeItem> Item) const;

	/** Get icon for test item */
	const FSlateBrush* GetTestItemIcon(TSharedPtr<FDelveDeepTestTreeItem> Item) const;

	/** Tree view widget */
	TSharedPtr<STreeView<TSharedPtr<FDelveDeepTestTreeItem>>> TestTreeView;

	/** Root items for tree */
	TArray<TSharedPtr<FDelveDeepTestTreeItem>> RootItems;

	/** All test items (flat list) */
	TArray<TSharedPtr<FDelveDeepTestTreeItem>> AllItems;

	/** Currently selected items */
	TArray<TSharedPtr<FDelveDeepTestTreeItem>> SelectedItems;

	/** Filter text */
	FString FilterText;

	/** Current test report */
	FDelveDeepTestReport CurrentReport;

	/** Is currently running tests */
	bool bIsRunning;

	/** Progress (0.0 to 1.0) */
	float Progress;

	/** Status text */
	FText StatusText;
};

/** Test runner row widget */
class SDelveDeepTestRunnerRow : public STableRow<TSharedPtr<FDelveDeepTestTreeItem>>
{
public:
	SLATE_BEGIN_ARGS(SDelveDeepTestRunnerRow) {}
	SLATE_END_ARGS()

	void Construct(
		const FArguments& InArgs,
		const TSharedRef<STableViewBase>& InOwnerTable,
		TSharedPtr<FDelveDeepTestTreeItem> InItem);

private:
	TSharedPtr<FDelveDeepTestTreeItem> Item;
};
