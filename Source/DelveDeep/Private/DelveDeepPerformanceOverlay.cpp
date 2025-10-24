// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepPerformanceOverlay.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "DelveDeepTelemetrySubsystem.h"

FDelveDeepPerformanceOverlay::FDelveDeepPerformanceOverlay()
	: Mode(EOverlayMode::Standard)
{
	FrameTimeGraph.Reserve(MaxGraphFrames);
}

void FDelveDeepPerformanceOverlay::Render(UCanvas* Canvas,
                                         const FFramePerformanceData& FrameData,
                                         const TArray<FSystemPerformanceData>& SystemData,
                                         const FMemorySnapshot& MemoryData)
{
	if (!Canvas)
	{
		return;
	}

	// Measure rendering time to ensure <0.1ms overhead
	const double StartTime = FPlatformTime::Seconds();

	float X = OverlayX;
	float Y = OverlayY;

	switch (Mode)
	{
	case EOverlayMode::Minimal:
		RenderMinimal(Canvas, FrameData, X, Y);
		break;

	case EOverlayMode::Standard:
		RenderStandard(Canvas, FrameData, X, Y);
		break;

	case EOverlayMode::Detailed:
		RenderDetailed(Canvas, FrameData, SystemData, MemoryData, X, Y);
		break;
	}

	const double EndTime = FPlatformTime::Seconds();
	const double RenderTimeMs = (EndTime - StartTime) * 1000.0;

	// Log warning if overlay rendering exceeds target
	if (RenderTimeMs > 0.1)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning,
			TEXT("Performance overlay rendering exceeded target: %.3fms"), RenderTimeMs);
	}
}

void FDelveDeepPerformanceOverlay::SetMode(EOverlayMode NewMode)
{
	Mode = NewMode;
}

void FDelveDeepPerformanceOverlay::AddFrameTime(float FrameTimeMs)
{
	FrameTimeGraph.Add(FrameTimeMs);

	// Keep only last MaxGraphFrames
	if (FrameTimeGraph.Num() > MaxGraphFrames)
	{
		FrameTimeGraph.RemoveAt(0);
	}
}

void FDelveDeepPerformanceOverlay::ClearHistory()
{
	FrameTimeGraph.Empty();
	FrameTimeGraph.Reserve(MaxGraphFrames);
}

float FDelveDeepPerformanceOverlay::RenderMinimal(UCanvas* Canvas, const FFramePerformanceData& FrameData, float X, float Y)
{
	// Calculate FPS
	const float FPS = FrameData.FrameTime > 0.0f ? 1000.0f / FrameData.FrameTime : 0.0f;
	const FLinearColor Color = GetPerformanceZoneColor(FrameData.FrameTime);

	// Draw FPS
	const FString FPSText = FString::Printf(TEXT("FPS: %.1f"), FPS);
	DrawTextWithShadow(Canvas, FPSText, X, Y, Color);

	return Y + LineHeight;
}

float FDelveDeepPerformanceOverlay::RenderStandard(UCanvas* Canvas, const FFramePerformanceData& FrameData, float X, float Y)
{
	// Render FPS
	Y = RenderMinimal(Canvas, FrameData, X, Y);

	// Add spacing
	Y += 5.0f;

	// Render frame time graph
	Y = RenderFrameTimeGraph(Canvas, X, Y);

	return Y;
}

float FDelveDeepPerformanceOverlay::RenderDetailed(UCanvas* Canvas,
                                                  const FFramePerformanceData& FrameData,
                                                  const TArray<FSystemPerformanceData>& SystemData,
                                                  const FMemorySnapshot& MemoryData,
                                                  float X, float Y)
{
	// Render standard overlay (FPS + graph)
	Y = RenderStandard(Canvas, FrameData, X, Y);

	// Add spacing
	Y += 10.0f;

	// Render system breakdown
	Y = RenderSystemBreakdown(Canvas, SystemData, X, Y);

	// Add spacing
	Y += 10.0f;

	// Render memory stats
	Y = RenderMemoryStats(Canvas, MemoryData, X, Y);

	return Y;
}

