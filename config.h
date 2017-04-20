// user process that is spawned to handle actions specified in spawnshortcuts
auto userprocess = "/home/henry/.config/qsurf/usr.sh";

// this action will be executed on startup, when no url is provided.
// Action is identified by it's shortcut. Assign nullptr to disable this
// functionality
const char* startupaction = "Ctrl+g";

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
 [](auto view, auto output) { view->load(QUrl::fromUserInput(output.c_str())); }},
{"find", "Ctrl+f", [](auto view, auto output) { view->findText(output.c_str()); }},
{"bookmark", "Ctrl+b", [](auto, auto) {}},
};

std::initializer_list<std::tuple<const char *, std::function<void(WebView*)>>>
generalshortcuts = {
{"Ctrl+n", [](auto view) { view->findNext(); }},
{"Ctrl+Shift+n", [](auto view) { view->findNext(QWebEnginePage::FindBackward); }},
{"Ctrl+y", [](auto view) { clipboard->setText(view->url().toString()); }},
{"Ctrl+p", [](auto view) { view->load(clipboard->text()); }},
{"Esc", [](auto view) {
  if (view->isFullScreen()) {
    view->triggerPageAction(QWebEnginePage::ExitFullScreen);
  } else {
    // clear find highlights
    view->findText("");
  }
}},
};
