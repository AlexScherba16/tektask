import subprocess
import sys

from scripts.python.build_utils import configure_and_build_project


def run_unit_tests(test_path):
    if test_path is None:
        return

    result = subprocess.run(test_path)
    if result.returncode != 0:
        sys.exit("Unit tests failed")


def run_e2e_tests(exe_path):
    print("\n🔍 Running E2E tests...")

    test_cases = [
        {
            "name": "Valid_Linear_And_Quadratic",
            "args": ["0", "0", "0", "1", "-2", "-3", "1", "2", "1"],
            "expected_output": "(0, 0, 0) => infinite roots, no extremum\n"
                               "(1, -2, -3) => (3, -1), Xmin=1\n"
                               "(1, 2, 1) => (-1), Xmin=-1",
            "expect_failure": False
        },
        {
            "name": "Invalid_NonNumeric",
            "args": ["1", "abc", "3"],
            "expected_output": "(1,abc,3) => Invalid input: failed to parse triplet\n"
                               "Invalid input: no valid parameters",
            "expect_failure": True
        },
        {
            "name": "IncompleteTriplet",
            "args": ["1", "2"],
            "expected_output": "(1,2,) => Invalid input: parameter count must be a multiple of 3!\n"
                               "Invalid input: no valid parameters",
            "expect_failure": True
        },
        {
            "name": "NoArgs",
            "args": [],
            "expected_output": "Invalid input: missing command line arguments",
            "expect_failure": True
        },
        {
            "name": "Single_Valid_Triplet",
            "args": ["1", "-2", "-3"],
            "expected_output": "(1, -2, -3) => (3, -1), Xmin=1\n",
            "expect_failure": False
        },
        {
            "name": "Valid_Invalid_Triplets",
            "args": ["-20", "-30", "a", "0", "0", "0", "a", "b", "100", "2", "-6", "-8", "1", "0", "1", "100", "200"],
            "expected_output": "(-20,-30,a) => Invalid input: failed to parse triplet\n"
                               "(a,b,100) => Invalid input: failed to parse triplet\n"
                               "(100,200,) => Invalid input: parameter count must be a multiple of 3!\n\n"
                               "(0, 0, 0) => infinite roots, no extremum\n"
                               "(2, -6, -8) => (4, -1), Xmin=1.5\n"
                               "(1, 0, 1) => no real roots, Xmin=0\n",
            "expect_failure": False
        },

        # add test here
    ]

    for case in test_cases:
        print(f"\nRunning [{case['name']}]")

        try:
            result = subprocess.run(
                [exe_path] + case["args"],
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,  # stdout | stderr
                check=False
            )

            test_stdout = result.stdout.strip()
            expected_stdout = case["expected_output"].strip()

            if case["expect_failure"]:
                if result.returncode == 0:
                    raise AssertionError(f"Expected failure, but got success.\nstdout:\n{test_stdout}")
                print(f"[{case['name']}] expected failure, stdout:\n{test_stdout}")

            else:
                if result.returncode != 0:
                    raise AssertionError(f"Unexpected failure (code={result.returncode})\nstdout:\n{test_stdout}")

            if test_stdout != expected_stdout:
                raise AssertionError(
                    f"Output mismatch in '{case['name']}':\n"
                    f"Expected:\n{expected_stdout}\nGot:\n{test_stdout}"
                )
            print(f"[{case['name']}] OK")

        except Exception as e:
            print(f"[{case['name']}] FAILED:\n{e}")
            sys.exit(1)

    print("\nAll E2E tests passed.")


def main():
    try:
        build_dir_name = "cpp_build"
        exe_name = "se_solver"
        test_name = "se_solver_test"

        exe_absolute_path, test_absolute_path = configure_and_build_project(build_dir_name, exe_name, test_name)
        print(f"EXEC_PATH : {exe_absolute_path}\nTEST_PATH : {test_absolute_path}")

        run_unit_tests(test_absolute_path)
        run_e2e_tests(exe_absolute_path)
    except FileNotFoundError as e:
        sys.exit(str(e))


if __name__ == "__main__":
    main()
