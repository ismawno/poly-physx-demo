from pathlib import Path
from collections.abc import Sequence
from time import perf_counter
from arguments import create_and_parse_scenario_report_args

import generate_report
import sys


def main() -> None:
    t1 = perf_counter()

    args = create_and_parse_scenario_report_args()
    common_args = sys.argv[1:]

    input_path = Path(f"output/benchmark/data/{args.name}")
    output_path: Path = args.output / args.name

    def create_groups(groups: Sequence[str]) -> dict[str, list[int]]:
        parsed_groups: dict[str, list[int]] = {}
        for group in set(groups):
            sgroup = sorted({int(g) for g in group.split("_")})
            parsed_groups["-".join([str(g) for g in sgroup])] = sgroup
        return parsed_groups

    if args.scenario_groups is not None:
        scenario_groups = create_groups(args.scenario_groups)

    if args.cycle_groups is not None:
        cycle_groups = create_groups(args.cycle_groups)

    def filter_by_runs(runs: Sequence[Path], selected: str) -> list[Path]:
        selected_runs = sorted({int(index) for index in selected.split("_")})
        filtered_runs = []
        for run in runs:
            index = int(run.name.split("-")[0])
            if index in selected_runs:
                filtered_runs.append(run)
        return filtered_runs

    input_runs = sorted([folder for folder in input_path.glob("*") if folder.is_dir()])

    if args.scenario_runs is not None:
        input_runs = filter_by_runs(input_runs, args.scenario_runs)

    # standalone report generation
    if args.standalone and args.cycle_runs is None:
        genargs = common_args.copy()
        genargs.extend(
            [
                "-i",
                *[str(folder) for folder in input_runs],
                "-o",
                *[str(output_path / folder.name) for folder in input_runs],
            ]
        )

        generate_report.main(genargs, log_duration=False)

    combined_args = common_args.copy()
    combined_args.append("-c")
    # for every group, a dict of inputs and outputs with the cycle name as key
    grouped_scenarios: dict[str, dict[str, tuple[list[str], str]]] = {}
    for run_folder in input_runs:
        scindex = int(run_folder.name.split("-")[0])

        cycle_runs = sorted(
            [folder for folder in run_folder.glob("*") if folder.is_dir()]
        )
        if args.cycle_runs is not None:
            cycle_runs = filter_by_runs(cycle_runs, args.cycle_runs)
            if args.standalone:
                for folder in cycle_runs:
                    genargs = common_args.copy()
                    genargs.extend(
                        [
                            "-i",
                            str(folder),
                            "-o",
                            str(output_path / run_folder.name / folder.name),
                        ]
                    )
                    generate_report.main(genargs, log_duration=False)

        if args.cycle_groups is not None:
            grouped_cycles: dict[str, list[str]] = {}
            for cycle in cycle_runs:
                cindex = int(cycle.name.split("-")[0])
                for group_str, group_ints in cycle_groups.items():
                    if cindex in group_ints:
                        grouped_cycles.setdefault(group_str, []).append(str(cycle))
            for group_str, cycles in grouped_cycles.items():
                genargs = combined_args.copy()
                genargs.extend(
                    [
                        "-i",
                        *cycles,
                        "-o",
                        str(output_path / run_folder.name / group_str),
                    ]
                )
                generate_report.main(genargs, log_duration=False)

        if args.scenario_groups is not None:
            for group_str, group_ints in scenario_groups.items():
                if scindex not in group_ints:
                    continue
                for cycle in cycle_runs:
                    grouped_scenarios.setdefault(group_str, {}).setdefault(
                        cycle.name,
                        ([], str(output_path / group_str / cycle.name)),
                    )[0].append(str(cycle))

    for cycles in grouped_scenarios.values():
        for inputs, output in cycles.values():
            genargs = combined_args.copy()
            genargs.extend(["-i", *inputs, "-o", output])
            generate_report.main(genargs, log_duration=False)

    print(f"Done in {perf_counter() - t1:.2f} seconds")


if __name__ == "__main__":
    main()
