import os
import shutil

def delete_cmake_files(root_dir):
    """
    Recursively deletes CMake-generated files in the specified directory.

    :param root_dir: The root directory to search for CMake files.
    """
    # List of CMake-generated files to delete
    files_to_delete = ["CMakeCache.txt", "cmake_install.cmake", "Makefile", "compile_commands.json", ".clangd"]
    dirs_to_delete = ["CMakeFiles"]

    for dirpath, dirnames, filenames in os.walk(root_dir):
        # Remove specific files
        for filename in filenames:
            if filename in files_to_delete: # or filename.endswith(".ninja"):
                file_path = os.path.join(dirpath, filename)
                try:
                    os.remove(file_path)
                    print(f"Deleted file: {file_path}")
                except OSError as e:
                    print(f"Error deleting file {file_path}: {e}")
        
        # Remove specific directories
        for dirname in dirnames:
            if dirname in dirs_to_delete:
                dir_path = os.path.join(dirpath, dirname)
                try:
                    shutil.rmtree(dir_path)
                    print(f"Deleted directory: {dir_path}")
                except OSError as e:
                    print(f"Error deleting directory {dir_path}: {e}")

if __name__ == "__main__":
    # Use the current working directory as the root directory
    current_directory = os.getcwd()
    print(f"Cleaning CMake files under: {current_directory}")
    delete_cmake_files(current_directory)
