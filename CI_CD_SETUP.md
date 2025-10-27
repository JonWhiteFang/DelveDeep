# CI/CD Pipeline Setup Guide

This guide explains how to set up continuous integration and continuous deployment (CI/CD) pipelines for the DelveDeep automated testing framework.

## Overview

The DelveDeep project includes CI/CD configurations for three popular platforms:
- **GitHub Actions** (`.github/workflows/run-tests.yml`)
- **GitLab CI** (`.gitlab-ci.yml`)
- **Jenkins** (`Jenkinsfile`)

All configurations follow the same workflow:
1. Build the project
2. Run unit tests
3. Run integration tests
4. Run performance tests
5. Generate code coverage reports
6. Create consolidated test reports
7. Deploy reports (optional)

## Prerequisites

### All Platforms

- Unreal Engine 5.6 installed on build agents
- Git with LFS support
- Sufficient disk space (minimum 50GB per agent)
- Network access to repository

### Platform-Specific Requirements

**GitHub Actions:**
- GitHub repository with Actions enabled
- Self-hosted runners (recommended for UE5 projects)
- GitHub Pages enabled (optional, for report deployment)

**GitLab CI:**
- GitLab repository with CI/CD enabled
- GitLab Runners with appropriate tags
- GitLab Pages enabled (optional)

**Jenkins:**
- Jenkins server with Pipeline plugin
- Build agents with Windows and Linux
- Slack/Email plugins (optional, for notifications)

## GitHub Actions Setup

### 1. Configure Self-Hosted Runners

GitHub's hosted runners don't have Unreal Engine installed. You'll need self-hosted runners:

```bash
# On Windows
cd C:\actions-runner
.\config.cmd --url https://github.com/your-org/DelveDeep --token YOUR_TOKEN
.\run.cmd

# On Linux
cd /opt/actions-runner
./config.sh --url https://github.com/your-org/DelveDeep --token YOUR_TOKEN
./run.sh
```

### 2. Set Environment Variables

Update `.github/workflows/run-tests.yml` with your Unreal Engine paths:

```yaml
env:
  UE_VERSION: '5.6'
  UE_ROOT_WINDOWS: 'C:\UnrealEngine'  # Update this
  UE_ROOT_LINUX: '/opt/UnrealEngine'  # Update this
```

### 3. Configure Secrets

Add these secrets in GitHub repository settings:
- `CODECOV_TOKEN` (if using Codecov)
- `SLACK_WEBHOOK_URL` (if using Slack notifications)

### 4. Enable GitHub Pages

1. Go to repository Settings > Pages
2. Select "GitHub Actions" as source
3. Test reports will be available at `https://your-org.github.io/DelveDeep/test-reports/`

### 5. Test the Workflow

Push a commit to trigger the workflow:

```bash
git add .
git commit -m "Add CI/CD configuration"
git push origin main
```

Monitor progress at: `https://github.com/your-org/DelveDeep/actions`

## GitLab CI Setup

### 1. Configure GitLab Runners

Register runners with appropriate tags:

```bash
# Windows runner
gitlab-runner register \
  --url https://gitlab.com/ \
  --registration-token YOUR_TOKEN \
  --executor shell \
  --tag-list "windows,unreal"

# Linux runner
gitlab-runner register \
  --url https://gitlab.com/ \
  --registration-token YOUR_TOKEN \
  --executor shell \
  --tag-list "linux,unreal"
```

### 2. Set CI/CD Variables

In GitLab project settings > CI/CD > Variables, add:
- `UE_ROOT_WINDOWS`: `C:\UnrealEngine`
- `UE_ROOT_LINUX`: `/opt/UnrealEngine`
- `SLACK_WEBHOOK_URL`: Your Slack webhook (optional)

### 3. Enable GitLab Pages

GitLab Pages is automatically configured in `.gitlab-ci.yml`. Reports will be available at:
`https://your-org.gitlab.io/DelveDeep/`

### 4. Test the Pipeline

Push a commit to trigger the pipeline:

```bash
git add .
git commit -m "Add CI/CD configuration"
git push origin main
```

Monitor progress at: `https://gitlab.com/your-org/DelveDeep/-/pipelines`

## Jenkins Setup

### 1. Install Required Plugins

Install these Jenkins plugins:
- Pipeline
- Git
- JUnit
- HTML Publisher
- Email Extension
- Slack Notification (optional)

### 2. Configure Build Agents

Add Windows and Linux agents with labels:
- Windows agent: labels `windows` and `unreal`
- Linux agent: labels `linux` and `unreal`

### 3. Create Pipeline Job

1. New Item > Pipeline
2. Name: `DelveDeep-Tests`
3. Pipeline > Definition: "Pipeline script from SCM"
4. SCM: Git
5. Repository URL: Your repository URL
6. Script Path: `Jenkinsfile`

### 4. Configure Environment Variables

In Jenkins > Manage Jenkins > Configure System, add:
- `UE_ROOT_WINDOWS`: `C:\UnrealEngine`
- `UE_ROOT_LINUX`: `/opt/UnrealEngine`

