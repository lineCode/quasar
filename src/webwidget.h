#pragma once

#include <QWidget>
#include <QtGui>
#include <QtWebEngineWidgets/QWebEngineView>

QT_FORWARD_DECLARE_CLASS(QMenu);

// From https://stackoverflow.com/questions/19362455/dark-transparent-layer-over-a-qmainwindow-in-qt
class OverlayWidget : public QWidget
{
    void newParent()
    {
        if (!parent())
            return;
        parent()->installEventFilter(this);
        raise();
    }

public:
    explicit OverlayWidget(QWidget* parent = {})
        : QWidget{ parent }
    {
        setAttribute(Qt::WA_NoSystemBackground);
        newParent();
    }

protected:
    //! Catches resize and child events from the parent widget
    bool eventFilter(QObject* obj, QEvent* ev) override
    {
        if (obj == parent())
        {
            if (ev->type() == QEvent::Resize)
                resize(static_cast<QResizeEvent*>(ev)->size());
            else if (ev->type() == QEvent::ChildAdded)
                raise();
        }
        return QWidget::eventFilter(obj, ev);
    }
    //! Tracks parent widget changes
    bool event(QEvent* ev) override
    {
        if (ev->type() == QEvent::ParentAboutToChange)
        {
            if (parent())
                parent()->removeEventFilter(this);
        }
        else if (ev->type() == QEvent::ParentChange)
            newParent();
        return QWidget::event(ev);
    }
};

class QuasarWebView : public QWebEngineView
{
public:
    QuasarWebView(QWidget* parent = Q_NULLPTR)
        : QWebEngineView{ parent }
    {
    }

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) override
    {
        // Block default context menu and send to parent if exist
        if (parent())
        {
            parent()->event(event);
        }

        event->accept();
    }
};

class QuasarWebPage : public QWebEnginePage
{
public:
    QuasarWebPage(QObject* parent = Q_NULLPTR)
        : QWebEnginePage{ parent }
    {
    }

protected:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);
};

class WebWidget : public QWidget
{
    friend class WidgetRegistry;

    Q_OBJECT

public:
    ~WebWidget();

    static bool validateWidgetDefinition(const QJsonObject& dat);
    static bool acceptSecurityWarnings(const QJsonObject& dat);

    QJsonObject getData() { return data; }
    QString     getName() { return m_Name; }
    QMenu*      getMenu() { return m_Menu; }

    QString getFullPath();

    void saveSettings();

signals:
    void WebWidgetClosed(WebWidget* widget);

protected:
    void createContextMenuActions();
    void createContextMenu();

    // Overrides
    virtual void mousePressEvent(QMouseEvent* evt) override;
    virtual void mouseMoveEvent(QMouseEvent* evt) override;

protected slots:
    void toggleOnTop(bool ontop);

private:
    explicit WebWidget(QString widgetName, const QJsonObject& dat, QWidget* parent = Q_NULLPTR);
    WebWidget(const WebWidget&) = delete;
    WebWidget& operator=(const WebWidget&) = delete;

    QString getSettingKey(QString key);

    static QString PageGlobalTemp;

    bool m_fixedposition = false;

    QString m_Name;

    // Web engine widget
    QuasarWebView* webview;

    // Widget overlay
    OverlayWidget* overlay;

    // Widget data
    QJsonObject data;

    // Drag and drop pos
    QPoint dragPosition;

    // Menu/actions
    QMenu*   m_Menu;
    QAction* rName;
    QAction* rReload;
    QAction* rResetPos;
    QAction* rOnTop;
    QAction* rFixedPos;
    QAction* rClickable;
    QAction* rClose;
};
