#pragma once
#include <vector>
#include "config/CustomInstrumentStore.hpp"
#include "ui/settings/Controls.hpp"
#include "ui/settings/ScriptEditor.hpp"

namespace bh {
class InstrumentSettings {
public:
  void onEnter();
  void draw() const;
  void events();
  bool canLeave();
  // Reload saved definitions and retain the selected file when it still exists.
  // Unlike reset(), this removes drafts; reset() only releases input focus.
  void discard();
  bool deletionPending() const { return m_confirmDelete; }
  void reset();
private:
  void select(int index);
  bool save();
  std::vector<InstrumentDefinition> m_instruments;
  settings_ui::TextField m_name{{1020, 310, 710, 60}};
  settings_ui::ScriptEditor m_script{{1020, 495, 710, 320}};
  int m_selected{-1};
  int m_page{};
  int m_function{};
  int m_newType{};
  bool m_creating{};
  bool m_dirty{};
  bool m_saved{};
  bool m_confirmDelete{};
  std::string m_error;
};
} // namespace bh
