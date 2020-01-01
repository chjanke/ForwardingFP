cmake_minimum_required(VERSION 3.12...3.16)
project(ForwardingFP VERSION 1.0.0 LANGUAGES CXX)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
	${PROJECT_NAME} INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include/FwdFP>
	)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

add_library(FwdFP::${PROJECT_NAME} ALIAS ${PROJECT_NAME})


install(
	TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
	INCLUDES DESTINATION include
)

install(
	EXPORT ${PROJECT_NAME}Targets
	FILE ${PROJECT_NAME}Targets.cmake
	NAMESPACE FwdFP::
	DESTINATION lib/cmake/FwdFP
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	${PROJECT_NAME}Version.cmake
	VERSION ${PACKAGE_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(
	FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/ForwardingFPConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/ForwardingFPVersion.cmake"
	DESTINATION lib/cmake/FwdFP
)

install(FILES include/Forwarding.hpp DESTINATION include/FwdFP)
