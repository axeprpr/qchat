#include "SandboxManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QStandardPaths>
#include <QTemporaryFile>

SandboxManager::SandboxManager(QObject *parent) : QObject(parent) {
    m_timeout = new QTimer(this);
    m_timeout->setSingleShot(true);
    m_timeout->setInterval(kTimeoutMs);
    connect(m_timeout, &QTimer::timeout, this, &SandboxManager::onTimeout);
}

SandboxManager::~SandboxManager() {
    cancelExecution();
}

void SandboxManager::executeScript(const QString &script, const QString &language) {
    cancelExecution();
    m_accumulatedOutput.clear();

    // Write script to temp file
    QString suffix = (language == "python") ? ".py" : (language == "js" ? ".js" : ".sh");
    QString tmpPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                      + "/qchat_sandbox_XXXXXX" + suffix;
    QTemporaryFile tmpFile(tmpPath);
    tmpFile.setAutoRemove(false);
    if (!tmpFile.open()) {
        emit executionError("Failed to create temp file");
        return;
    }
    tmpFile.write(script.toUtf8());
    tmpFile.close();
    QString filePath = tmpFile.fileName();

    // Determine interpreter
    QString interpreter;
    QStringList args;
    if (language == "python") {
        interpreter = "python3";
        args << filePath;
    } else if (language == "js") {
        interpreter = "node";
        args << filePath;
    } else {
        interpreter = "bash";
        args << filePath;
    }

    m_process = new QProcess(this);
    m_process->setWorkingDirectory(
        QStandardPaths::writableLocation(QStandardPaths::TempLocation));

    // Restrict environment
    QProcessEnvironment env;
    env.insert("PATH", "/usr/local/bin:/usr/bin:/bin");
    env.insert("HOME", QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    m_process->setProcessEnvironment(env);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &SandboxManager::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &SandboxManager::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SandboxManager::onProcessFinished);

    m_process->start(interpreter, args);
    m_timeout->start();
    emit isRunningChanged();
}

void SandboxManager::executeSearch(const QStringList &queries) {
    // Build a simple search script using curl + a search API
    QString script;
    for (const QString &q : queries) {
        script += QString("echo 'QUERY: %1'\n").arg(q);
        script += QString("curl -s 'https://api.duckduckgo.com/?q=%1&format=json&no_html=1' 2>/dev/null\n")
                      .arg(QString(q).replace(" ", "+"));
    }
    executeScript(script, "bash");
}

void SandboxManager::cancelExecution() {
    m_timeout->stop();
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(1000);
        m_process->deleteLater();
        m_process = nullptr;
        emit isRunningChanged();
    }
}

void SandboxManager::onProcessOutput() {
    if (!m_process) return;
    QString out = m_process->readAllStandardOutput();
    QString err = m_process->readAllStandardError();
    if (!out.isEmpty()) {
        m_accumulatedOutput += out;
        emit outputReceived(out);
    }
    if (!err.isEmpty()) {
        emit outputReceived(err);
    }
}

void SandboxManager::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
    m_timeout->stop();
    Q_UNUSED(status)
    if (exitCode == 0) {
        emit executionFinished(m_accumulatedOutput);
    } else {
        emit executionError(QString("Process exited with code %1").arg(exitCode));
    }
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
        emit isRunningChanged();
    }
}

void SandboxManager::onTimeout() {
    emit executionError("Execution timed out");
    cancelExecution();
}
