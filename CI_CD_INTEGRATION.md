# CI/CD Integration Guide

This document explains how to integrate DelveDeep's automated testing framework with various CI/CD systems.

## Quick Start

### Unix/Linux/Mac
```bash
export UE5_ENGINE_PATH="/path/to/UnrealEngine"
./RunTests.sh
```

### Windows
```batch
set UE5_ENGINE_PATH=C:\UnrealEngine
RunTests.bat
```

## Test Scripts

### RunTests.sh (Unix/Linux/Mac)

**Features:**
- Runs all unit tests (ProductFilter by default)
- Generates test reports in TestResults/
- Exits with non-zero code on failure
- Uses headless mode (-NullRHI) for CI environments
- Colored output for easy reading
- Automatic platform detection (macOS/Linux)

**Environment Variables:**
- `UE5_ENGINE_PATH` - Path to Unreal Engine installation (required)
- `TEST_FILTER` - Test filter to run (default: Product)
  - `Product` - Unit tests
  - `Perf` - Performance tests
  - `Stress` - Stress tests

**Usage:**
```bash
# Run unit tests (default)
./RunTests.sh

# Run performance tests
TEST_FILTER=Perf ./RunTests.sh

# Run stress tests
TEST_FILTER=Stress ./RunTests.sh
```

### RunTests.bat (Windows)

**Features:**
- Runs all unit tests (ProductFilter by default)
- Generates test reports in TestResults/
- Exits with non-zero code on failure
- Uses headless mode (-NullRHI) for CI environments
- Colored output for easy reading

**Environment Variables:**
- `UE5_ENGINE_PATH` - Path to Unreal Engine installation (required)
- `TEST_FILTER` - Test filter to run (default: Product)
  - `Product` - Unit tests
  - `Perf` - Performance tests
  - `Stress` - Stress tests

**Usage:**
```batch
REM Run unit tests (default)
RunTests.bat

REM Run performance tests
set TEST_FILTER=Perf
RunTests.bat

REM Run stress tests
set TEST_FILTER=Stress
RunTests.bat
```

## CI/CD Platform Integration

### GitHub Actions

Create `.github/workflows/tests.yml`:

```yaml
name: Automated Tests

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Setup Unreal Engine
      uses: game-ci/unity-builder@v2
      with:
        unreal-version: '5.6'
    
    - name: Run unit tests
      env:
        UE5_ENGINE_PATH: ${{ secrets.UE5_ENGINE_PATH }}
      run: |
        chmod +x RunTests.sh
        ./RunTests.sh
    
    - name: Upload test reports
      if: always()
      uses: actions/upload-artifact@v3
      with:
        name: test-reports
        path: TestResults/
    
    - name: Publish test results
      if: always()
      uses: EnricoMi/publish-unit-test-result-action@v2
      with:
        files: TestResults/**/*.xml

  performance:
    runs-on: ubuntu-latest
    if: github.event_name == 'push' && github.ref == 'refs/heads/main'
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Setup Unreal Engine
      uses: game-ci/unity-builder@v2
      with:
        unreal-version: '5.6'
    
    - name: Run performance tests
      env:
        UE5_ENGINE_PATH: ${{ secrets.UE5_ENGINE_PATH }}
        TEST_FILTER: Perf
      run: |
        chmod +x RunTests.sh
        ./RunTests.sh
    
    - name: Upload performance reports
      if: always()
      uses: actions/upload-artifact@v3
      with:
        name: performance-reports
        path: TestResults/
```

### GitLab CI

Create `.gitlab-ci.yml`:

