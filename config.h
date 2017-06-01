// user process that is spawned to handle actions specified in spawnshortcuts
auto userprocess = "/home/henry/.config/qsurf/usr.sh";
// this script is injected to all pages
auto scriptfile = "/home/henry/.config/qsurf/script.js";

// this action will be executed on startup, when no url is provided.
// Action is identified by it's shortcut. Assign nullptr to disable this
// functionality
auto startupaction = "Ctrl+g";

// maximum http cache size to use in bytes
const int maximum_cache_size = 1073741824 /* 1 GB */;

/* shortcuts */
std::initializer_list<std::tuple<QWebEnginePage::WebAction, const char *>>
webshortcuts = {
{QWebEnginePage::Reload, "Ctrl+r"},
{QWebEnginePage::ReloadAndBypassCache, "Ctrl+Shift+r"},
{QWebEnginePage::Forward, "Forward; Ctrl+l"},
{QWebEnginePage::Back, "Back; Ctrl+h"},
{QWebEnginePage::Stop, "Ctrl+Esc"},
};

std::initializer_list<std::tuple<const char *, const char *,
                                 std::function<void(WebView*, const std::string &)>>>
spawnshortcuts = {
{"navigate", "Ctrl+g",
 [](auto view, auto output) { if (output.empty()) return;
    view->load(QUrl::fromUserInput(output.c_str())); }},
{"find", "Ctrl+f", [](auto view, auto output) { view->findText(output.c_str()); }},
{"bookmark", "Ctrl+b", [](auto, auto) {}},
};

std::initializer_list<std::tuple<const char *, std::function<void(WebView*)>>>
generalshortcuts = {
{"Ctrl+n", [](auto view) { view->findNext(); }},
{"Ctrl+Shift+n", [](auto view) { view->findNext(QWebEnginePage::FindBackward); }},
{"Ctrl+y", [](auto view) { clipboard->setText(view->url().toString()); }},
{"Ctrl+p", [](auto view) { view->load(clipboard->text()); }},
{"Ctrl++", [](auto view) { view->setZoomFactor(view->zoomFactor() + 0.25); }},
{"Ctrl+-", [](auto view) { view->setZoomFactor(view->zoomFactor() - 0.25); }},
{"Ctrl+0", [](auto view) { view->setZoomFactor(1.0); }},
{"Esc", [](auto view) {
  if (view->isFullScreen()) {
    view->triggerPageAction(QWebEnginePage::ExitFullScreen);
  } else {
    // clear find highlights
    view->findText("");
  }
}},
};
