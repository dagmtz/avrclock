import os
import subprocess
import sys
import platform


def run_command(command, cwd=None):
    """
    Runs a shell command and prints its output.

    :param command: The command to execute (list of strings).
    :param cwd: The working directory for the command.
    :return: True if the command succeeds, False otherwise.
    """
    try:
        process = subprocess.run(command, cwd=cwd, check=True, text=True)
        return process.returncode == 0
    except subprocess.CalledProcessError as e:
        print(f"Error: Command '{' '.join(command)}' failed with exit code {e.returncode}")
        return False

def build_cmake_project(build_dir):
    """
    Builds a CMake project.

    :param build_dir: The source directory containing the CMakeLists.txt.
    """

    # Configure the project
    print(f"Configuring project in {build_dir}...")

    if os.name == "nt":
        if not run_command(["cmake", build_dir, "-G MinGW Makefiles"]):
            print("Configuration failed.")
            return
    
    elif os.name == "posix":
        if not run_command(["cmake", build_dir], build_dir):
            print("Configuration failed.")
            return
        
    if not run_command(["cmake", "--build", build_dir], build_dir):
        print("Build failed.")
        return

    print("Build completed successfully.")

if __name__ == "__main__":
    os_name = platform.system()
    current_directory = os.getcwd()
    current_dir_name = os.path.basename(current_directory)

    print(f"Building project '{current_dir_name}' under {os_name} system")
    build_cmake_project(current_directory)
