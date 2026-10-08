#pragma once
#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace bh {
enum class CustomKind { Sections = 1, Bits, Script };
enum class SourceStem { Bass, Drums, Guitar, Piano, Other };
inline constexpr std::array<std::string_view, 5> SourceStemNames{
    "bass", "drums", "guitar", "piano", "other"};

// Editor snapshot: first/second are section count/bits per section for Sections,
// or Easy/Hard effective bits for Bits. Script uses functions in FunctionNames
// order. source retains the last loaded/saved file for external-change checks.
struct InstrumentDefinition {
  std::filesystem::path path;
  std::string name;
  CustomKind kind{CustomKind::Sections};
  // Chart generation uses Audio/<SourceStem>.wav as its model input.
  SourceStem sourceStem{SourceStem::Guitar};
  int first{3};
  int second{2};
  std::array<std::string, 3> functions;
  std::string source;
  std::string error;
};
class CustomInstrumentStore {
public:
  static constexpr std::array<const char *, 3> FunctionNames{"PlayEasy", "PlayHard", "Draw"};
  // Invalid and duplicate definitions stay in the list with an error so the
  // editor can repair them; callers must check error before offering gameplay.
  static std::vector<InstrumentDefinition> list();
  static InstrumentDefinition load(const std::filesystem::path &path);
  static bool save(InstrumentDefinition &definition, std::string &error);
  static bool remove(const InstrumentDefinition &definition, std::string &error);
  static bool create(std::string name, CustomKind kind, InstrumentDefinition &result, std::string &error,
                     SourceStem sourceStem = SourceStem::Guitar);
};
} // namespace bh
