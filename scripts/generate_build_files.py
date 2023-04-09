from setup_py import validate_python_version

validate_python_version()

from argparse import ArgumentParser
from generator import FullGenerator, PPXGenerator, SFMLGenerator, Generator
from exceptions import UnrecognizedWhichArgumentError, GeneratorNotSupportedError
from utils import ROOT_PATH
import platform


def main() -> None:
    parser = ArgumentParser(
        description="Generate the build files for the project.",
        epilog="Once the build files have been generated, the project must be compiled according to the chosen generator.",
    )
    parser.add_argument(
        "--which",
        dest="which",
        default="all",
        type=str,
        help="can be one of the following: 'sfml' - builds SFML as a shared library using CMake, 'ppx' -  generates poly-physx-demo's build files with premake5. SFML must be built first or 'all' - executes both 'sfml' and 'ppx'. Default: 'all'",
    )
    parser.add_argument(
        "--clean",
        dest="clean",
        action="store_const",
        const=True,
        default=False,
        help="clears all build files for the selected project component",
    )
    parser.add_argument(
        "--generator",
        dest="generator",
        default="gmake2" if platform.system() == "Darwin" else "vs2022",
        type=str,
        help="Can be one of the actions listed in 'premake5 --help' option. Defaults to 'gmake2' in MacOS and 'vs2022' in Windows",
    )

    args = parser.parse_args()
    if not args.generator.startswith("vs") and not args.generator.startswith("gmake"):
        raise GeneratorNotSupportedError(args.generator)

    print(f"Setup wrt root: {ROOT_PATH}\n")
    options = {
        "all": FullGenerator(args.generator),
        "ppx": PPXGenerator(args.generator),
        "sfml": SFMLGenerator(args.generator),
    }

    try:
        gen: Generator = options[args.which]
        if args.clean:
            gen.clean()
        else:
            gen.build()
            print(
                "\nCompile the project according to the selected generator.\nIf you have selected gmake2 (default), use 'make' from the root folder to compile the project. Enter 'make help' to see all possible configurations. Choose the one compatible with your architecture.\nThe executable will be located in the poly-physx-demo subfolder's binaries.\nThe program should be executed from the source path for the executable to be able to locate the SFML shared libs."
            )
    except KeyError:
        raise UnrecognizedWhichArgumentError(args.which)


if __name__ == "__main__":
    main()
