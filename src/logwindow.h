#pragma once

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QTextEdit)

class LogWindow : public QObject
{
    Q_OBJECT

public:
    ~LogWindow();
    explicit LogWindow(QObject* parent = nullptr);

    QTextEdit* release();

private:
    bool m_released = false;
};
