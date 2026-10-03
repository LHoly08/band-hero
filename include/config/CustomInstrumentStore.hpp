#pragma once
#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace bh {
enum class CustomKind { Sections = 1, Bits, Script };

struct InstrumentDefinition {
  std::filesystem::path path;
  std::string name;
  CustomKind kind{CustomKind::Sections};
  int first{3};
  int second{2};
  std::array<std::string, 3> functions;
  std::string source;
  std::string error;
};
class CustomInstrumentStore {
public:
  static constexpr std::array<const char *, 3> FunctionNames{"PlayEasy", "PlayHard", "Draw"};
  static std::vector<InstrumentDefinition> list();
  static InstrumentDefinition load(const std::filesystem::path &path);
  static bool save(InstrumentDefinition &definition, std::string &error);
  static bool remove(const InstrumentDefinition &definition, std::string &error);
  static bool create(std::string name, CustomKind kind, InstrumentDefinition &result, std::string &error);
};
} // namespace bh
