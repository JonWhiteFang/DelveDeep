// DelveDeep Jenkins Pipeline
// Automated testing for Unreal Engine 5.6 project

pipeline {
    agent none
    
    environment {
        UE_VERSION = '5.6'
        PROJECT_NAME = 'DelveDeep'
        UE_ROOT_WINDOWS = 'C:\\UnrealEngine'
        UE_ROOT_LINUX = '/opt/UnrealEngine'
    }
    
    options {
        buildDiscarder(logRotator(numToKeepStr: '30', artifactNumToKeepStr: '10'))
        timestamps()
        timeout(time: 2, unit: 'HOURS')
    }
    
    stages {
        stage('Checkout') {
            agent any
            steps {
                checkout scm
                script {
                    env.GIT_COMMIT_SHORT = sh(
                        script: "git rev-parse --short HEAD",
                        returnStdout: true
                    ).trim()
                }
            }
        }
        
        stage('Build') {
            parallel {
                stage('Build Windows') {
                    agent {
                        label 'windows && unreal'
                    }
                    steps {
                        echo "Building ${PROJECT_NAME} on Windows..."
                        bat """
                            set UE_ROOT=${UE_ROOT_WINDOWS}
                            "%UE_ROOT%\\Engine\\Build\\BatchFiles\\Build.bat" DelveDeepEditor Win64 Development "%WORKSPACE%\\DelveDeep.uproject" -waitmutex
                        """
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: 'Binaries/**/*', fingerprint: true
                        }
                    }
                }
                
                stage('Build Linux') {
                    agent {
                        label 'linux && unreal'
                    }
                    steps {
                        echo "Building ${PROJECT_NAME} on Linux..."
                        sh """
                            export UE_ROOT=${UE_ROOT_LINUX}
                            \$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh DelveDeepEditor Linux Development "\$WORKSPACE/DelveDeep.uproject" -waitmutex
                        """
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: 'Binaries/**/*', fingerprint: true
                        }
                    }
                }
            }
        }
        
        stage('Test') {
            parallel {
                stage('Unit Tests - Windows') {
                    agent {
                        label 'windows && unreal'
                    }
                    steps {
                        echo "Running unit tests on Windows..."
                        bat """
                            .\\RunTests.bat -filter=Product -output=TestResults\\unit-windows
                        """
                    }
                    post {
                        always {
                            junit 'TestResults/unit-windows/*.xml'
                            archiveArtifacts artifacts: 'TestResults/unit-windows/**/*', allowEmptyArchive: true
                        }
                    }
                }
                
                stage('Unit Tests - Linux') {
                    agent {
                        label 'linux && unreal'
                    }
                    steps {
                        echo "Running unit tests on Linux..."
                        sh """
                            ./RunTests.sh --filter=Product --output=TestResults/unit-linux
                        """
                    }
                    post {
                        always {
                            junit 'TestResults/unit-linux/*.xml'
                            archiveArtifacts artifacts: 'TestResults/unit-linux/**/*', allowEmptyArchive: true
                        }
                    }
                }
                
                stage('Integration Tests - Windows') {
                    agent {
                        label 'windows && unreal'
                    }
                    steps {
                        echo "Running integration tests on Windows..."
                        bat """
                            .\\RunTests.bat -filter=Integration -output=TestResults\\integration-windows
                        """
                    }
                    post {
                        always {
                            junit 'TestResults/integration-windows/*.xml'
                            archiveArtifacts artifacts: 'TestResults/integration-windows/**/*', allowEmptyArchive: true
                        }
                    }
                }
                
                stage('Integration Tests - Linux') {
                    agent {
                        label 'linux && unreal'
                    }
                    steps {
                        echo "Running integration tests on Linux..."
                        sh """
                            ./RunTests.sh --filter=Integration --output=TestResults/integration-linux
                        """
                    }
                    post {
                        always {
                            junit 'TestResults/integration-linux/*.xml'
                            archiveArtifacts artifacts: 'TestResults/integration-linux/**/*', allowEmptyArchive: true
                        }
                    }
                }
                
                stage('Performance Tests - Windows') {
                    agent {
                        label 'windows && unreal'
                    }
                    steps {
                        echo "Running performance tests on Windows..."
                        bat """
                            .\\RunTests.bat -filter=Perf -output=TestResults\\performance-windows
                        """
                    }
                    post {
                        always {
                            junit 'TestResults/performance-windows/*.xml'
                            archiveArtifacts artifacts: 'TestResults/performance-windows/**/*', allowEmptyArchive: true
                        }
                        failure {
                            echo "Performance tests failed - this is non-blocking"
                        }
                    }
                }
                
                stage('Performance Tests - Linux') {
                    agent {
                        label 'linux && unreal'
                    }
                    steps {
                        echo "Running performance tests on Linux..."
                        sh """
                            ./RunTests.sh --filter=Perf --output=TestResults/performance-linux
                        """
                    }
                    post {
                        always {
                            junit 'TestResults/performance-linux/*.xml'
                            archiveArtifacts artifacts: 'TestResults/performance-linux/**/*', allowEmptyArchive: true
                        }
                        failure {
                            echo "Performance tests failed - this is non-blocking"
                        }
                    }
                }
            }
        }
        
        stage('Code Coverage') {
            agent {
                label 'linux && unreal'
            }
            steps {
                echo "Generating code coverage report..."
                sh """
                    ./Scripts/generate-coverage.sh
                """
            }
            post {
                always {
                    publishHTML([
                        allowMissing: false,
                        alwaysLinkToLastBuild: true,
                        keepAll: true,
                        reportDir: 'TestResults/coverage',
                        reportFiles: 'index.html',
                        reportName: 'Code Coverage Report'
                    ])
                    archiveArtifacts artifacts: 'TestResults/coverage/**/*', allowEmptyArchive: true
                }
            }
        }
        
        stage('Generate Report') {
            agent {
                label 'linux'
            }
            steps {
                echo "Generating consolidated test report..."
                sh """
                    ./Scripts/generate-report.sh
                """
            }
            post {
                always {
                    publishHTML([
                        allowMissing: false,
                        alwaysLinkToLastBuild: true,
                        keepAll: true,
                        reportDir: 'TestResults',
                        reportFiles: 'consolidated-report.html',
                        reportName: 'Test Report'
                    ])
                    archiveArtifacts artifacts: 'TestResults/consolidated-report.html', fingerprint: true
                }
            }
        }
    }
    
    post {
        always {
            node('linux') {
                echo "Pipeline completed"
                
                // Send email notification
                emailext(
                    subject: "${PROJECT_NAME} - Build ${currentBuild.result}: ${env.BUILD_NUMBER}",
                    body: """
                        <h2>${PROJECT_NAME} Build ${env.BUILD_NUMBER}</h2>
                        <p><strong>Status:</strong> ${currentBuild.result}</p>
                        <p><strong>Branch:</strong> ${env.BRANCH_NAME}</p>
                        <p><strong>Commit:</strong> ${env.GIT_COMMIT_SHORT}</p>
                        <p><strong>Duration:</strong> ${currentBuild.durationString}</p>
                        <p><a href="${env.BUILD_URL}">View Build</a></p>
                        <p><a href="${env.BUILD_URL}Test_20Report/">View Test Report</a></p>
                        <p><a href="${env.BUILD_URL}Code_20Coverage_20Report/">View Coverage Report</a></p>
                    """,
                    to: '${DEFAULT_RECIPIENTS}',
                    mimeType: 'text/html'
                )
            }
        }
        
        success {
            node('linux') {
                echo "All tests passed!"
                
                // Send Slack notification
                slackSend(
                    color: 'good',
                    message: "✅ ${PROJECT_NAME} - Build ${env.BUILD_NUMBER} succeeded\nBranch: ${env.BRANCH_NAME}\nCommit: ${env.GIT_COMMIT_SHORT}"
                )
            }
        }
        
        failure {
            node('linux') {
                echo "Tests failed!"
                
                // Send Slack notification
                slackSend(
                    color: 'danger',
                    message: "❌ ${PROJECT_NAME} - Build ${env.BUILD_NUMBER} failed\nBranch: ${env.BRANCH_NAME}\nCommit: ${env.GIT_COMMIT_SHORT}\n<${env.BUILD_URL}|View Build>"
                )
            }
        }
        
        unstable {
            node('linux') {
                echo "Build is unstable"
                
                // Send Slack notification
                slackSend(
                    color: 'warning',
                    message: "⚠️ ${PROJECT_NAME} - Build ${env.BUILD_NUMBER} is unstable\nBranch: ${env.BRANCH_NAME}\nCommit: ${env.GIT_COMMIT_SHORT}"
                )
            }
        }
    }
}
