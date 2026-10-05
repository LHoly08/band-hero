#include "config/CustomInstrumentStore.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <lua.hpp>
#if defined(_WIN32)
#include <windows.h>
#endif

namespace bh {
namespace {
// Settings owns only this trailing override block. Preserve the user's base
// script and helper functions rather than regenerating the entire Lua file.
constexpr std::string_view Begin = "\n-- BandHero settings overrides v1\n";
constexpr std::string_view End = "-- End BandHero settings overrides v1\n";
using Lua = std::unique_ptr<lua_State, decltype(&lua_close)>;

Lua newState() {
  Lua state(luaL_newstate(), lua_close);
  if (state) {
    // Metadata inspection never opens the OS, filesystem or package libraries.
    for (int i = 1; i <= 3; ++i) {
      lua_pushinteger(state.get(), i + 2);
      lua_setglobal(state.get(), ("Custom_" + std::to_string(i)).c_str());
    }
    lua_sethook(state.get(), [](lua_State *lua, lua_Debug *) {
      luaL_error(lua, "Instrument initialization exceeded its instruction limit");
    }, LUA_MASKCOUNT, 100000);
  }
  return state;
}
bool execute(lua_State *state, const std::string &source, std::string &error) {
  if (!state) { error = "Could not open the Lua editor."; return false; }
  if (luaL_loadbuffer(state, source.data(), source.size(), "instrument") != LUA_OK ||
      lua_pcall(state, 0, 0, 0) != LUA_OK) {
    const char *message = lua_tostring(state, -1);
    error = message ? message : "Invalid instrument script.";
    return false;
  }
  return true;
}
std::string read(const std::filesystem::path &path) {
  std::ifstream file(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(file), {}};
}
std::string quote(const std::string &name) {
  std::string result = "\"";
  for (unsigned char ch : name) {
    if (ch == '"' || ch == '\\') result += '\\';
    result += static_cast<char>(ch);
  }
  return result + '"';
}
std::string nameKey(std::string_view name) {
  // Names differing only in capitalization or extra spaces represent the
  // same instrument. Preserve the user's spelling in the actual Lua file.
  std::string key;
  bool space = false;
  for (unsigned char ch : name) {
    if (ch == ' ') { space = !key.empty(); continue; }
    if (space) key += ' ';
    space = false;
    key += static_cast<char>(ch < 128 ? std::tolower(ch) : ch);
  }
  return key;
}
bool namesMatch(std::string_view first, std::string_view second) {
  const auto a = nameKey(first), b = nameKey(second);
#if defined(_WIN32)
  // Windows supplies Unicode case comparison for accented instrument names.
  auto wide = [](const std::string &value) {
    std::wstring result(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        value.data(), static_cast<int>(value.size()), nullptr, 0), L'\0');
    if (!result.empty()) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        value.data(), static_cast<int>(value.size()), result.data(), static_cast<int>(result.size()));
    return result;
  };
  const auto wa = wide(a), wb = wide(b);
  if (!wa.empty() && !wb.empty()) return CompareStringOrdinal(wa.data(), static_cast<int>(wa.size()),
      wb.data(), static_cast<int>(wb.size()), TRUE) == CSTR_EQUAL;
#endif
  return a == b;
}
bool valid(const InstrumentDefinition &d, std::string &error) {
  if (d.kind != CustomKind::Sections && d.kind != CustomKind::Bits && d.kind != CustomKind::Script) {
    error = "Choose an instrument type."; return false;
  }
  if (d.name.empty() || d.name.size() > 64 ||
      d.name.find_first_not_of(' ') == std::string::npos ||
      std::ranges::any_of(d.name, [](unsigned char c) { return c < 32; })) {
    error = "Enter a name (1-64 characters)."; return false;
  }
  if (d.kind == CustomKind::Sections &&
      (d.first < 1 || d.first > 6 || d.second < 1 || d.second > 8 || d.first * d.second > 30)) {
    error = "Use 1-6 sections, 1-8 bits each, and at most 30 bits in total."; return false;
  }
  if (d.kind == CustomKind::Bits && (d.first < 1 || d.first > 30 || d.second < 1 || d.second > 30)) {
    error = "Easy and Hard must each use 1-30 bits."; return false;
  }
  if (d.kind == CustomKind::Bits && d.first > d.second) {
    error = "Easy bits must be less than or equal to Hard bits."; return false;
  }
  if (d.kind == CustomKind::Script) {
    for (int i = 0; i < 3; ++i) {
      auto state = newState();
      if (!execute(state.get(), d.functions[i], error)) return false;
      lua_getglobal(state.get(), CustomInstrumentStore::FunctionNames[i]);
      if (!lua_isfunction(state.get(), -1)) {
        error = std::string("Define function ") + CustomInstrumentStore::FunctionNames[i] + ".";
        return false;
      }
    }
  }
  return true;
}
std::string functionSource(lua_State *state, const std::string &source, const char *name) {
  // Lua supplies the actual function's source line range, avoiding a text scan
  // that could mistake nested blocks or comments for the function boundary.
  lua_getglobal(state, name);
  if (!lua_isfunction(state, -1)) { lua_pop(state, 1); return {}; }
  lua_Debug info{};
  if (!lua_getinfo(state, ">S", &info)) return {};
  std::istringstream lines(source);
  std::string line, result;
  for (int number = 1; std::getline(lines, line); ++number) {
    if (number >= info.linedefined && number <= info.lastlinedefined) result += line + '\n';
  }
  return result;
}
bool writeAtomic(const std::filesystem::path &path, const std::string &source, std::string &error) {
  // Write beside the target so replacement stays on the same filesystem.
  // Keep the existing file intact if writing or replacement fails.
  auto temporary = path;
  temporary += ".bandhero-tmp";
  std::error_code ec;
  if (std::filesystem::exists(temporary, ec)) {
    error = "A pending instrument write exists. Reopen settings after checking the file.";
    return false;
  }
  std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
  file.write(source.data(), static_cast<std::streamsize>(source.size()));
  file.close();
  if (file.fail()) {
    std::filesystem::remove(temporary, ec);
    error = "Could not write the instrument. Check folder permissions.";
    return false;
  }
#if defined(_WIN32)
  const bool success = MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
  std::filesystem::rename(temporary, path, ec);
  const bool success = !ec;
#endif
  if (!success) {
    std::filesystem::remove(temporary, ec);
    error = "Could not replace the instrument file. Your original is unchanged.";
  }
  return success;
}
}

