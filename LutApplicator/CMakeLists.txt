# CMakeList.txt: LutApplicator 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
add_executable (LutApplicator "LutApplicator.cpp" "LutApplicator.h" "src/public/FolderWatcher.h" "src/public/FileChangeNotification.h" "src/public/LutApplicator.h" "src/public/ImageProcessor.h" "src/private/ImageProcessor.cpp" "src/public/MetadataProcessor.h" "src/private/MetadataProcessor.cpp" "src/public/Lut3D.h" "src/private/Lut3D.cpp")

target_include_directories(LutApplicator PRIVATE 
    src/private
    src/public
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LutApplicator PROPERTY CXX_STANDARD 20)
endif()

find_package(JPEG REQUIRED)
target_link_libraries(LutApplicator PRIVATE JPEG::JPEG)

find_package(libjpeg-turbo REQUIRED) 
target_link_libraries(LutApplicator PRIVATE libjpeg-turbo::turbojpeg) 

find_package(exiv2 CONFIG REQUIRED)
target_link_libraries(LutApplicator PRIVATE Exiv2::exiv2lib)

# TODO: 如有需要，请添加测试并安装目标。
