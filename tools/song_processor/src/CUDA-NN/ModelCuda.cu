#include "ModelCuda.cuh"

#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <utility>


namespace {

constexpr std::array<char, 4> Magic{'L', 'H', 'N', 'N'};

constexpr std::uint32_t Version = 1;

} // namespace

ModelCuda::ModelCuda() : m_l1(784, 128), m_l2(128, 10) {}

ModelCuda::ModelCuda(LinearCuda &&l1, LinearCuda &&l2)
    : m_l1(std::move(l1)), m_l2(std::move(l2)) {}

TensorCuda ModelCuda::forward(const TensorCuda &input) {

  m_h1 = m_l1.forward(input);
  m_h2 = m_relu.forward(*m_h1);

  return m_l2.forward(*m_h2);
}

TensorCuda ModelCuda::backward(const TensorCuda &gradient, float learningRate) {
  assert(m_h1.has_value());
  assert(m_h2.has_value());

  TensorCuda g = m_l2.backward(gradient, learningRate);

  g = m_relu.backward(g);

  m_h1.reset();
  m_h2.reset();

  return m_l1.backward(g, learningRate);
}

void ModelCuda::save(const std::filesystem::path &path) const {
  std::ofstream out{path, std::ios::binary};

  if (!out) {
    throw std::runtime_error{"Failed to open model file for writing"};
  }

  // -------------------------
  // Header
  // -------------------------

  out.write(Magic.data(), static_cast<std::streamsize>(Magic.size()));

  out.write(reinterpret_cast<const char *>(&Version), sizeof(Version));

  if (!out) {
    throw std::runtime_error{"Failed to write model header"};
  }

  // -------------------------
  // Parameters
  // -------------------------

  m_l1.save(out);
  m_l2.save(out);

  if (!out) {
    throw std::runtime_error{"Failed to write model"};
  }
}

ModelCuda ModelCuda::load(const std::filesystem::path &path) {
  std::ifstream in{path, std::ios::binary};

  if (!in) {
    throw std::runtime_error{"Failed to open model file"};
  }

  // -------------------------
  // Magic
  // -------------------------

  std::array<char, 4> magic{};

  in.read(magic.data(), static_cast<std::streamsize>(magic.size()));

  if (!in || magic != Magic) {
    throw std::runtime_error{"Invalid model file"};
  }

  // -------------------------
  // Version
  // -------------------------

  std::uint32_t version{};

  in.read(reinterpret_cast<char *>(&version), sizeof(version));

  if (!in) {
    throw std::runtime_error{"Failed to read model version"};
  }

  if (version != Version) {
    throw std::runtime_error{"Unsupported model version"};
  }

  // -------------------------
  // Layers
  // -------------------------

  LinearCuda l1 = LinearCuda::load(in);

  LinearCuda l2 = LinearCuda::load(in);

  // Validate this model's architecture.

  // The LinearCuda(Tensor, Tensor)
  // constructor already validates that each
  // bias matches its corresponding weights.

  return ModelCuda{std::move(l1), std::move(l2)};
}