InstrumentDefinition CustomInstrumentStore::load(const std::filesystem::path &path) {
  InstrumentDefinition d;
  d.path = path;
  d.name = path.stem().string();
  std::error_code ec;
  if (std::filesystem::file_size(path, ec) > 1024 * 1024 || ec) {
    d.error = "Could not read instrument (maximum size is 1 MB)."; return d;
  }
  d.source = read(path);
  auto state = newState();
  if (!execute(state.get(), d.source, d.error)) return d;
  lua_getglobal(state.get(), "name");
  if (lua_type(state.get(), -1) == LUA_TSTRING) d.name = lua_tostring(state.get(), -1);
  lua_pop(state.get(), 1);
  lua_getglobal(state.get(), "Type");
  const int type = lua_isinteger(state.get(), -1) ? static_cast<int>(lua_tointeger(state.get(), -1)) : 0;
  lua_pop(state.get(), 1);
  if (type < 3 || type > 5) { d.error = "Type must be Custom_1, Custom_2 or Custom_3."; return d; }
  d.kind = static_cast<CustomKind>(type - 2);
  if (d.kind == CustomKind::Script) {
    for (int i = 0; i < 3; ++i) d.functions[i] = functionSource(state.get(), d.source, FunctionNames[i]);
  } else {
    lua_getglobal(state.get(), "Composition");
    if (!lua_istable(state.get(), -1)) { d.error = "Missing Composition table."; return d; }
    const char *first = d.kind == CustomKind::Sections ? "NumberSections" : "NumberEffectiveBitsEasy";
    const char *second = d.kind == CustomKind::Sections ? "NumberBitsSection" : "NumberEffectiveBitsHard";
    lua_getfield(state.get(), -1, first);
    d.first = lua_isinteger(state.get(), -1) ? static_cast<int>(lua_tointeger(state.get(), -1)) : 0;
    lua_pop(state.get(), 1);
    lua_getfield(state.get(), -1, second);
    d.second = lua_isinteger(state.get(), -1) ? static_cast<int>(lua_tointeger(state.get(), -1)) : 0;
    lua_pop(state.get(), 2);
  }
  valid(d, d.error);
  return d;
}

