#include "webwidget.h"

#include "widgetdefs.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QtWebEngineWidgets/QWebEngineScript>
#include <QtWebEngineWidgets/QWebEngineScriptCollection>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineView>

QString WebWidget::PageGlobalTemp;

WebWidget::WebWidget(QString widgetName, const QJsonObject& dat, QWidget* parent)
    : QWidget(parent), m_Name(widgetName)
{
    if (m_Name.isEmpty())
    {
        throw std::invalid_argument("Widget name cannot be null");
    }

    // Copy data
    data = dat;

    // No frame/border, no taskbar button
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::SubWindow;

    webview = new QWebEngineView(this);

    QString startFilePath = QFileInfo(data[WGT_DEF_FULLPATH].toString()).canonicalPath().append("/");
    QUrl    startFile     = QUrl::fromLocalFile(startFilePath.append(data[WGT_DEF_STARTFILE].toString()));

    webview->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);

    if (data[WGT_DEF_REMOTEACCESS].toBool())
    {
        webview->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    }

    webview->load(startFile);

    // Optional background transparency
    if (data[WGT_DEF_TRANSPARENTBG].toBool())
    {
        setAttribute(Qt::WA_TranslucentBackground);
        webview->page()->setBackgroundColor(Qt::transparent);
    }

    // Overlay for catching drag and drop events
    OverlayWidget* overlay = new OverlayWidget(this);

    // Create context menu
    createContextMenuActions();
    createContextMenu();

    // Restore settings
    QSettings settings;
    restoreGeometry(settings.value(getWidgetConfigKey("geometry")).toByteArray());
    bool ontop      = settings.value(getWidgetConfigKey("alwaysOnTop")).toBool();
    m_fixedposition = settings.value(getWidgetConfigKey("fixedPosition")).toBool();

    rFixedPos->setChecked(m_fixedposition);

    if (ontop)
    {
        flags |= Qt::WindowStaysOnTopHint;
        rOnTop->setChecked(true);
    }

    // Set flags
    setWindowFlags(flags);

    // resize
    webview->resize(data[WGT_DEF_WIDTH].toInt(), data[WGT_DEF_HEIGHT].toInt());
    resize(data[WGT_DEF_WIDTH].toInt(), data[WGT_DEF_HEIGHT].toInt());

    // Create page globals
    if (PageGlobalTemp.isEmpty())
    {
        QFile file(":/Resources/pageglobals.js");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            throw std::exception("pageglobal script load failure");
        }

        QTextStream in(&file);
        PageGlobalTemp = in.readAll();
    }

    quint16 port = settings.value(QUASAR_CONFIG_PORT, QUASAR_DATA_SERVER_DEFAULT_PORT).toUInt();

    QString pageGlobals = PageGlobalTemp.arg(m_Name).arg(port);

    QWebEngineScript script;
    script.setName("PageGlobals");
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(0);
    script.setSourceCode(pageGlobals);

    webview->page()->scripts().insert(script);

    setWindowTitle(m_Name);
}

bool WebWidget::validateWidgetDefinition(const QJsonObject& dat)
{
    if (!dat.isEmpty() &&
        dat.contains(WGT_DEF_NAME) && !dat[WGT_DEF_NAME].toString().isNull() &&
        dat.contains(WGT_DEF_WIDTH) && dat[WGT_DEF_WIDTH].toInt() > 0 &&
        dat.contains(WGT_DEF_HEIGHT) && dat[WGT_DEF_HEIGHT].toInt() > 0 &&
        dat.contains(WGT_DEF_STARTFILE) && !dat[WGT_DEF_STARTFILE].toString().isNull() &&
        dat.contains(WGT_DEF_FULLPATH) && !dat[WGT_DEF_FULLPATH].toString().isNull())
    {
        return true;
    }

    return false;
}

bool WebWidget::acceptSecurityWarnings(const QJsonObject& dat)
{
    bool accept = true;

    if (dat.contains(WGT_DEF_REMOTEACCESS) && dat[WGT_DEF_REMOTEACCESS].toBool())
    {
        accept = false;

        auto reply = QMessageBox::warning(nullptr,
                                          tr("Remote Access"),
                                          tr("This widget requires remote access to external URLs. This may pose a security risk.\n\nContinue loading?"),
                                          QMessageBox::Ok | QMessageBox::Cancel);

        if (reply == QMessageBox::Ok)
        {
            accept = true;
        }
    }

    return accept;
}

QString WebWidget::getFullPath()
{
    return data[WGT_DEF_FULLPATH].toString();
}

void WebWidget::saveSettings()
{
    QSettings settings;
    settings.setValue(getWidgetConfigKey("geometry"), saveGeometry());
    settings.setValue(getWidgetConfigKey("alwaysOnTop"), rOnTop->isChecked());
    settings.setValue(getWidgetConfigKey("fixedPosition"), m_fixedposition);
}

void WebWidget::createContextMenuActions()
{
    rName   = new QAction(m_Name, this);
    QFont f = rName->font();
    f.setBold(true);
    rName->setFont(f);

    rReload = new QAction(tr("&Reload"), this);
    connect(rReload, &QAction::triggered, webview, &QWebEngineView::reload);

    rResetPos = new QAction(tr("Re&set Position"), this);
    connect(rResetPos, &QAction::triggered, [=](bool e) {
        this->move(0, 0);
    });

    rOnTop = new QAction(tr("&Always on Top"), this);
    rOnTop->setCheckable(true);
    connect(rOnTop, &QAction::triggered, this, &WebWidget::toggleOnTop);

    rFixedPos = new QAction(tr("&Fixed Position"), this);
    rFixedPos->setCheckable(true);
    connect(rFixedPos, &QAction::triggered, [=](bool enabled) {
        this->m_fixedposition = enabled;
    });

    rClose = new QAction(tr("&Close"), this);
    connect(rClose, &QAction::triggered, this, &WebWidget::close);
}

void WebWidget::createContextMenu()
{
    m_Menu = new QMenu(m_Name, this);
    m_Menu->addAction(rName);
    m_Menu->addSeparator();
    m_Menu->addAction(rReload);
    m_Menu->addAction(rResetPos);
    m_Menu->addSeparator();
    m_Menu->addAction(rOnTop);
    m_Menu->addAction(rFixedPos);
    m_Menu->addSeparator();
    m_Menu->addAction(rClose);
}

void WebWidget::mousePressEvent(QMouseEvent* evt)
{
    if (!m_fixedposition && evt->button() == Qt::LeftButton)
    {
        dragPosition = evt->globalPos() - frameGeometry().topLeft();
        evt->accept();
    }
}

void WebWidget::mouseMoveEvent(QMouseEvent* evt)
{
    if (!m_fixedposition && evt->buttons() & Qt::LeftButton)
    {
        move(evt->globalPos() - dragPosition);
        evt->accept();
    }
}

void WebWidget::contextMenuEvent(QContextMenuEvent* event)
{
    m_Menu->exec(event->globalPos());
}

void WebWidget::closeEvent(QCloseEvent* event)
{
    saveSettings();

    emit WebWidgetClosed(this);
    event->accept();
}

void WebWidget::toggleOnTop(bool ontop)
{
    auto flags = windowFlags();

    if (ontop)
    {
        flags |= Qt::WindowStaysOnTopHint;
    }
    else
    {
        flags &= ~Qt::WindowStaysOnTopHint;
    }

    setWindowFlags(flags);

    // Refresh widget
    QPoint pos = this->pos();
    move(pos);
    show();
}

QString WebWidget::getWidgetConfigKey(QString key)
{
    return m_Name + "/" + key;
}