### 5. Configure Notifications

**Email:**
1. Manage Jenkins > Configure System > Extended E-mail Notification
2. Set SMTP server and default recipients

**Slack:**
1. Install Slack Notification plugin
2. Configure Slack workspace and channel
3. Add webhook URL to Jenkins credentials

### 6. Test the Pipeline

Trigger a build manually or push a commit:

```bash
git add .
git commit -m "Add CI/CD configuration"
git push origin main
```

Monitor progress in Jenkins dashboard.

## Test Execution

### Running Specific Test Suites

**Unit Tests Only:**
```bash
# GitHub Actions
gh workflow run run-tests.yml -f test-suite=unit

# GitLab CI
# Modify .gitlab-ci.yml to run specific jobs

# Jenkins
# Use build parameters
```

**Integration Tests Only:**
```bash
# Similar to above, use test-suite=integration
```

**Performance Tests Only:**
```bash
# Similar to above, use test-suite=performance
```

### Manual Test Execution

You can also run tests manually on any platform:

```bash
# Windows
.\RunTests.bat -filter=Product -output=TestResults

# Linux
./RunTests.sh --filter=Product --output=TestResults
```

## Test Reports

### Viewing Reports

**GitHub Actions:**
- Artifacts: Available in workflow run page
- GitHub Pages: `https://your-org.github.io/DelveDeep/test-reports/`

**GitLab CI:**
- Artifacts: Available in pipeline page
- GitLab Pages: `https://your-org.gitlab.io/DelveDeep/`

**Jenkins:**
- HTML Publisher: Available in build page
- Artifacts: Available in build artifacts

### Report Contents

Each test run generates:
- **JUnit XML**: Machine-readable test results
- **HTML Report**: Human-readable test results with charts
- **Coverage Report**: Code coverage analysis
- **Summary**: Quick overview of test results

## Troubleshooting

### Build Fails to Start

**Issue:** Runner/agent not available

**Solution:**
- Verify runners/agents are online
- Check runner/agent labels match job requirements
- Ensure Unreal Engine is installed on agents

### Tests Timeout

**Issue:** Tests take too long to complete

**Solution:**
- Increase timeout in CI configuration
- Optimize test execution (parallel execution)
- Check for hanging tests

### Out of Disk Space

**Issue:** Build artifacts fill disk

**Solution:**
- Configure artifact retention policies
- Clean up old builds regularly
- Increase disk space on agents

### Unreal Engine Not Found

**Issue:** UE_ROOT path incorrect

**Solution:**
- Verify Unreal Engine installation path
- Update environment variables in CI configuration
- Check path format (Windows vs Linux)

### Test Results Not Published

**Issue:** JUnit XML not found

**Solution:**
- Verify test output path is correct
- Check RunTests script generates XML
- Ensure artifacts are uploaded before publishing

## Performance Optimization

### Parallel Execution

All configurations support parallel test execution:
- Unit tests run in parallel with integration tests
- Windows and Linux builds run in parallel
- Multiple test suites run concurrently

### Caching

**GitHub Actions:**
```yaml
- uses: actions/cache@v3
  with:
    path: |
      Intermediate/
      Saved/
    key: ${{ runner.os }}-ue-${{ hashFiles('*.uproject') }}
```

**GitLab CI:**
```yaml
cache:
  paths:
    - Intermediate/
    - Saved/
```

**Jenkins:**
Use workspace caching or shared directories.

### Incremental Builds

Configure incremental builds to speed up compilation:
- Only rebuild changed modules
- Reuse previous build artifacts
- Skip unnecessary steps

## Best Practices

1. **Run tests on every commit** to catch issues early
2. **Use parallel execution** to minimize pipeline duration
3. **Set up notifications** for build failures
4. **Archive test reports** for historical analysis
5. **Monitor test execution time** and optimize slow tests
6. **Use code coverage** to identify untested code
7. **Configure automatic retries** for flaky tests
8. **Separate fast and slow tests** for quick feedback
9. **Use build caching** to speed up builds
10. **Document CI/CD setup** for team members

## Maintenance

### Regular Tasks

- **Weekly:** Review test execution times
- **Monthly:** Clean up old artifacts
- **Quarterly:** Update CI/CD configurations
- **Annually:** Review and optimize pipeline

### Monitoring

Monitor these metrics:
- Build success rate
- Test pass rate
- Average build duration
- Code coverage trends
- Flaky test frequency

## Support

For issues or questions:
- Check CI/CD platform documentation
- Review Unreal Engine automation documentation
- Consult team lead or DevOps engineer
- File issue in project repository

## References

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [GitLab CI/CD Documentation](https://docs.gitlab.com/ee/ci/)
- [Jenkins Pipeline Documentation](https://www.jenkins.io/doc/book/pipeline/)
- [Unreal Engine Automation System](https://docs.unrealengine.com/5.6/en-US/automation-system-in-unreal-engine/)