float FDelveDeepPerformanceOverlay::RenderFrameTimeGraph(UCanvas* Canvas, float X, float Y)
{
	if (FrameTimeGraph.Num() == 0)
	{
		return Y;
	}

	// Draw graph background
	DrawFilledRect(Canvas, X, Y, GraphWidth, GraphHeight, FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));

	// Draw performance zone lines
	const float GoodLineY = Y + GraphHeight - (GoodThreshold / 33.33f) * GraphHeight; // 33.33ms = 30 FPS scale
	const float WarningLineY = Y + GraphHeight - (WarningThreshold / 33.33f) * GraphHeight;

	DrawFilledRect(Canvas, X, GoodLineY, GraphWidth, 1.0f, FLinearColor(0.0f, 1.0f, 0.0f, 0.3f));
	DrawFilledRect(Canvas, X, WarningLineY, GraphWidth, 1.0f, FLinearColor(1.0f, 1.0f, 0.0f, 0.3f));

	// Draw frame time bars
	const float BarWidth = GraphWidth / MaxGraphFrames;
	const int32 StartIndex = FMath::Max(0, FrameTimeGraph.Num() - MaxGraphFrames);

	for (int32 i = StartIndex; i < FrameTimeGraph.Num(); ++i)
	{
		const float FrameTime = FrameTimeGraph[i];
		const float BarHeight = FMath::Min((FrameTime / 33.33f) * GraphHeight, GraphHeight);
		const float BarX = X + (i - StartIndex) * BarWidth;
		const float BarY = Y + GraphHeight - BarHeight;

		const FLinearColor BarColor = GetPerformanceZoneColor(FrameTime);
		DrawFilledRect(Canvas, BarX, BarY, BarWidth, BarHeight, BarColor);
	}

	// Draw graph border
	Canvas->K2_DrawBox(FVector2D(X, Y), FVector2D(GraphWidth, GraphHeight), 1.0f, FLinearColor::White);

	// Draw scale labels
	DrawTextWithShadow(Canvas, TEXT("16ms"), X + GraphWidth + 5.0f, GoodLineY - 8.0f, FLinearColor::Green);
	DrawTextWithShadow(Canvas, TEXT("20ms"), X + GraphWidth + 5.0f, WarningLineY - 8.0f, FLinearColor::Yellow);

	return Y + GraphHeight + 5.0f;
}

float FDelveDeepPerformanceOverlay::RenderSystemBreakdown(UCanvas* Canvas, const TArray<FSystemPerformanceData>& SystemData, float X, float Y)
{
	if (SystemData.Num() == 0)
	{
		return Y;
	}

	// Draw header
	DrawTextWithShadow(Canvas, TEXT("System Breakdown (Top 5):"), X, Y, FLinearColor::White);
	Y += LineHeight;

	// Sort systems by cycle time (descending)
	TArray<FSystemPerformanceData> SortedSystems = SystemData;
	SortedSystems.Sort([](const FSystemPerformanceData& A, const FSystemPerformanceData& B)
	{
		return A.CycleTime > B.CycleTime;
	});

	// Calculate total time for percentage
	double TotalTime = 0.0;
	for (const FSystemPerformanceData& System : SortedSystems)
	{
		TotalTime += System.CycleTime;
	}

	// Draw top 5 systems
	const int32 MaxSystems = FMath::Min(5, SortedSystems.Num());
	for (int32 i = 0; i < MaxSystems; ++i)
	{
		const FSystemPerformanceData& System = SortedSystems[i];
		const float Percentage = TotalTime > 0.0 ? (System.CycleTime / TotalTime) * 100.0f : 0.0f;

		// Draw system name and time
		const FString SystemText = FString::Printf(TEXT("%s: %.2fms (%.1f%%)"),
			*System.SystemName.ToString(), System.CycleTime, Percentage);

		// Color based on budget utilization
		FLinearColor Color = FLinearColor::White;
		if (System.BudgetTime > 0.0)
		{
			const float Utilization = System.CycleTime / System.BudgetTime;
			if (Utilization > 1.0f)
			{
				Color = FLinearColor::Red;
			}
			else if (Utilization > 0.8f)
			{
				Color = FLinearColor::Yellow;
			}
			else
			{
				Color = FLinearColor::Green;
			}
		}

		DrawTextWithShadow(Canvas, SystemText, X + 10.0f, Y, Color);

		// Draw percentage bar
		const float BarWidth = (Percentage / 100.0f) * (GraphWidth - 20.0f);
		DrawFilledRect(Canvas, X + 10.0f, Y + LineHeight - 5.0f, BarWidth, 3.0f, Color);

		Y += LineHeight;
	}

	return Y;
}

