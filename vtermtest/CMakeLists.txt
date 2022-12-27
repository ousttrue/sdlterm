find_package(
  ICU
  COMPONENTS uc io i18n data
  REQUIRED)

set(TARGET_NAME vtermtest)
add_executable(${TARGET_NAME} main.cpp vtermtest.cpp)
target_link_libraries(
  ${TARGET_NAME}
  PRIVATE SDL2::SDL2
          SDL2::SDL2main
          SDL2_ttf::SDL2_ttf
          ICU::uc
          ICU::io
          ICU::i18n
          ICU::data
          vterm
          freetype)
