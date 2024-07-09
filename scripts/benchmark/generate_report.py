from pathlib import Path
from typing import Any
from time import perf_counter

from arguments import create_and_parse_args
from report import generate_report, generate_combined_report

import pandas as pd
import multiprocess as mp
import yaml
import shutil
import functools


def main() -> None:
    t1 = perf_counter()
    args = create_and_parse_args()

    output_path: Path = args.output.resolve()
    if args.combine:
        if output_path.exists() and args.regenerate:
            shutil.rmtree(output_path)
        elif output_path.exists():
            print(f"Done in {perf_counter() - t1:.2f} seconds")
            return
        output_path.mkdir(parents=True, exist_ok=True)

    if not output_path.is_dir():
        raise NotADirectoryError(f"Output path '{output_path}' is not a directory")

    csvs: dict[Path, pd.DataFrame] = {}
    summaries: dict[Path, Any] = {}
    for input_path in args.input:
        input_path: Path = input_path.resolve()
        if not input_path.is_dir():
            raise NotADirectoryError(f"Input path '{input_path}' is not a directory")

        for f in input_path.rglob("*"):
            if f.is_dir() or f.name.startswith("."):
                continue

            output_folder = output_path / f.parent.name
            if not args.combine:
                if output_folder.exists() and args.regenerate:
                    shutil.rmtree(output_folder)
                elif output_folder.exists() and not args.combine:
                    continue
                output_folder.mkdir(parents=True, exist_ok=True)

            if f.suffix == ".csv":
                data = pd.read_csv(f.absolute())
                csvs[output_folder] = data
            else:
                with open(f, "r") as ffile:
                    data = yaml.safe_load(ffile)
                summaries[output_folder] = data

    if args.combine:
        generate_combined_report(summaries, csvs, args)
    elif args.jobs is not None:
        fn = functools.partial(generate_report, csvs=csvs)
        with mp.Pool(args.jobs) as pool:
            pool.starmap(fn, summaries.items())
    else:
        for output_folder, summary in summaries.items():
            generate_report((output_folder, summary), csvs)

    print(f"Done in {perf_counter() - t1:.2f} seconds")


if __name__ == "__main__":
    main()