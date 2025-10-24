// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepFramePerformanceTracker.h"
#include "DelveDeepSystemProfiler.h"
#include "DelveDeepMemoryTracker.h"
#include "DelveDeepPerformanceOverlay.generated.h"

/**
 * Overlay display modes for performance visualization
 */
UENUM(BlueprintType)
enum class EOverlayMode : uint8
{
	/** FPS counter only */
	Minimal UMETA(DisplayName = "Minimal"),
	
	/** FPS + frame time graph */
	Standard UMETA(DisplayName = "Standard"),
	
	/** FPS + graph + system breakdown + memory */
	Detailed UMETA(DisplayName = "Detailed")
};

/**
 * Performance zones for color-coded visualization
 */
enum class EPerformanceZone : uint8
{
	/** Frame time < 16ms (60+ FPS) */
	Good,
	
	/** Frame time 16-20ms (50-60 FPS) */
	Warning,
	
	/** Frame time > 20ms (<50 FPS) */
	Critical
};

/**
 * Real-time performance overlay for visualizing telemetry data
 * 
 * Provides three display modes:
 * - Minimal: FPS counter only
 * - Standard: FPS + frame time graph (last 120 frames)
 * - Detailed: FPS + graph + system breakdown + memory usage
 * 
 * Designed for minimal overhead (<0.1ms per frame) while providing
 * actionable real-time performance insights.
 */
class DELVEDEEP_API FDelveDeepPerformanceOverlay
{
public:
	FDelveDeepPerformanceOverlay();

	/**
	 * Render the performance overlay
	 * @param Canvas Canvas to draw on
	 * @param FrameData Current frame performance data
	 * @param SystemData System performance data
	 * @param MemoryData Memory snapshot
	 */
	void Render(UCanvas* Canvas,
	           const FFramePerformanceData& FrameData,
	           const TArray<FSystemPerformanceData>& SystemData,
	           const FMemorySnapshot& MemoryData);

	/**
	 * Set the overlay display mode
	 * @param NewMode Display mode to set
	 */
	void SetMode(EOverlayMode NewMode);

	/**
	 * Get the current overlay display mode
	 * @return Current display mode
	 */
	EOverlayMode GetMode() const { return Mode; }

	/**
	 * Add a frame time to the graph history
	 * @param FrameTimeMs Frame time in milliseconds
	 */
	void AddFrameTime(float FrameTimeMs);

	/**
	 * Clear the frame time graph history
	 */
	void ClearHistory();

private:
	/** Current display mode */
	EOverlayMode Mode;

	/** Frame time history for graph (last 120 frames) */
	TArray<float> FrameTimeGraph;

	/** Maximum frames to store in graph */
	static constexpr int32 MaxGraphFrames = 120;

	/** Overlay position and sizing */
	static constexpr float OverlayX = 20.0f;
	static constexpr float OverlayY = 20.0f;
	static constexpr float GraphWidth = 240.0f;
	static constexpr float GraphHeight = 100.0f;
	static constexpr float LineHeight = 20.0f;

	/** Performance zone thresholds (in milliseconds) */
	static constexpr float GoodThreshold = 16.0f;
	static constexpr float WarningThreshold = 20.0f;

	/**
	 * Render minimal overlay (FPS only)
	 * @param Canvas Canvas to draw on
	 * @param FrameData Frame performance data
	 * @param X X position
	 * @param Y Y position
	 * @return Next Y position after rendering
	 */
	float RenderMinimal(UCanvas* Canvas, const FFramePerformanceData& FrameData, float X, float Y);

	/**
	 * Render standard overlay (FPS + graph)
	 * @param Canvas Canvas to draw on
	 * @param FrameData Frame performance data
	 * @param X X position
	 * @param Y Y position
	 * @return Next Y position after rendering
	 */
	float RenderStandard(UCanvas* Canvas, const FFramePerformanceData& FrameData, float X, float Y);

	/**
	 * Render detailed overlay (FPS + graph + systems + memory)
	 * @param Canvas Canvas to draw on
	 * @param FrameData Frame performance data
	 * @param SystemData System performance data
	 * @param MemoryData Memory snapshot
	 * @param X X position
	 * @param Y Y position
	 * @return Next Y position after rendering
	 */
	float RenderDetailed(UCanvas* Canvas,
	                    const FFramePerformanceData& FrameData,
	                    const TArray<FSystemPerformanceData>& SystemData,
	                    const FMemorySnapshot& MemoryData,
	                    float X, float Y);

	/**
	 * Render frame time graph
	 * @param Canvas Canvas to draw on
	 * @param X X position
	 * @param Y Y position
	 * @return Next Y position after rendering
	 */
	float RenderFrameTimeGraph(UCanvas* Canvas, float X, float Y);

	/**
	 * Render system breakdown
	 * @param Canvas Canvas to draw on
	 * @param SystemData System performance data
	 * @param X X position
	 * @param Y Y position
	 * @return Next Y position after rendering
	 */
	float RenderSystemBreakdown(UCanvas* Canvas, const TArray<FSystemPerformanceData>& SystemData, float X, float Y);

	/**
	 * Render memory statistics
	 * @param Canvas Canvas to draw on
	 * @param MemoryData Memory snapshot
	 * @param X X position
	 * @param Y Y position
	 * @return Next Y position after rendering
	 */
	float RenderMemoryStats(UCanvas* Canvas, const FMemorySnapshot& MemoryData, float X, float Y);

	/**
	 * Get color for performance zone
	 * @param FrameTimeMs Frame time in milliseconds
	 * @return Color for the zone
	 */
	FLinearColor GetPerformanceZoneColor(float FrameTimeMs) const;

	/**
	 * Get performance zone for frame time
	 * @param FrameTimeMs Frame time in milliseconds
	 * @return Performance zone
	 */
	EPerformanceZone GetPerformanceZone(float FrameTimeMs) const;

	/**
	 * Draw text with shadow for better visibility
	 * @param Canvas Canvas to draw on
	 * @param Text Text to draw
	 * @param X X position
	 * @param Y Y position
	 * @param Color Text color
	 */
	void DrawTextWithShadow(UCanvas* Canvas, const FString& Text, float X, float Y, const FLinearColor& Color) const;

	/**
	 * Draw a filled rectangle
	 * @param Canvas Canvas to draw on
	 * @param X X position
	 * @param Y Y position
	 * @param Width Rectangle width
	 * @param Height Rectangle height
	 * @param Color Fill color
	 */
	void DrawFilledRect(UCanvas* Canvas, float X, float Y, float Width, float Height, const FLinearColor& Color) const;

	/**
	 * Format bytes to human-readable string
	 * @param Bytes Number of bytes
	 * @return Formatted string (e.g., "1.5 MB")
	 */
	FString FormatBytes(uint64 Bytes) const;
};
