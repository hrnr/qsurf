#include <iostream>

#include <QApplication>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include <QAction>

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);

  QWebEngineView view;
  view.setUrl(QUrl(QStringLiteral("http://google.com")));
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

  return app.exec();
}
