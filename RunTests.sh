#!/bin/bash
# DelveDeep Automated Test Runner (Unix/Linux/Mac)
# Runs all unit tests and generates test reports for CI/CD integration

set -e  # Exit on error

# Configuration
ENGINE_PATH="${UE5_ENGINE_PATH:-/path/to/UnrealEngine}"
PROJECT_PATH="$(cd "$(dirname "$0")" && pwd)/DelveDeep.uproject"
OUTPUT_PATH="$(cd "$(dirname "$0")" && pwd)/TestResults"
TEST_FILTER="${TEST_FILTER:-Product}"  # Default to unit tests

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Print banner
echo "========================================"
echo "DelveDeep Automated Test Runner"
echo "========================================"
echo ""

# Validate engine path
if [ ! -d "$ENGINE_PATH" ]; then
    echo -e "${RED}ERROR: Unreal Engine not found at: $ENGINE_PATH${NC}"
    echo "Set UE5_ENGINE_PATH environment variable or edit this script"
    exit 1
fi

# Validate project file
if [ ! -f "$PROJECT_PATH" ]; then
    echo -e "${RED}ERROR: Project file not found: $PROJECT_PATH${NC}"
    exit 1
fi

# Create output directory
mkdir -p "$OUTPUT_PATH"
echo -e "${GREEN}Output directory: $OUTPUT_PATH${NC}"
echo ""

# Determine editor executable based on platform
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    EDITOR_CMD="$ENGINE_PATH/Engine/Binaries/Mac/UnrealEditor-Cmd"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    EDITOR_CMD="$ENGINE_PATH/Engine/Binaries/Linux/UnrealEditor-Cmd"
else
    echo -e "${RED}ERROR: Unsupported platform: $OSTYPE${NC}"
    exit 1
fi

# Validate editor executable
if [ ! -f "$EDITOR_CMD" ]; then
    echo -e "${RED}ERROR: Editor executable not found: $EDITOR_CMD${NC}"
    exit 1
fi

# Print configuration
echo "Configuration:"
echo "  Engine: $ENGINE_PATH"
echo "  Project: $PROJECT_PATH"
echo "  Test Filter: $TEST_FILTER"
echo "  Editor: $EDITOR_CMD"
echo ""

# Run tests
echo -e "${YELLOW}Running tests...${NC}"
echo ""

START_TIME=$(date +%s)

# Execute tests with proper flags
"$EDITOR_CMD" \
    "$PROJECT_PATH" \
    -ExecCmds="Automation RunFilter $TEST_FILTER" \
    -ReportOutputPath="$OUTPUT_PATH" \
    -unattended \
    -nopause \
    -NullRHI \
    -log \
    -stdout \
    -FullStdOutLogOutput

TEST_EXIT_CODE=$?

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))

echo ""
echo "========================================"

# Check test results
if [ $TEST_EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    echo "  Duration: ${DURATION}s"
    echo "  Reports: $OUTPUT_PATH"
    echo "========================================"
    exit 0
else
    echo -e "${RED}✗ Tests failed!${NC}"
    echo "  Exit code: $TEST_EXIT_CODE"
    echo "  Duration: ${DURATION}s"
    echo "  Reports: $OUTPUT_PATH"
    echo "========================================"
    exit $TEST_EXIT_CODE
fi
