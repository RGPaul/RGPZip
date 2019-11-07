from conans import ConanFile, CMake, tools
import os

class RGPZipConan(ConanFile):
    name = "rgpzip"
    author = "Ralph-Gordon Paul (gordon@rgpaul.com)"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "android_ndk": "ANY", "android_stl_type":["c++_static", "c++_shared"]}
    default_options = "shared=False", "android_ndk=None", "android_stl_type=c++_static"
    description = "A lightweight C++ Wrapper for Minizip."
    url = "https://github.com/RGPaul/RGPZip"
    license = "MIT"
    exports_sources = ("CMakeLists.txt", "src/*", "include/*", "deps/zlib/*", "cmake-modules/*", "test/*")
    generators = "cmake_paths"

    # compile using cmake
    def build(self):
        cmake = CMake(self)
        cmake.verbose = True

        if self.settings.os == "Android":
            self.applyCmakeSettingsForAndroid(cmake)

        if self.settings.os == "iOS":
            self.applyCmakeSettingsForiOS(cmake)

        if self.settings.os == "Macos":
            self.applyCmakeSettingsFormacOS(cmake)

        if self.settings.os == "Windows":
            self.applyCmakeSettingsForWindows(cmake)

        cmake.configure(source_folder=self.source_folder)
        cmake.build()
        cmake.install()

        # run unit tests if not cross building
        if not tools.cross_building(self.settings):
            cmake.test()

    def applyCmakeSettingsForAndroid(self, cmake):
        android_toolchain = os.environ["ANDROID_NDK_PATH"] + "/build/cmake/android.toolchain.cmake"
        cmake.definitions["CMAKE_SYSTEM_NAME"] = "Android"
        cmake.definitions["CMAKE_TOOLCHAIN_FILE"] = android_toolchain
        cmake.definitions["ANDROID_NDK"] = os.environ["ANDROID_NDK_PATH"]
        cmake.definitions["ANDROID_ABI"] = tools.to_android_abi(self.settings.arch)
        cmake.definitions["ANDROID_STL"] = self.options.android_stl_type
        cmake.definitions["ANDROID_NATIVE_API_LEVEL"] = self.settings.os.api_level
        cmake.definitions["ANDROID_TOOLCHAIN"] = "clang"
        cmake.definitions["BUILD_TESTS"] = "OFF"

    def applyCmakeSettingsForiOS(self, cmake):
        ios_toolchain = "cmake-modules/Toolchains/ios.toolchain.cmake"
        cmake.definitions["CMAKE_TOOLCHAIN_FILE"] = ios_toolchain
        cmake.definitions["DEPLOYMENT_TARGET"] = "10.0"
        cmake.definitions["BUILD_TESTS"] = "OFF"

        if self.settings.arch == "x86":
            cmake.definitions["PLATFORM"] = "SIMULATOR"
        elif self.settings.arch == "x86_64":
            cmake.definitions["PLATFORM"] = "SIMULATOR64"
        else:
            cmake.definitions["PLATFORM"] = "OS"

        # define all architectures for ios fat library
        if "arm" in self.settings.arch:
            cmake.definitions["ARCHS"] = "armv7;armv7s;arm64;arm64e"
        else:
            cmake.definitions["ARCHS"] = tools.to_apple_arch(self.settings.arch)
    
    def applyCmakeSettingsFormacOS(self, cmake):
        cmake.definitions["CMAKE_OSX_ARCHITECTURES"] = tools.to_apple_arch(self.settings.arch)

    def applyCmakeSettingsForWindows(self, cmake):
        cmake.definitions["CMAKE_BUILD_TYPE"] = self.settings.build_type
        if self.settings.compiler == "Visual Studio":
            # check that runtime flags and build_type correspond (consistency check)
            if "d" not in self.settings.compiler.runtime and self.settings.build_type == "Debug":
                raise Exception("Compiling for Debug mode but compiler runtime does not contain 'd' flag.")

            if self.settings.build_type == "Debug":
                cmake.definitions["CMAKE_CXX_FLAGS_DEBUG"] = "/%s" % self.settings.compiler.runtime
            elif self.settings.build_type == "Release":
                cmake.definitions["CMAKE_CXX_FLAGS_RELEASE"] = "/%s" % self.settings.compiler.runtime

    def build_requirements(self):
        self.build_requires("boost/1.70.0@%s/%s" % (self.user, self.channel))
        self.build_requires("zlib/1.2.11@%s/%s" % (self.user, self.channel))
        
    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
        self.cpp_info.includedirs = ['include']

    def package_id(self):
        if "arm" in self.settings.arch and self.settings.os == "iOS":
            self.info.settings.arch = "AnyARM"

    def config_options(self):
        # remove android specific option for all other platforms
        if self.settings.os != "Android":
            del self.options.android_ndk
            del self.options.android_stl_type

    def configure(self):
        if self.settings.os == "Android":
            self.options["boost"].android_ndk = self.options.android_ndk
            self.options["boost"].android_stl_type = self.options.android_stl_type
            self.options["zlib"].android_ndk = self.options.android_ndk
            self.options["zlib"].android_stl_type = self.options.android_stl_type
