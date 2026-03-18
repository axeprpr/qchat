#pragma once

#include <QObject>
#include <QProcess>
#include <QJsonArray>
#include <QStringList>
#include <QTimer>

class SandboxManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)

public:
    explicit SandboxManager(QObject *parent = nullptr);
    ~SandboxManager();

    bool isRunning() const { return m_process && m_process->state() != QProcess::NotRunning; }

    Q_INVOKABLE void executeScript(const QString &script, const QString &language);
    Q_INVOKABLE void executeSearch(const QStringList &queries);
    Q_INVOKABLE void cancelExecution();

signals:
    void outputReceived(const QString &output);
    void executionFinished(const QString &result);
    void executionError(const QString &error);
    void searchResults(const QJsonArray &results);
    void isRunningChanged();

private slots:
    void onProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onTimeout();

private:
    QProcess *m_process = nullptr;
    QTimer *m_timeout = nullptr;
    QString m_accumulatedOutput;
    static constexpr int kTimeoutMs = 30000;
};
