from utils import ROOT_PATH
import os
from utils import download_file
import subprocess
from exceptions import DependencyNotFoundError
import sys


def validate_mingw() -> None:
    print("====MINGW VALIDATION====")

    if not __is_mingw_installed() and not __install_mingw():
        raise DependencyNotFoundError("MinGW")
    print("MinGW installed")


def __is_mingw_installed() -> bool:
    if not os.path.exists("C:\\MinGW\\bin"):
        return False

    sys.path.append("C:\\MinGW\\bin")
    return (
        subprocess.run(["g++", "--version"], shell=True, capture_output=True).returncode
        == 0
    )


def __install_mingw() -> bool:
    if os.path.exists("C:\\MinGW\\bin"):
        subprocess.run(["set", "PATH=%PATH%;C:\MinGW\bin"], shell=True)
        subprocess.run(
            ["mingw-get.exe", "install", "g++"], shell=True
        )  # Should already be on path...
        return True

    dir = f"{ROOT_PATH}/vendor/MinGW/bin"
    installer_url = (
        "https://osdn.net/frs/redir.php?m=nchc&f=mingw%2F68260%2Fmingw-get-setup.exe"
    )
    installer_path = f"{dir}/mingw-get-setup.exe"

    while True:
        answer = input("MinGW installation not found. Do you wish to install? [Y]/N ")
        if answer == "n" or answer == "N":
            return False
        elif answer == "y" or answer == "Y" or answer == "":
            break

    print("Starting MinGW installation...")
    print(f"Downloading {installer_url} to {installer_path}...")
    download_file(installer_url, installer_path)
    print(
        "MinGW installer will now be executed. DO NOT CHECK THE MINGW GUI OPTION INSTALLATION (it won't be necessary for this setup). Once the installation completes, RE-RUN the script"
    )
    input("Press any key to execute installer...")
    os.startfile(installer_path)
    exit()
