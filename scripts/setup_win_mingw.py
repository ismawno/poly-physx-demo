from utils import Buddy, download_file, unzip_file
import os
import subprocess
from exceptions import DependencyNotFoundError


def validate_mingw() -> None:
    print("\n==== MINGW VALIDATION ====")

    if not __resolve_mingw_installation() and not __install_mingw():
        raise DependencyNotFoundError("MinGW")
    print("MinGW installed")


def __is_mingw_installed() -> bool:
    return __is_gxx_installed() and __is_make_installed()


def __resolve_mingw_installation() -> bool:
    if __is_mingw_installed():
        return True

    print(
        "A valid MinGW installation was not found in path (g++ and/or make are missing)"
    )
    bud = Buddy()
    if os.path.exists(bud.mingw_path):
        print(f"MinGW installation found at {bud.mingw_path}. Adding to path...")
        bud.add_to_path_with_binaries(bud.mingw_path)
        if __is_mingw_installed():
            return True
        print("Failed. Still unable to find g++ and make executables in path.")

    if bud.prompt(
        f"MinGW installation not found at {bud.mingw_path}. Is it located elsewhere?"
    ):
        mingw_path = input(
            "Enter MinGW installation path (Use forward or double backward slashes): "
        )
        print(f"Adding {mingw_path} to path...")
        bud.add_to_path_with_binaries(mingw_path)
        bud.mingw_path = mingw_path
        return __is_mingw_installed()
    return False


def __is_gxx_installed() -> bool:
    return (
        subprocess.run(["g++", "--version"], shell=True, capture_output=True).returncode
        == 0
    )


def __is_make_installed() -> bool:
    return (
        subprocess.run(
            ["mingw32-make", "--version"], shell=True, capture_output=True
        ).returncode
        == 0
    )


def __install_mingw() -> bool:
    bud = Buddy()

    dir = f"{bud.root_path}/vendor/MinGW/bin"
    arch = "i686" if bud.os_architecture == "x86" else bud.os_architecture
    thingy = "dwarf" if bud.os_architecture == "x86" else "seh"

    zip_name = f"winlibs-{arch}-posix-{thingy}-gcc-12.2.0-mingw-w64ucrt-10.0.0-r5.zip"
    zip_url = f"https://github.com/brechtsanders/winlibs_mingw/releases/download/12.2.0-16.0.0-10.0.0-ucrt-r5/{zip_name}"

    zip_path = f"{dir}/{zip_name}"

    if not bud.prompt_to_install("MinGW"):
        return False

    print("Starting MinGW installation...")
    print(f"Downloading {zip_url} to {zip_path}...")
    download_file(zip_url, zip_path)
    print(f"\nExtracting {zip_path}...")
    unzip_file(zip_path, "C:/")

    bud.add_to_path_with_binaries(bud.mingw_path)
    print(f"\nMinGW has been successfully installed at {bud.mingw_path}.")

    return __is_mingw_installed()
