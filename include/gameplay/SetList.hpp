#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace bh {

class SetList {
public:
  SetList() = default;
  SetList(const SetList &) = delete;
  void operator=(const SetList &) = delete;
  SetList(SetList &&other)
      : start(std::exchange(other.start, nullptr)),
        end(std::exchange(other.end, nullptr)) {}

  SetList &operator=(SetList &&other) {
    if (this != &other) {
      start = std::exchange(other.start, nullptr);
      end = std::exchange(other.end, nullptr);
    }
    return *this;
  }
  ~SetList() = default;

  // Playback can consume its own queue while the song menu keeps its selection.
  [[nodiscard]] SetList clone() const {
    SetList result;
    for (const auto *node = start.get(); node; node = node->next.get()) {
      result.add(std::string{node->audio}, std::string{node->charts});
    }
    return result;
  }

  inline void add(std::string &&audio, std::string &&charts) {
    auto node = std::make_unique<SetListNode>(std::move(audio),
                                              std::move(charts), nullptr);
    if (end) {
      end->next = std::move(node);
      end = end->next.get();
    } else {
      start = std::move(node);
      end = start.get();
    }
  }

  inline std::string getAudio() {
    if (!start) {
      return {};
    }
    return start->audio;
  }

  inline char *getData() {
    if (!start) {
      return nullptr;
    }
    return start->charts.data();
  }

  inline std::string getCharts() {
    if (!start) {
      return {};
    }
    return start->charts;
  }

  inline void pop() {
    if (!start) {
      return;
    }
    start = std::move(start->next);

    if (!start) {
      end = nullptr;
    }
  }

  inline bool empty() const { return start == nullptr; }

  inline void reset() {
    start = nullptr;
    end = nullptr;
  }

private:
  struct SetListNode {
    std::string audio{};
    std::string charts{};
    std::unique_ptr<SetListNode> next{nullptr};
  };

  std::unique_ptr<SetListNode> start{nullptr};
  SetListNode *end{nullptr};
};

} // namespace bh