```yaml
stages:
  - test
  - performance

variables:
  UE5_ENGINE_PATH: "/opt/UnrealEngine"

unit_tests:
  stage: test
  script:
    - chmod +x RunTests.sh
    - ./RunTests.sh
  artifacts:
    when: always
    reports:
      junit: TestResults/**/*.xml
    paths:
      - TestResults/
  only:
    - main
    - develop
    - merge_requests

performance_tests:
  stage: performance
  script:
    - chmod +x RunTests.sh
    - TEST_FILTER=Perf ./RunTests.sh
  artifacts:
    when: always
    paths:
      - TestResults/
  only:
    - main
  when: manual

stress_tests:
  stage: performance
  script:
    - chmod +x RunTests.sh
    - TEST_FILTER=Stress ./RunTests.sh
  artifacts:
    when: always
    paths:
      - TestResults/
  only:
    - schedules
```

### Jenkins

Create `Jenkinsfile`:

```groovy
pipeline {
    agent any
    
    environment {
        UE5_ENGINE_PATH = 'C:\\UnrealEngine'
    }
    
    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        
        stage('Unit Tests') {
            steps {
                bat 'RunTests.bat'
            }
            post {
                always {
                    junit 'TestResults/**/*.xml'
                    archiveArtifacts artifacts: 'TestResults/**/*', allowEmptyArchive: true
                }
            }
        }
        
        stage('Performance Tests') {
            when {
                branch 'main'
            }
            steps {
                bat '''
                    set TEST_FILTER=Perf
                    RunTests.bat
                '''
            }
            post {
                always {
                    archiveArtifacts artifacts: 'TestResults/**/*', allowEmptyArchive: true
                }
            }
        }
    }
    
    post {
        failure {
            emailext (
                subject: "Build Failed: ${env.JOB_NAME} - ${env.BUILD_NUMBER}",
                body: "Test failures detected. Check console output for details.",
                to: "${env.CHANGE_AUTHOR_EMAIL}"
            )
        }
    }
}
```

### Azure DevOps

Create `azure-pipelines.yml`:

```yaml
trigger:
  branches:
    include:
    - main
    - develop

pool:
  vmImage: 'windows-latest'

variables:
  UE5_ENGINE_PATH: 'C:\UnrealEngine'

stages:
- stage: Test
  jobs:
  - job: UnitTests
    steps:
    - script: RunTests.bat
      displayName: 'Run Unit Tests'
      env:
        UE5_ENGINE_PATH: $(UE5_ENGINE_PATH)
    
    - task: PublishTestResults@2
      condition: always()
      inputs:
        testResultsFormat: 'JUnit'
        testResultsFiles: 'TestResults/**/*.xml'
        failTaskOnFailedTests: true
    
    - task: PublishBuildArtifacts@1
      condition: always()
      inputs:
        pathToPublish: 'TestResults'
        artifactName: 'test-reports'

- stage: Performance
  condition: and(succeeded(), eq(variables['Build.SourceBranch'], 'refs/heads/main'))
  jobs:
  - job: PerformanceTests
    steps:
    - script: |
        set TEST_FILTER=Perf
        RunTests.bat
      displayName: 'Run Performance Tests'
      env:
        UE5_ENGINE_PATH: $(UE5_ENGINE_PATH)
    
    - task: PublishBuildArtifacts@1
      condition: always()
      inputs:
        pathToPublish: 'TestResults'
        artifactName: 'performance-reports'
```

## Test Report Formats

The test scripts generate reports in multiple formats:

### JSON Format
- Machine-readable test results
- Location: `TestResults/index.json`
- Contains: Test names, pass/fail status, execution time, error messages

### HTML Format
- Human-readable test results
- Location: `TestResults/index.html`
- Contains: Visual test report with filtering and sorting

### JUnit XML Format
- CI/CD integration format
- Location: `TestResults/junit.xml`
- Contains: Standard JUnit XML for CI dashboards

## Performance Targets

Tests are configured to meet these performance targets:

**Configuration System:**
- Initialization: <100ms for 100+ assets
- Single Query: <1ms per query
- Bulk Queries: <1ms average for 1000 queries
- Cache Hit Rate: >95% for repeated queries

