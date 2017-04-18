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

QClipboard *clipboard;

class WebView : public QWebEngineView {
public:
  WebView();
  virtual ~WebView() { std::cout << "destroyed" << std::endl; }

  void
  findText(const QString &subString,
           QWebEnginePage::FindFlags options = QWebEnginePage::FindFlags()) {
    find_text = subString;
    QWebEngineView::findText(find_text, options);
  }
  void
  findNext(QWebEnginePage::FindFlags options = QWebEnginePage::FindFlags()) {
    QWebEngineView::findText(find_text, options);
  }

private:
  QWebEngineView *createWindow(QWebEnginePage::WebWindowType) {
    std::cout << "window requested" << std::endl;
    return new WebView();
  }

  QProcess proc;
  std::function<void(const std::string &)> proc_finished_cb;
  std::list<QAction> actions; // QAction is not copyable
  QString find_text;
};

#include "config.h"

WebView::WebView() {
  std::cout << "created" << std::endl;
  setAttribute(Qt::WA_DeleteOnClose, true);
  show();

  // allow fullscreen
  settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
  QObject::connect(
      page(), &QWebEnginePage::fullScreenRequested,
      [this](const QWebEngineFullScreenRequest &request) {
        const_cast<QWebEngineFullScreenRequest &>(request).accept();
        if (request.toggleOn()) {
          showFullScreen();
        } else {
          showNormal();
        }
      });
  // show favicon for pages
  QObject::connect(this, &QWebEngineView::iconChanged,
                   [this](const QIcon &icon) { setWindowIcon(icon); });
  // connect callback reading output for user process
  QObject::connect(&proc,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   [this](int exitCode, QProcess::ExitStatus exitStatus) {
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

  /* shortcuts */
  // web actions
  for (auto shortcut : webshortcuts) {
    auto action = pageAction(std::get<0>(shortcut));
    action->setShortcuts(QKeySequence::listFromString(std::get<1>(shortcut)));
    addAction(action);
  }
  // create spawn triggering actions
  for (auto shortcut : spawnshortcuts) {
    actions.emplace_back(std::get<0>(shortcut));
    auto action = &actions.back();
    action->setShortcuts(QKeySequence::listFromString(std::get<1>(shortcut)));
    addAction(action);
    QObject::connect(action, &QAction::triggered, [shortcut, this](bool) {
      proc.start(userprocess, {std::get<0>(shortcut), url().toString()});
      proc_finished_cb = [this, &shortcut](auto output) {
        std::get<2>(shortcut)(this, output);
      };
    });
  }
  // general actions
  for (auto shortcut : generalshortcuts) {
    actions.emplace_back();
    auto action = &actions.back();
    action->setShortcuts(QKeySequence::listFromString(std::get<0>(shortcut)));
    addAction(action);
    QObject::connect(action, &QAction::triggered,
                     [this, shortcut]() { std::get<1>(shortcut)(this); });
  }
}

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);
  clipboard = QGuiApplication::clipboard();

  new WebView();

  return app.exec();
}
