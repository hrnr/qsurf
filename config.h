auto userprocess = "/home/henry/.config/qsurf/usr.sh";

std::initializer_list<std::tuple<QWebEnginePage::WebAction, const char *>>
webshortcuts = {
{QWebEnginePage::Reload, "Ctrl+r"},
{QWebEnginePage::ReloadAndBypassCache, "Ctrl+Shift+r"},
{QWebEnginePage::Forward, "Forward; Ctrl+l"},
{QWebEnginePage::Back, "Back; Ctrl+h"},
{QWebEnginePage::Stop, "Ctrl+Esc"},
};

std::initializer_list<std::tuple<const char *, const char *,
                                 std::function<void(const std::string &)>>>
spawnshortcuts = {
{"navigate", "Ctrl+g",
 [](auto output) { view->load(QUrl::fromUserInput(output.c_str())); }},
{"find", "Ctrl+f", [](auto output) { find_text = output, view->findText(output.c_str());}},
{"bookmark", "Ctrl+b", [](auto) {}},
};

std::initializer_list<std::tuple<const char *, std::function<void()>>>
generalshortcuts = {
{"Ctrl+n", []() { view->findText(find_text.c_str()); }},
{"Ctrl+Shift+n", []() { view->findText(find_text.c_str(), QWebEnginePage::FindBackward); }},
{"Ctrl+y", []() { clipboard->setText(view->url().toString()); }},
{"Ctrl+p", []() { view->load(clipboard->text()); }},
{"Esc", []() {
  if (view->isFullScreen()) {
    view->triggerPageAction(QWebEnginePage::ExitFullScreen);
  } else {
    find_text = "", view->findText("");
  }
}},
};
