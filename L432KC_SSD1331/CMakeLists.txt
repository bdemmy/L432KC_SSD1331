cmake_minimum_required(VERSION 3.15)

project(L432KC_SSD1331 LANGUAGES C CXX ASM)

find_bsp(ID com.sysprogs.project_importers.stm32.cubemx
	SOURCE_PROJECT L432KC_SSD1331.ioc)

add_bsp_based_executable(NAME L432KC_SSD1331
	GENERATE_BIN
	GENERATE_MAP)
