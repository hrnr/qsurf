#include <iostream>
#include <string>

#include <QAction>
#include <QApplication>
#include <QIODevice>
#include <QProcess>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineSettings>
#include <QWebEngineView>

#include "config.h"

QProcess proc;
std::function<void(const std::string &)> proc_finished_cb;

void spawn(QWebEngineView &view) {
  proc.start(userprocess, {"navigate", view.url().toString()});
  proc_finished_cb = [&view](auto output) {
    auto url = QUrl::fromUserInput(QString(output.c_str()));
    std::cout << url.toString().toStdString() << std::endl;
    view.load(url);
  };
}

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);

  QWebEngineView view;
  view.load(QUrl("http://google.com"));
  view.show();

  // allow fullscreen
  view.settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled,
                                true);
  QObject::connect(
      view.page(), &QWebEnginePage::fullScreenRequested,
      [&view](const QWebEngineFullScreenRequest &request) {
        const_cast<QWebEngineFullScreenRequest &>(request).accept();
        if (request.toggleOn()) {
          view.showFullScreen();
        } else {
          view.showNormal();
        }
      });
  auto exitAction = view.pageAction(QWebEnginePage::ExitFullScreen);
  exitAction->setShortcut(QKeySequence(QKeySequence::Cancel));
  view.addAction(exitAction);

  // show favicon for pages
  QObject::connect(&view, &QWebEngineView::iconChanged,
                   [&view](const QIcon &icon) { view.setWindowIcon(icon); });

  // go back
  auto action = view.pageAction(QWebEnginePage::Back);
  action->setShortcut(QKeySequence("Ctrl+H"));
  view.addAction(action);

  /* set spawn infrastructure */
  // connect callback reading output for user process
  QObject::connect(&proc,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   [](int exitCode, QProcess::ExitStatus exitStatus) {
                     if (exitStatus == QProcess::NormalExit && exitCode == 0 &&
                         proc_finished_cb) {
                       std::size_t len = proc.bytesAvailable();
                       std::string output(len, '\0');
                       proc.readLine(&output[0], len);
                       proc_finished_cb(output);
                     }
                   });
  // create spawn triggering actions
  QAction spawnact("spawn");
  spawnact.setShortcut(QKeySequence("Ctrl+G"));
  view.addAction(&spawnact);
  QObject::connect(&spawnact, &QAction::triggered,
                   [&view](bool) { spawn(view); });

  return app.exec();
}