**Event System:**
- Event Broadcast: <1ms for 50 listeners
- Listener Invocation: <0.1ms per listener
- System Overhead: <0.1ms per event
- Deferred Processing: <10ms for 1000 events

**Telemetry System:**
- Frame Tracking: <0.1ms per frame update
- Memory Tracking: <0.1ms per allocation
- Budget Validation: <1ms per check

**Full Test Suite:**
- Total Execution Time: <30 seconds

## Troubleshooting

### Tests Not Running

**Check engine path:**
```bash
echo $UE5_ENGINE_PATH  # Unix
echo %UE5_ENGINE_PATH%  # Windows
```

**Verify project file exists:**
```bash
ls -la DelveDeep.uproject  # Unix
dir DelveDeep.uproject     # Windows
```

**Check script permissions (Unix):**
```bash
chmod +x RunTests.sh
```

### Tests Failing in CI

**Use headless mode:**
- Scripts automatically use `-NullRHI` flag
- No graphics or input required

**Check for editor-only dependencies:**
- Use `#if WITH_EDITOR` guards
- Ensure tests work in runtime context

**Verify test isolation:**
- Tests should not depend on each other
- Use fixtures for proper setup/teardown

### Slow Test Execution

**Run specific test filter:**
```bash
TEST_FILTER=Product ./RunTests.sh  # Fast unit tests only
```

**Check for performance regressions:**
```bash
TEST_FILTER=Perf ./RunTests.sh  # Run performance benchmarks
```

**Review test reports:**
- Check `TestResults/` for execution times
- Identify slow tests
- Optimize or move to stress filter

## Best Practices

### Development Workflow

1. **Run unit tests locally** before committing
2. **Use test filtering** to run only relevant tests
3. **Check test reports** for warnings and failures
4. **Fix failing tests** before pushing

### CI/CD Pipeline

1. **Run unit tests on every commit** - Fast feedback
2. **Run performance tests on main branch** - Track trends
3. **Run stress tests nightly** - Verify stability
4. **Generate and archive reports** - Historical tracking
5. **Fail build on test failures** - Enforce quality

### Test Maintenance

1. **Keep tests fast** - Target <30 seconds for unit tests
2. **Update performance targets** - As systems improve
3. **Add tests for new features** - Maintain coverage
4. **Remove obsolete tests** - Keep suite clean
5. **Document complex tests** - Aid understanding

## Environment Setup

### Local Development

**Unix/Linux/Mac:**
```bash
# Add to ~/.bashrc or ~/.zshrc
export UE5_ENGINE_PATH="/path/to/UnrealEngine"
```

**Windows:**
```batch
REM Add to system environment variables
setx UE5_ENGINE_PATH "C:\UnrealEngine"
```

### CI/CD Environment

**GitHub Actions:**
- Set `UE5_ENGINE_PATH` as repository secret
- Use `secrets.UE5_ENGINE_PATH` in workflow

**GitLab CI:**
- Set `UE5_ENGINE_PATH` as CI/CD variable
- Mark as protected for main branch

**Jenkins:**
- Set `UE5_ENGINE_PATH` in environment variables
- Configure per-node if needed

**Azure DevOps:**
- Set `UE5_ENGINE_PATH` as pipeline variable
- Mark as secret if needed

## Support

For issues or questions:
1. Check test reports in `TestResults/`
2. Review test logs for detailed errors
3. Consult [Testing Framework README](Source/DelveDeep/Private/Tests/README.md)
4. Check [Test Filtering Guide](Source/DelveDeep/Private/Tests/TEST_FILTERING.md)

## References

- [Unreal Engine Automation System](https://docs.unrealengine.com/en-US/TestingAndOptimization/Automation/)
- [DelveDeep Testing Framework](Source/DelveDeep/Private/Tests/README.md)
- [Test Filtering Guide](Source/DelveDeep/Private/Tests/TEST_FILTERING.md)
