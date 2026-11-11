#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <string>
#include <thread>
#include "tr/args.hpp"
#include "tr/bmp.hpp"
#include "tr/pattern.hpp"

namespace {

using PatternFillFn = void(*)(tr::img::BMP *, double, tr::ArgConfig *);

template <PatternFillFn pattern_fill>
void fill(tr::ArgConfig &config) {
    std::vector<std::jthread> thread_list {};
    std::size_t thread_size = std::max(1u, std::min(config.frames_amount, std::thread::hardware_concurrency()));

    std::atomic<std::size_t> index { 0 };

    std::printf("Generating %u Frames across %lu threads\n", config.frames_amount, thread_size);

    for (std::size_t thread_index = 0; thread_index < thread_size; ++thread_index) {
        thread_list.emplace_back([&]() -> void {
            while (true) {
                std::size_t frame_index_local = index.fetch_add(1, std::memory_order_relaxed);

                if (frame_index_local >= config.frames_amount) {
                    return;
                } else {
                    int tenth_now = static_cast<int>((double)frame_index_local / config.frames_amount * 10);
                    int tenth_before = static_cast<int>(((double)frame_index_local - 1) / config.frames_amount * 10);

                    if (tenth_now != tenth_before) {
                        std::printf("%d0%% is completed\n", tenth_now);
                    }
                }

                std::filesystem::path filepath { config.dirpath };
                std::ofstream fout { config.dirpath / std::format("frame_{:04}.bmp", frame_index_local) };

                if (fout.is_open()) {
                    tr::img::BMP bmp { config.screen_width, config.screen_height, };

                    double time = (double)frame_index_local / (double)config.frames_ratio;
                    pattern_fill(&bmp, time, &config);

                    bmp.write(&fout);

                    fout.close();
                }
            }
        });
    }
}

}

int main(int argc, char **argv) {
    tr::ArgConfig config {};
    std::optional<tr::ArgError> args_error = tr::args_parse(&config, argc, argv);

    if (args_error.has_value()) {
        std::printf("Arg Parsing Error: %s\n", args_error.value().message.c_str());

        return 1;
    }

    std::filesystem::create_directory(config.dirpath);

    {
        std::filesystem::directory_iterator dir { config.dirpath };

        for (auto file : dir) {
            const std::filesystem::path &path = file.path();

            if (path.filename().string().starts_with("frame_") && path.filename().string().ends_with(".bmp")) {
                std::filesystem::remove(path);
            }
        }
    }

    std::printf("begin generation\n");

    if (config.flag_precise) {
        fill<tr::pattern_fill_precise>(config);
    } else {
        fill<tr::pattern_fill_batch>(config);
    }

    std::printf("100%% is completed\n");

    return 0;
}
