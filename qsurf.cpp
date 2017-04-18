#include <iostream>
#include <string>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QIODevice>
#include <QProcess>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineSettings>
#include <QWebEngineView>

QProcess proc;
std::function<void(const std::string &)> proc_finished_cb;
QWebEngineView *view;
QClipboard *clipboard;
std::list<QAction> actions; // QAction is not copyable
std::string find_text;

#include "config.h"

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);
  QWebEngineView lview;
  view = &lview;
  clipboard = QGuiApplication::clipboard();

  view->load(QUrl("http://google.com"));
  view->show();

  // allow fullscreen
  view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled,
                                 true);
  QObject::connect(
      view->page(), &QWebEnginePage::fullScreenRequested,
      [](const QWebEngineFullScreenRequest &request) {
        const_cast<QWebEngineFullScreenRequest &>(request).accept();
        if (request.toggleOn()) {
          view->showFullScreen();
        } else {
          view->showNormal();
        }
      });

  // show favicon for pages
  QObject::connect(view, &QWebEngineView::iconChanged,
                   [](const QIcon &icon) { view->setWindowIcon(icon); });

  // web actions
  for (auto shortcut : webshortcuts) {
    auto action = view->pageAction(std::get<0>(shortcut));
    action->setShortcuts(QKeySequence::listFromString(std::get<1>(shortcut)));
    view->addAction(action);
  }

  /* set spawn infrastructure */
  // connect callback reading output for user process
  QObject::connect(&proc,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   [](int exitCode, QProcess::ExitStatus exitStatus) {
                     if (exitStatus == QProcess::NormalExit && exitCode == 0 &&
                         proc_finished_cb) {
                       std::size_t len = proc.bytesAvailable();
                       if (len < 2) {
                         return;
                       }
                       std::string output(len, '\0');
                       proc.readLine(&output[0], len);
                       proc_finished_cb(output);
                     }
                   });
  // create spawn triggering actions
  for (auto shortcut : spawnshortcuts) {
    actions.emplace_back(std::get<0>(shortcut));
    auto action = &actions.back();
    action->setShortcuts(QKeySequence::listFromString(std::get<1>(shortcut)));
    view->addAction(action);
    QObject::connect(action, &QAction::triggered, [shortcut](bool) {
      proc.start(userprocess, {std::get<0>(shortcut), view->url().toString()});
      proc_finished_cb = std::get<2>(shortcut);
    });
  }

  for (auto shortcut : generalshortcuts) {
    actions.emplace_back();
    auto action = &actions.back();
    action->setShortcuts(QKeySequence::listFromString(std::get<0>(shortcut)));
    view->addAction(action);
    QObject::connect(action, &QAction::triggered, std::get<1>(shortcut));
  }

  return app.exec();
}