std::vector<InstrumentDefinition> CustomInstrumentStore::list() {
  std::vector<InstrumentDefinition> result;
  std::error_code ec;
  std::filesystem::directory_iterator files("Instruments", ec), end;
  for (; !ec && files != end; files.increment(ec)) {
    if (files->is_regular_file(ec) && files->path().extension() == ".lua")
      result.push_back(load(files->path()));
  }
  std::ranges::sort(result, [](const auto &a, const auto &b) { return a.path < b.path; });
  for (std::size_t i = 0; i < result.size(); ++i) {
    for (std::size_t j = i + 1; j < result.size(); ++j) {
      if (namesMatch(result[i].name, result[j].name)) {
        const std::string error = "Duplicate instrument name. Rename one of these instruments.";
        if (result[i].error.empty()) result[i].error = error;
        if (result[j].error.empty()) result[j].error = error;
      }
    }
  }
  return result;
}

bool CustomInstrumentStore::save(InstrumentDefinition &d, std::string &error) {
  error.clear();
  if (!valid(d, error)) return false;
  std::error_code ec;
  const auto ownPath = std::filesystem::weakly_canonical(d.path, ec);
  if (ec) { error = "Could not resolve the instrument file."; return false; }
  for (const auto &other : list()) {
    const auto otherPath = std::filesystem::weakly_canonical(other.path, ec);
    if (ec) { error = "Could not check existing instrument names."; return false; }
    if (otherPath != ownPath && namesMatch(d.name, other.name)) {
      error = "An instrument with this name already exists. Choose another name.";
      return false;
    }
  }
  // Refuse stale editor snapshots instead of overwriting external edits.
  if (std::filesystem::exists(d.path, ec) && read(d.path) != d.source) {
    error = "This file changed outside the game. Reopen settings before editing it."; return false;
  }
  std::string base = d.source;
  if (base.ends_with(End)) {
    const auto start = base.rfind(Begin);
    if (start != std::string::npos) base.resize(start);
  }
  std::string source = base + std::string(Begin);
  source += "name = " + quote(d.name) + "\nType = Custom_" + std::to_string(static_cast<int>(d.kind)) + "\n";
  if (d.kind == CustomKind::Script) {
    for (const auto &function : d.functions) source += function + '\n';
  } else {
    source += d.kind == CustomKind::Sections ? "Composition = { NumberSections = " : "Composition = { NumberEffectiveBitsEasy = ";
    source += std::to_string(d.first);
    source += d.kind == CustomKind::Sections ? ", NumberBitsSection = " : ", NumberEffectiveBitsHard = ";
    source += std::to_string(d.second) + " }\n";
  }
  source += End;
  // Validate the combined base + overrides too: edited functions may interact
  // with helper code even when each function validated independently.
  auto state = newState();
  if (!execute(state.get(), source, error)) return false;
  if (!writeAtomic(d.path, source, error)) return false;
  d.source = std::move(source);
  d.error.clear();
  return true;
}

bool CustomInstrumentStore::remove(const InstrumentDefinition &d, std::string &error) {
  error.clear();
  std::error_code ec;
  const auto root = std::filesystem::canonical("Instruments", ec);
  if (ec) { error = "Could not open the Instruments folder."; return false; }
  const auto target = std::filesystem::canonical(d.path, ec);
  if (ec || target.parent_path() != root || target.extension() != ".lua" ||
      std::filesystem::is_symlink(d.path, ec) || !std::filesystem::is_regular_file(target, ec) || ec) {
    error = "Only Lua files inside the Instruments folder can be deleted.";
    return false;
  }
  if (read(target) != d.source) {
    error = "This file changed outside the game. Reopen settings before deleting it.";
    return false;
  }
  if (!std::filesystem::remove(target, ec) || ec) {
    error = "Could not delete the instrument. Check folder permissions.";
    return false;
  }
  return true;
}

bool CustomInstrumentStore::create(std::string name, CustomKind kind,
                                   InstrumentDefinition &result, std::string &error) {
  InstrumentDefinition d;
  d.name = std::move(name);
  d.kind = kind;
  if (kind == CustomKind::Bits) {
    d.first = 2;
    d.second = 3;
  }
  d.functions = {"function PlayEasy(note)\n  return note\nend\n",
                 "function PlayHard(note)\n  return note\nend\n",
                 "function Draw(incomingNote)\n  drawNote(0, 0)\nend\n"};
  if (!valid(d, error)) return false;
  std::error_code ec;
  std::filesystem::create_directories("Instruments", ec);
  if (ec) { error = "Could not create the Instruments folder."; return false; }
  for (int id = 1; id <= 65535; ++id) {
    d.path = std::filesystem::path("Instruments") / (std::to_string(id) + ".lua");
    if (!std::filesystem::exists(d.path, ec)) {
      if (!save(d, error)) return false;
      result = std::move(d);
      return true;
    }
  }
  error = "No free instrument file numbers remain.";
  return false;
}
} // namespace bh
