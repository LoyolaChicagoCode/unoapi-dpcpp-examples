add_executable(wordcloud main.cpp)
target_link_libraries(wordcloud fmt::fmt spdlog::spdlog scn::scn CLI11::CLI11)
