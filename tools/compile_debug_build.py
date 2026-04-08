import subprocess
import sys
import os
import time

# Paths
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PARENT_DIR = os.path.dirname(SCRIPT_DIR)
XPBD_Plugin_DIR = os.path.join(PARENT_DIR, "XPBD_Plugin")
DONT_TOUCH_PATH = os.path.join(PARENT_DIR, "dont_touch.txt")

def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')


def read_saved_godot_path():
    """Read the saved Godot executable path from dont_touch.txt line 3."""
    try:
        with open(DONT_TOUCH_PATH, "r") as f:
            lines = f.readlines()
        if len(lines) >= 3:
            path = lines[2].strip()
            if path and path != "GODOT_ENGINE_PATH_GOES_HERE" and os.path.isfile(path):
                return path
    except Exception:
        pass
    return None


def save_godot_path(godot_path):
    """Save the Godot executable path to dont_touch.txt line 3."""
    try:
        with open(DONT_TOUCH_PATH, "r") as f:
            lines = f.readlines()
        while len(lines) < 3:
            lines.append("\n")
        lines[2] = godot_path + "\n"
        with open(DONT_TOUCH_PATH, "w") as f:
            f.writelines(lines)
    except Exception:
        pass


def get_godot_executable_path():
    """Find the executable path of a running Godot editor process (Windows only)."""
    if os.name != 'nt':
        return None
    try:
        result = subprocess.run(
            ['powershell', '-NoProfile', '-Command',
             "(Get-Process -Name 'Godot*' -ErrorAction SilentlyContinue | Select-Object -First 1).Path"],
            capture_output=True, text=True
        )
        path = result.stdout.strip()
        if path:
            return path
    except Exception:
        pass
    return None


def reload_godot_project(godot_path):
    """Reopen the test project in the Godot editor."""
    try:
        print("\nReopening Godot editor...")
        subprocess.Popen(
            [godot_path, '--editor', '--path', XPBD_Plugin_DIR],
            creationflags=subprocess.DETACHED_PROCESS | subprocess.CREATE_NEW_PROCESS_GROUP
        )
        print("Godot project reloaded.")
    except Exception as e:
        print(f"Failed to reload Godot project: {e}")


def run_scons_build():
    """Run 'scons compiledb=yes' from the project root and show output in real-time."""
    try:
        # Check if Godot is running before compilation (it locks the DLL on Windows)
        godot_path = get_godot_executable_path()
        was_running = godot_path is not None
        if godot_path:
            save_godot_path(godot_path)
            print("Godot editor detected. Closing it to unlock the DLL for compilation...")
            subprocess.run(
                ['powershell', '-NoProfile', '-Command',
                 "Get-Process -Name 'Godot*' -ErrorAction SilentlyContinue | Stop-Process -Force"],
                capture_output=True, text=True
            )
            time.sleep(1)
        else:
            godot_path = read_saved_godot_path()

        process = subprocess.Popen(
            ["scons", "compiledb=yes"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
            cwd=PARENT_DIR  # Run from root directory where SConstruct is
        )

        stdout_lines = []
        stderr_lines = []

        while True:
            output = process.stdout.readline()
            if output:
                print(output, end="")
                stdout_lines.append(output)
            elif process.poll() is not None:
                break

        remaining_out, remaining_err = process.communicate()
        if remaining_out:
            print(remaining_out, end="")
            stdout_lines.append(remaining_out)
        if remaining_err:
            stderr_lines.append(remaining_err)

        if process.returncode == 0:
            print("\nCompilation finished successfully.")
            print("A debug build for your current OS and architecture was added to the bin folder.")
            print("The compile_commands.json file was also updated to improve IntelliSense support.")
            if godot_path:
                reload_godot_project(godot_path)
            else:
                print("\nNo saved Godot editor path found. Open the project manually or run Godot once so the path can be saved.")
        else:
            print("\nCompilation FAILED:")
            print(''.join(stderr_lines).strip() or "Unknown error occurred.")

    except FileNotFoundError:
        print("Error: 'scons' command not found. Make sure SCons is installed and available in your PATH.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

    input("\nPress any key to continue...")


if __name__ == "__main__":
    clear_screen()
    run_scons_build()
