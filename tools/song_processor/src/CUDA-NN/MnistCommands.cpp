#include "MnistCommands.hpp"
#include "CrossEntropyCuda.cuh"
#include "MNIST.hpp"
#include "ModelCuda.cuh"
#include "SoftmaxCuda.cuh"
#include "TensorCuda.cuh"
#include <chrono>
#include <algorithm>
#include <filesystem>
#include <print>

namespace {

constexpr std::size_t BatchSize = 16384;
constexpr std::size_t Epochs = 5000;
float learningRate = 0.001f;

const std::filesystem::path ModelPath{
    "mnist.lhnn"
};

std::size_t predict(
    const Tensor& probabilities,
    std::size_t row)
{
    std::size_t best{};

    for (std::size_t col{1};
         col < probabilities.cols();
         ++col) {

        if (probabilities(row, col) >
            probabilities(row, best)) {
            best = col;
        }
    }

    return best;
}

float testModel(
    ModelCuda& model,
    const MNIST& test)
{
    const auto start =
        std::chrono::steady_clock::now();

    std::size_t correct{};

    for (std::size_t offset{};
         offset < test.images.rows();
         offset += BatchSize) {

        const std::size_t count =
            std::min(
                BatchSize,
                test.images.rows() - offset
            );

        Tensor input =
            getBatch(
                test.images,
                offset,
                count
            );

        TensorCuda inputCuda{input};

        TensorCuda logits =
            model.forward(inputCuda);

        TensorCuda probabilitiesCuda =
            softmaxCuda(logits);

        Tensor probabilities{
            probabilitiesCuda
        };

        for (std::size_t row{};
             row < count;
             ++row) {

            const std::size_t predicted =
                predict(
                    probabilities,
                    row
                );

            if (predicted ==
                test.labels[offset + row]) {
                ++correct;
            }
        }
    }

    const auto end =
        std::chrono::steady_clock::now();

    const float accuracy =
        static_cast<float>(correct) /
        static_cast<float>(
            test.images.rows()
        );

    const double seconds =
        std::chrono::duration<double>(
            end - start
        ).count();

    std::println(
        "Accuracy: {:.2f}%",
        accuracy * 100.0f
    );

    std::println(
        "Testing time: {:.3f}s",
        seconds
    );

    return accuracy;
}

void trainModel(
    ModelCuda& model,
    const MNIST& training)
{
    const auto trainingStart =
        std::chrono::steady_clock::now();

    for (std::size_t epoch{};
         epoch < Epochs;
         ++epoch) {

            if (epoch >= 2500) {
                learningRate = 0.000001f;
            } else if (epoch >= 1000) {
                learningRate = 0.000005f;
            } else if (epoch >= 500) {
                learningRate = 0.00001f;
            } else if (epoch >= 300) {
                learningRate = 0.00005f;
            } else if (epoch >= 150) {
                learningRate = 0.0001f;
            } else if (epoch >= 50) {
                learningRate = 0.0005f;
            }
        const auto epochStart =
            std::chrono::steady_clock::now();

        float totalLoss{};
        std::size_t batches{};

        for (std::size_t offset{};
             offset + BatchSize <= training.images.rows();
             offset += BatchSize) {

            Tensor input =
                getBatch(
                    training.images,
                    offset,
                    BatchSize
                );

            auto labels =
                getLabels(
                    training.labels,
                    offset,
                    BatchSize
                );

            TensorCuda inputCuda{input};

            // Forward
            TensorCuda logits =
                model.forward(inputCuda);

            TensorCuda probabilities =
                softmaxCuda(logits);

            const float loss =
                crossEntropyCuda(
                    probabilities,
                    labels
                );

            totalLoss += loss;
            ++batches;

            // Backward
            TensorCuda gradient =
                crossEntropyBackwardCuda(
                    probabilities,
                    labels
                );

            static_cast<void>(
                model.backward(
                    gradient,
                    learningRate
                )
            );
        }

        const auto epochEnd =
            std::chrono::steady_clock::now();

        const double seconds =
            std::chrono::duration<double>(
                epochEnd - epochStart
            ).count();

        std::println(
            "Epoch {}: loss = {:.6f}, lr = {}, time = {:.3f}s",
            epoch + 1,
            totalLoss /
                static_cast<float>(batches),
            learningRate,
            seconds
        );
    }

    const auto trainingEnd =
        std::chrono::steady_clock::now();

    const double seconds =
        std::chrono::duration<double>(
            trainingEnd - trainingStart
        ).count();

    std::println(
        "Total training time: {:.3f}s",
        seconds
    );
}

} // namespace

void trainMnist()
{
    std::println("Loading training data...");

    const auto training =
        loadMNIST(
            "data/train-images-idx3-ubyte",
            "data/train-labels-idx1-ubyte"
        );

    const auto test =
        loadMNIST(
            "data/t10k-images-idx3-ubyte",
            "data/t10k-labels-idx1-ubyte"
        );

    std::println(
        "Training images: {}",
        training.images.rows()
    );

    std::println(
        "Test images: {}",
        test.images.rows()
    );

    std::println("\n=== Training ===");

    ModelCuda model = std::filesystem::exists(ModelPath)
        ? ModelCuda::load(ModelPath)
        : ModelCuda{};

    trainModel(model, training);

    std::println(
        "\n=== Testing trained model ==="
    );

    testModel(model, test);

    std::println(
        "\nSaving model to {}...",
        ModelPath.string()
    );

    model.save(ModelPath);

    std::println("Model saved.");
}

void testMnist()
{
    std::println("Loading test data...");

    const auto test =
        loadMNIST(
            "data/t10k-images-idx3-ubyte",
            "data/t10k-labels-idx1-ubyte"
        );

    std::println(
        "Test images: {}",
        test.images.rows()
    );

    std::println(
        "Loading model from {}...",
        ModelPath.string()
    );

    ModelCuda model =
        ModelCuda::load(ModelPath);

    std::println("Model loaded.");

    std::println("\n=== Testing ===");

    testModel(model, test);
}
