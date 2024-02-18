
# UnoAPI:CMakeLists-targetlibraries:begin

add_executable(cipher main.cpp)
target_link_libraries(cipher spdlog::spdlog CLI11::CLI11)

# UnoAPI: CMakeLists-targetlibraries:end
