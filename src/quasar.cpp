#include "quasar.h"

#include "preproc.h"
#include "version.h"

#include "configdialog.h"
#include "dataservices.h"
#include "logwindow.h"
#include "webwidget.h"
#include "widgetdefs.h"
#include "widgetregistry.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>

Quasar::Quasar(LogWindow* log, DataServices* s, QWidget* parent)
    : QMainWindow(parent), logWindow(log), service(s)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        throw std::runtime_error("System Tray is not supported on the current desktop manager");
    }

    if (nullptr == logWindow)
    {
        throw std::invalid_argument("Invalid LogWindow");
    }

    if (nullptr == service)
    {
        throw std::invalid_argument("Invalid DataServices");
    }

    logWindow->setParent(this);
    service->setParent(this);

    ui.setupUi(this);

    // Setup system tray
    createActions();
    createTrayIcon();

    // Setup icon
    QIcon icon(":/Resources/q.png");
    setWindowIcon(icon);
    trayIcon->setIcon(icon);

    trayIcon->show();

    // Setup log widget
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(logWindow->release());

    ui.centralWidget->setLayout(layout);

    resize(800, 400);
}

void Quasar::openWebWidget()
{
    QSettings settings;
    QString   lastpath = settings.value(QUASAR_CONFIG_LASTPATH, QDir::currentPath()).toString();

    QString fname = QFileDialog::getOpenFileName(this, tr("Load Widget"), lastpath, tr("Widget Definitions (*.json)"));

    if (!fname.isNull())
    {
        QFileInfo info(fname);
        settings.setValue(QUASAR_CONFIG_LASTPATH, info.canonicalPath());

        service->getRegistry()->loadWebWidget(fname);
    }
}

void Quasar::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Context:
        {
            // Regenerate widget list menu
            widgetListMenu->clear();

            WidgetMapType& widgets = service->getRegistry()->getWidgets();

            for (auto& w : widgets)
            {
                widgetListMenu->addMenu(w.second->getMenu());
            }
            break;
        }

        case QSystemTrayIcon::DoubleClick:
        {
            openWebWidget();
            break;
        }
    }
}

void Quasar::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(loadAction);
    trayIconMenu->addMenu(widgetListMenu);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addAction(logAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addAction(aboutQtAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, &QSystemTrayIcon::activated, this, &Quasar::trayIconActivated);
}

void Quasar::createActions()
{
    loadAction = new QAction(tr("&Load"), this);
    connect(loadAction, &QAction::triggered, this, &Quasar::openWebWidget);

    widgetListMenu = new QMenu(tr("Widgets"), this);

    settingsAction = new QAction(tr("&Settings"), this);
    connect(settingsAction, &QAction::triggered, [=] {
        ConfigDialog dialog(service);
        dialog.exec();
    });

    logAction = new QAction(tr("L&og"), this);
    connect(logAction, &QAction::triggered, this, &QWidget::showNormal);

    aboutAction = new QAction(tr("&About Quasar"), this);

    connect(aboutAction, &QAction::triggered, [=](bool checked) {
        static QString aboutMsg =
            "Quasar " GIT_VER_STRING "<br/>"
#ifndef NDEBUG
            "DEBUG BUILD<br/>"
#endif
            "<br/>"
            "Compiled on: " __DATE__ "<br/>"
            "Compiler: " COMPILER_STRING "<br/>"
            "Qt version: " QT_VERSION_STR "<br/>"
            "<br/>"
            "Licensed under GPLv3<br/>"
            "Source code available at <a href='https://github.com/r52/quasar'>GitHub</a>";

        QMessageBox::about(this, tr("About Quasar"), aboutMsg);
    });

    aboutQtAction = new QAction(tr("About &Qt"), this);
    connect(aboutQtAction, &QAction::triggered, [=](bool checked) {
        QMessageBox::aboutQt(this, "About Qt");
    });

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void Quasar::closeEvent(QCloseEvent* event)
{
#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible())
    {
        return;
    }
#endif
    if (trayIcon->isVisible())
    {
        hide();
        event->ignore();
    }
}
