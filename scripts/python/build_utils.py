import argparse
import os
import subprocess
import sys


def parse_args():
    parser = argparse.ArgumentParser(description="Configure and build cpp project for E2E testing.")
    parser.add_argument(
        "--build-type",
        choices=["debug", "release"],
        default="release",
        help="Build type: debug or release (default: release)"
    )
    parser.add_argument(
        "--with-tests",
        action="store_true",
        help="Enable building tests (default: off)"
    )
    parser.add_argument(
        "--binary-name",
        default="main_exe",
        help="Name of the output binary file (default: main_exe)"
    )
    args = parser.parse_args()
    return args


def configure_cmake(args, build_dir_name):
    os.makedirs(build_dir_name, exist_ok=True)

    build_type = args.build_type.upper()
    enable_tests = "ON" if args.with_tests else "OFF"

    configure_cmake_cmd = [
        "cmake",
        "-B", build_dir_name,
        "-DCMAKE_BUILD_TYPE=" + build_type,
        "-DBUILD_TESTS=" + enable_tests
    ]

    result = subprocess.run(configure_cmake_cmd)
    if result.returncode != 0:
        sys.exit("CMake configuration failed")


def build_project(build_dir_name, exe_name, test_name, with_tests):
    cmake_build_cmd = ["cmake", "--build", build_dir_name]
    result = subprocess.run(cmake_build_cmd)
    if result.returncode != 0:
        sys.exit("Build failed")

    # create execution file absolute path
    exe_path = None
    for fname in os.listdir(build_dir_name):
        if fname.startswith(exe_name):
            exe_path = os.path.abspath(os.path.join(build_dir_name, fname))
            if os.path.isfile(exe_path) and os.access(exe_path, os.X_OK):
                break

    if exe_path is None:
        raise FileNotFoundError(f"Error : execfile starting with '{exe_name}' not found in {build_dir_name}")

    # create tests file absolute path if needed
    test_path = None
    if with_tests:
        for fname in os.listdir(build_dir_name):
            if fname.startswith("test"):
                test_path = os.path.abspath(os.path.join(build_dir_name, fname, test_name))
                if os.path.isfile(test_path) and os.access(test_path, os.X_OK):
                    return exe_path, test_path
        raise FileNotFoundError(f"Error : tests starting with '{test_name}' not found in {build_dir_name}/test")

    return exe_path, test_path


def configure_and_build_project(build_dir_name, exe_name, test_name):
    args = parse_args()
    print(args)

    print("run CMake configuration")
    configure_cmake(args, build_dir_name)

    print("build project")
    return build_project(build_dir_name, exe_name, test_name, args.with_tests)
