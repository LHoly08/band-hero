#include "ui/settings/InstrumentSettings.hpp"
#include <algorithm>
#include "ui/Theme.hpp"

namespace bh {
namespace {
constexpr Rectangle NewButton{620, 235, 330, 60};
constexpr Rectangle CreateButton{1020, 665, 300, 60};
constexpr Rectangle CancelButton{1350, 665, 300, 60};
constexpr Rectangle SaveButton{1020, 850, 300, 58};
constexpr Rectangle RevertButton{1450, 850, 280, 58};
constexpr Rectangle DeleteButton{1540, 378, 190, 48};
constexpr Rectangle ConfirmDelete{930, 610, 300, 54};
constexpr Rectangle CancelDelete{1290, 610, 300, 54};
constexpr int PageSize = 7;
const char *kindName(CustomKind kind) {
  switch (kind) {
  case CustomKind::Sections: return "Custom 1 - Sections";
  case CustomKind::Bits: return "Custom 2 - Bits";
  case CustomKind::Script: return "Custom 3 - Lua functions";
  }
  return "";
}
void message(const std::string &value, Vector2 position, float limit) {
  std::string line;
  float y = position.y;
  for (char c : value) {
    if (c == '\n' || line.size() >= static_cast<std::size_t>(limit)) {
      settings_ui::text(line, {position.x, y}, 23, theme::Error);
      y += 28;
      line.clear();
      if (y > position.y + 56) break;
    }
    if (c != '\n') line += c;
  }
  settings_ui::text(line, {position.x, y}, 23, theme::Error);
}
}
void InstrumentSettings::onEnter() {
  m_instruments = CustomInstrumentStore::list();
  m_selected = -1;
  m_page = 0;
  m_creating = false;
  m_dirty = false;
  m_saved = false;
  m_confirmDelete = false;
  m_error.clear();
  if (!m_instruments.empty()) select(0);
}
void InstrumentSettings::select(int index) {
  m_selected = index;
  m_creating = false;
  m_function = 0;
  m_dirty = false;
  m_saved = false;
  m_confirmDelete = false;
  m_name.value = m_instruments[index].name;
  m_script.setText(m_instruments[index].functions[0]);
  m_error = m_instruments[index].error;
  reset();
}
void InstrumentSettings::reset() { m_name.blur(); m_script.blur(); }
void InstrumentSettings::draw() const {
  using namespace settings_ui;
  text("Custom Instruments", {620, 150}, 48);
  card({600, 215, 370, 685});
  card({990, 235, 780, 745});
  choice(NewButton, "+ New instrument", m_creating);
  const int count = static_cast<int>(m_instruments.size());
  for (int row = 0; row < PageSize; ++row) {
    const int index = m_page * PageSize + row;
    if (index >= count) break;
    std::string name = m_instruments[index].name;
    if (name.size() > 20) name = name.substr(0, 17) + "...";
    choice({620, 320 + row * 70.f, 330, 60}, name, index == m_selected && !m_creating);
  }
  if (count > PageSize) {
    choice({620, 830, 70, 55}, "<");
    text(std::to_string(m_page + 1) + " / " + std::to_string((count + PageSize - 1) / PageSize), {730, 845}, 24);
    choice({880, 830, 70, 55}, ">");
  }
  if (!m_creating && m_selected < 0) {
    text("No custom instruments yet.", {1020, 340}, 32);
    text("Create one with a name and type to get started.", {1020, 395}, 24, theme::MutedText);
    return;
  }
  text("Name", {1020, 265}, 28);
  m_name.draw();
  if (m_creating) {
    text("Choose a type", {1020, 415}, 28);
    for (int i = 1; i <= 3; ++i)
      choice({1020, 465 + (i - 1) * 62.f, 710, 52}, kindName(static_cast<CustomKind>(i)), m_newType == i);
    choice(CreateButton, "Create");
    choice(CancelButton, "Cancel");
  } else {
    const auto &d = m_instruments[m_selected];
    fittedText(kindName(d.kind), {1020, 380, 495, 27}, 26, theme::MutedText);
    text(d.path.filename().string(), {1020, 410}, 18, theme::MutedText);
    choice(DeleteButton, "Delete");
    if (d.kind == CustomKind::Script) {
      for (int i = 0; i < 3; ++i)
        choice({1020 + i * 240.f, 435, 230, 48}, CustomInstrumentStore::FunctionNames[i], m_function == i);
      m_script.draw();
      text("drawNote(position, colorIndex)  /  Colors: 0-5", {1020, 819}, 21, theme::MutedText);
    } else {
      const char *first = d.kind == CustomKind::Sections ? "Number of sections" : "Effective bits - Easy";
      const char *second = d.kind == CustomKind::Sections ? "Bits per section" : "Effective bits - Hard";
      for (int row = 0; row < 2; ++row) {
        const float y = 485 + row * 140.f;
        text(row == 0 ? first : second, {1020, y}, 28);
        choice({1020, y + 45, 70, 55}, "-");
        text(std::to_string(row == 0 ? d.first : d.second), {1250, y + 58}, 32, theme::Highlight);
        choice({1450, y + 45, 70, 55}, "+");
      }
      text(d.kind == CustomKind::Sections ? "Up to 6 sections and 30 total bits." : "1-30 bits. Easy cannot exceed Hard.",
           {1020, 815}, 24, theme::MutedText);
    }
    choice(SaveButton, "Save instrument", m_dirty);
    if (m_dirty || !m_error.empty()) choice(RevertButton, "Revert edits");
  }
  if (!m_error.empty()) message(m_error, {1020, 920}, 64);
  else if (m_dirty) text("Unsaved changes", {1020, 928}, 23, theme::MutedText);
  else if (m_saved) text("Instrument saved.", {1020, 928}, 23, theme::Highlight);
  if (m_confirmDelete && m_selected >= 0) {
    panel({0, 0, 1920, 1080}, Fade(BLACK, .75f));
    card({890, 420, 760, 280});
    text("Delete instrument?", {930, 450}, 36, theme::Error);
    fittedText(m_instruments[m_selected].name, {930, 510, 660, 36}, 30, theme::Text);
    fittedText("This removes its Lua file and any unsaved edits.", {930, 562, 660, 28}, 23, theme::MutedText);
    choice(ConfirmDelete, "Delete instrument");
    choice(CancelDelete, "Cancel");
  }
}
bool InstrumentSettings::save() {
  if (!m_dirty || m_creating || m_selected < 0) return true;
  if (!CustomInstrumentStore::save(m_instruments[m_selected], m_error)) return false;
  const auto path = m_instruments[m_selected].path;
  m_instruments = CustomInstrumentStore::list();
  const auto selected = std::ranges::find_if(m_instruments, [&](const auto &instrument) {
    return instrument.path == path;
  });
  if (selected != m_instruments.end()) select(static_cast<int>(selected - m_instruments.begin()));
  else { m_selected = -1; m_dirty = false; }
  m_saved = true;
  return true;
}
bool InstrumentSettings::canLeave() {
  if (m_dirty) {
    if (m_error.empty()) m_error = "Save or revert your instrument changes before leaving.";
    return false;
  }
  return true;
}
void InstrumentSettings::events() {
  using namespace settings_ui;
  if (m_confirmDelete) {
    if (clicked(CancelDelete) || IsKeyPressed(KEY_ESCAPE)) { m_confirmDelete = false; return; }
    if (clicked(ConfirmDelete) && m_selected >= 0) {
      const int index = m_selected;
      if (CustomInstrumentStore::remove(m_instruments[index], m_error)) {
        m_instruments = CustomInstrumentStore::list();
        m_dirty = false;
        m_saved = false;
        m_error.clear();
        if (m_instruments.empty()) {
          m_selected = -1;
          m_page = 0;
          m_name.value.clear();
          m_script.setText("");
        } else {
          select(std::min(index, static_cast<int>(m_instruments.size()) - 1));
          m_page = m_selected / PageSize;
        }
      }
      m_confirmDelete = false;
    }
    return;
  }
  if (clicked(NewButton) && canLeave()) {
    m_creating = true;
    m_newType = 0;
    m_name.value.clear();
    m_error.clear();
    reset();
  }
  for (int row = 0; row < PageSize; ++row) {
    const int index = m_page * PageSize + row;
    if (index < static_cast<int>(m_instruments.size()) && clicked({620, 320 + row * 70.f, 330, 60}) && canLeave()) select(index);
  }
  if (clicked({620, 830, 70, 55})) m_page = std::max(0, m_page - 1);
  if (clicked({880, 830, 70, 55})) m_page = std::min(std::max(0, (static_cast<int>(m_instruments.size()) - 1) / PageSize), m_page + 1);
  if (!m_creating && m_selected < 0) return;
  const bool nameChanged = m_name.input();
  if (m_creating) {
    for (int i = 1; i <= 3; ++i) if (clicked({1020, 465 + (i - 1) * 62.f, 710, 52})) m_newType = i;
    if (clicked(CreateButton)) {
      if (m_newType == 0) { m_error = "Choose an instrument type."; return; }
      InstrumentDefinition created;
      if (CustomInstrumentStore::create(m_name.value, static_cast<CustomKind>(m_newType), created, m_error)) {
        m_instruments.push_back(std::move(created));
        select(static_cast<int>(m_instruments.size()) - 1);
        m_page = m_selected / PageSize;
      }
    }
    if (clicked(CancelButton)) {
      m_creating = false;
      m_error.clear();
      if (m_selected >= 0) select(m_selected);
    }
    return;
  }
  auto &d = m_instruments[m_selected];
  if (clicked(DeleteButton)) { reset(); m_confirmDelete = true; return; }
  bool changed = nameChanged;
  if (nameChanged) d.name = m_name.value;
  if (d.kind == CustomKind::Script) {
    for (int i = 0; i < 3; ++i) {
      if (clicked({1020 + i * 240.f, 435, 230, 48})) {
        m_function = i;
        m_script.setText(d.functions[i]);
      }
    }
    if (m_script.input()) { d.functions[m_function] = m_script.value(); changed = true; }
  } else {
    for (int row = 0; row < 2; ++row) {
      const float y = 530 + row * 140.f;
      const int delta = clicked({1020, y, 70, 55}) ? -1 : clicked({1450, y, 70, 55}) ? 1 : 0;
      if (delta) {
        int &value = row == 0 ? d.first : d.second;
        const int minimum = d.kind == CustomKind::Bits && row == 1 ? std::clamp(d.first, 1, 30) : 1;
        const int maximum = d.kind == CustomKind::Sections ?
            std::max(1, std::min(row == 0 ? 6 : 8, 30 / std::max(1, row == 0 ? d.second : d.first))) :
            row == 0 ? std::clamp(d.second, 1, 30) : 30;
        const int adjusted = std::clamp(value + delta, minimum, maximum);
        changed = changed || adjusted != value;
        value = adjusted;
      }
    }
  }
  if (changed) { m_dirty = true; m_saved = false; m_error.clear(); }
  if (clicked(SaveButton)) save();
  if ((m_dirty || !m_error.empty()) && clicked(RevertButton)) {
    d = CustomInstrumentStore::load(d.path);
    select(m_selected);
  }
}
} // namespace bh
