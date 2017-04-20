#include <iostream>
#include <string>

#include <QAction>
#include <QApplication>
#include <QAuthenticator>
#include <QClipboard>
#include <QIODevice>
#include <QProcess>
#include <QString>
#include <QWebEngineDownloadItem>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

QClipboard *clipboard;

class WebView : public QWebEngineView {
public:
  WebView();

  void
  findText(const QString &subString,
           QWebEnginePage::FindFlags options = QWebEnginePage::FindFlags()) {
    find_text_ = subString;
    QWebEngineView::findText(find_text_, options);
  }
  void
  findNext(QWebEnginePage::FindFlags options = QWebEnginePage::FindFlags()) {
    QWebEngineView::findText(find_text_, options);
  }
  void setTitle() {
    QString security = tls_ ? QStringLiteral("T ") : QStringLiteral("- ");
    QString loading = progress_ > 0 && progress_ < 100
                          ? QString("[%1%] ").arg(progress_)
                          : QStringLiteral("");

    setWindowTitle(QString("%1%2%3").arg(security, loading, title()));
  }

private:
  QWebEngineView *createWindow(QWebEnginePage::WebWindowType) {
    return new WebView();
  }

  QProcess proc_;
  std::function<void(const std::string &)> proc_finished_cb_;
  std::list<QAction> actions_; // QAction is not copyable
  QString find_text_;
  int progress_ = 0;
  bool tls_ = false;
};

#include "config.h"

WebView::WebView() {
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
  QObject::connect(&proc_,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   [this](int exitCode, QProcess::ExitStatus exitStatus) {
                     if (exitStatus == QProcess::NormalExit && exitCode == 0 &&
                         proc_finished_cb_) {
                       std::size_t len = proc_.bytesAvailable();
                       std::string output(len, '\0');
                       proc_.readLine(&output[0], len);
                       proc_finished_cb_(output);
                     }
                   });

  /* shortcuts */
  // create spawn triggering actions
  for (auto shortcut : spawnshortcuts) {
    actions_.emplace_back(std::get<0>(shortcut));
    auto action = &actions_.back();
    action->setShortcuts(QKeySequence::listFromString(std::get<1>(shortcut)));
    addAction(action);
    QObject::connect(action, &QAction::triggered, [shortcut, this](bool) {
      proc_.start(userprocess, {std::get<0>(shortcut), url().toString()});
      proc_finished_cb_ = [this, &shortcut](auto output) {
        std::get<2>(shortcut)(this, output);
      };
    });
  }
  // web actions
  for (auto shortcut : webshortcuts) {
    auto action = pageAction(std::get<0>(shortcut));
    action->setShortcuts(QKeySequence::listFromString(std::get<1>(shortcut)));
    addAction(action);
  }
  // general actions
  for (auto shortcut : generalshortcuts) {
    actions_.emplace_back();
    auto action = &actions_.back();
    action->setShortcuts(QKeySequence::listFromString(std::get<0>(shortcut)));
    addAction(action);
    QObject::connect(action, &QAction::triggered,
                     [this, shortcut]() { std::get<1>(shortcut)(this); });
  }

  // setting window title
  QObject::connect(this, &QWebEngineView::titleChanged,
                   [this](const QString &) { setTitle(); });
  QObject::connect(this, &QWebEngineView::loadProgress,
                   [this](int p) { progress_ = p, setTitle(); });
  QObject::connect(this, &QWebEngineView::loadStarted,
                   [this]() { progress_ = 1, setTitle(); });
  QObject::connect(this, &QWebEngineView::urlChanged, [this](const QUrl &url) {
    tls_ = (url.scheme() == "https"), setTitle();
  });
  // managing downloads
  QObject::connect(
      page()->profile(), &QWebEngineProfile::downloadRequested,
      [this](QWebEngineDownloadItem *download) {
        proc_.start(userprocess, {"download", download->url().toString()});
        proc_finished_cb_ = [download](auto output) {
          if (output.size() > 1 && output[0] == 'y') {
            download->accept();
          }
        };
        // wait so that download struct will not be destroyed
        proc_.waitForFinished(-1);
      });
  // response to authetication
  QObject::connect(
      page(), &QWebEnginePage::authenticationRequired,
      [this](const QUrl &requestUrl, QAuthenticator *authenticator) {
        proc_.start(userprocess, {"authentication", requestUrl.toString()});
        proc_finished_cb_ = [authenticator](auto output) {
          auto output_list = QString::fromStdString(output).split(";");
          if (output_list.size() < 2) {
            return;
          }
          authenticator->setUser(output_list[0]);
          authenticator->setPassword(output_list[1]);
        };
        proc_.waitForFinished(-1);
      });
}

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);
  clipboard = QGuiApplication::clipboard();

  auto view = new WebView();

  // either load url provided in argument or spawn startup action
  if (argc > 1) {
    view->load(QUrl::fromUserInput(argv[1]));
  } else if (startupaction) {
    for (auto action : view->actions()) {
      if (action->shortcut() == QKeySequence(startupaction)) {
        action->trigger();
        break;
      }
    }
  }

  return app.exec();
}