float FDelveDeepPerformanceOverlay::RenderMemoryStats(UCanvas* Canvas, const FMemorySnapshot& MemoryData, float X, float Y)
{
	// Draw header
	DrawTextWithShadow(Canvas, TEXT("Memory:"), X, Y, FLinearColor::White);
	Y += LineHeight;

	// Draw total memory
	const FString TotalMemText = FString::Printf(TEXT("Total: %s"), *FormatBytes(MemoryData.TotalMemory));
	DrawTextWithShadow(Canvas, TotalMemText, X + 10.0f, Y, FLinearColor::White);
	Y += LineHeight;

	// Draw native memory
	const FString NativeMemText = FString::Printf(TEXT("Native: %s"), *FormatBytes(MemoryData.NativeMemory));
	DrawTextWithShadow(Canvas, NativeMemText, X + 10.0f, Y, FLinearColor::Cyan);
	Y += LineHeight;

	// Draw managed memory
	const FString ManagedMemText = FString::Printf(TEXT("Managed: %s"), *FormatBytes(MemoryData.ManagedMemory));
	DrawTextWithShadow(Canvas, ManagedMemText, X + 10.0f, Y, FLinearColor::Cyan);
	Y += LineHeight;

	return Y;
}

FLinearColor FDelveDeepPerformanceOverlay::GetPerformanceZoneColor(float FrameTimeMs) const
{
	const EPerformanceZone Zone = GetPerformanceZone(FrameTimeMs);

	switch (Zone)
	{
	case EPerformanceZone::Good:
		return FLinearColor::Green;

	case EPerformanceZone::Warning:
		return FLinearColor::Yellow;

	case EPerformanceZone::Critical:
		return FLinearColor::Red;

	default:
		return FLinearColor::White;
	}
}

EPerformanceZone FDelveDeepPerformanceOverlay::GetPerformanceZone(float FrameTimeMs) const
{
	if (FrameTimeMs < GoodThreshold)
	{
		return EPerformanceZone::Good;
	}
	else if (FrameTimeMs < WarningThreshold)
	{
		return EPerformanceZone::Warning;
	}
	else
	{
		return EPerformanceZone::Critical;
	}
}

void FDelveDeepPerformanceOverlay::DrawTextWithShadow(UCanvas* Canvas, const FString& Text, float X, float Y, const FLinearColor& Color) const
{
	if (!Canvas)
	{
		return;
	}

	// Draw shadow
	FCanvasTextItem ShadowTextItem(FVector2D(X + 1.0f, Y + 1.0f), FText::FromString(Text), GEngine->GetSmallFont(), FLinearColor::Black);
	ShadowTextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(ShadowTextItem);

	// Draw text
	FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), GEngine->GetSmallFont(), Color);
	Canvas->DrawItem(TextItem);
}

void FDelveDeepPerformanceOverlay::DrawFilledRect(UCanvas* Canvas, float X, float Y, float Width, float Height, const FLinearColor& Color) const
{
	if (!Canvas)
	{
		return;
	}

	FCanvasTileItem TileItem(FVector2D(X, Y), FVector2D(Width, Height), Color);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);
}

FString FDelveDeepPerformanceOverlay::FormatBytes(uint64 Bytes) const
{
	if (Bytes < 1024)
	{
		return FString::Printf(TEXT("%llu B"), Bytes);
	}
	else if (Bytes < 1024 * 1024)
	{
		return FString::Printf(TEXT("%.2f KB"), Bytes / 1024.0);
	}
	else if (Bytes < 1024 * 1024 * 1024)
	{
		return FString::Printf(TEXT("%.2f MB"), Bytes / (1024.0 * 1024.0));
	}
	else
	{
		return FString::Printf(TEXT("%.2f GB"), Bytes / (1024.0 * 1024.0 * 1024.0));
	}
}
