import typing as t
import argparse
import subprocess
from subprocess import Popen, PIPE
import os
import re
import pandas as pd


def run(
    data_graph: str,
    pattern_file: str,
    window_size: int,
    max_thread_num: int,
    runtime_record: str,
    subpattern_file: str,
    pre_run=0,
    re_run=1,
) -> t.Union[t.Tuple[int, float, int], None]:
    run_cmd = [
        "./bin/tirdf",
        data_graph,
        pattern_file,
        str(window_size),
        str(max_thread_num),
        runtime_record,
        subpattern_file,
    ]
    # def run(pattern_file: str, data_graph: str, window_size: int, pre_run=0, re_run=1) -> t.Union[t.Tuple[int, float, int], None]:
    # run_cmd = ['./target/release/ipmes-rust', pattern_file, data_graph, '-w', str(window_size), '--silent']

    print("Running: `{}`".format(" ".join(run_cmd)))

    for _ in range(pre_run):
        proc = Popen(run_cmd, stdout=None, stderr=None, encoding="utf-8")
        proc.wait()

    num_match = "0"
    peak_mem_result = None
    total_cpu_time = 0.0
    for i in range(re_run):
        print(f"Run {i + 1} / {re_run} ...")
        proc = Popen(run_cmd, stdout=PIPE, stderr=PIPE, encoding="utf-8")
        outs, errs = proc.communicate()
        if proc.wait() != 0:
            print(f"Run failed:\n{errs}")
            return None

        print(outs)

        num_match = re.search(r"Total number of matches: (\d+)", outs).group(1)
        cpu_time = re.search(r"CPU time elapsed: (\d+\.\d+) secs", outs).group(1)
        total_cpu_time += float(cpu_time)
        peak_mem_result = re.search(r"Peak memory usage: (\d+) (.)B", outs)

    avg_cpu_time = total_cpu_time / re_run
    num_match = int(num_match)
    if peak_mem_result is not None:
        peak_mem = peak_mem_result.group(1)
        peak_mem_unit = peak_mem_result.group(2)

        multiplier = 1
        if peak_mem_unit == "k":
            multiplier = 2**10
        elif peak_mem_unit == "M":
            multiplier = 2**20
        elif peak_mem_unit == "G":
            multiplier = 2**30
        else:
            print(f"Encounter unknown memory unit: {peak_mem_unit}")

        peak_mem = int(peak_mem) * multiplier
    else:
        peak_mem = 0

    return num_match, avg_cpu_time, peak_mem


if __name__ == "__main__":
    parser = parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        description="Run all pattern on all graph",
    )
    parser.add_argument(
        "-d",
        "--data-graph",
        default="data/preprocessed/",
        type=str,
        help="the folder of data graphs",
    )
    parser.add_argument(
        "-p",
        "--pattern-dir",
        default="data/universal_patterns/",
        type=str,
        help="the folder of patterns",
    )
    parser.add_argument(
        "-o", "--out-dir", default="results/timing/", type=str, help="the output folder"
    )
    parser.add_argument(
        "-r",
        "--re-run",
        default=1,
        type=int,
        help="Number of re-runs to measure CPU time",
    )
    parser.add_argument(
        "--pre-run",
        default=0,
        type=int,
        help="Number of runs before actual measurement",
    )
    parser.add_argument(
        "--no-darpa", default=False, action="store_true", help="Do not run on DARPA"
    )
    parser.add_argument(
        "--no-spade", default=False, action="store_true", help="Do not run on SPADE"
    )
    parser.add_argument(
        "--no-dp1", default=True, action="store_true", help="Do not run DP1 for all data graphs"
    )
    parser.add_argument(
        "--no-dp5-dd4", default=True, action="store_true", help="Do not run DP5 on dd4"
    )
    parser.add_argument(
        "--runtime-records",
        default="results/timing/runtime_records",
        type=str,
        help="the output folder for runtime records",
    )
    parser.add_argument(
        "--answers",
        default="results/timing/answers",
        type=str,
        help="the output folder for matched instances",
    )
    args = parser.parse_args()

    if os.getcwd().endswith("scripts"):
        os.chdir("..")

    # subprocess.run(['cargo', 'build', '--release'], check=True)
    subprocess.run(["make", "clean"], check=True)
    subprocess.run(["make", "-j"], check=True)

    os.makedirs(args.out_dir, exist_ok=True)
    os.makedirs(args.runtime_records, exist_ok=True)
    os.makedirs(args.answers, exist_ok=True)

    darpa_graphs = ["dd1", "dd2", "dd3", "dd4"]
    spade_graphs = ["attack", "mix", "benign"]

    run_result = []

    if not args.no_spade:
        for i in range(1, 13):
            filename = f'SP{i}_regex'
            window_size = 1800
            num_threads = 1
            for graph in spade_graphs:
                pattern = os.path.join(args.pattern_dir, f"{filename}.json")
                subpattern = os.path.join(
                    args.pattern_dir, f"subpatterns/{filename}.json"
                )
                data_graph = os.path.join(args.data_graph, graph + ".csv")
                runtime_record = os.path.join(args.runtime_records, f"{filename}.txt")

                # res = run(pattern, data_graph, 1800, args.pre_run, args.re_run)
                res = run(
                    data_graph,
                    pattern,
                    window_size,
                    num_threads,
                    runtime_record,
                    subpattern,
                    args.pre_run,
                    args.re_run
                )
                if not res is None:
                    num_match, cpu_time, peak_mem = res
                    run_result.append(
                        [f"SP{i}", graph, num_match, cpu_time, peak_mem / 2**20]
                    )

                subprocess.run(['mv', './answers', f"{os.path.join(args.out_dir, 'answers')}/{filename}_{graph}"], check=True)

    if not args.no_darpa:
        for i in range(1, 6):
            filename = f'DP{i}_regex'
            window_size = 1000
            num_threads = 1
            for graph in darpa_graphs:
                if i == 1 and args.no_dp1:
                    continue
                
                if i == 5 and graph == 'dd4' and args.no_dp5_dd4:
                    continue

                pattern = os.path.join(args.pattern_dir, f"{filename}.json")
                subpattern = os.path.join(
                    args.pattern_dir, f"subpatterns/{filename}.json"
                )
                data_graph = os.path.join(args.data_graph, graph + ".csv")
                runtime_record = os.path.join(args.runtime_records, f"{filename}.txt")

                res = run(
                    data_graph,
                    pattern,
                    window_size,
                    num_threads,
                    runtime_record,
                    subpattern,
                    args.pre_run,
                    args.re_run
                )
                if not res is None:
                    num_match, cpu_time, peak_mem = res
                    run_result.append(
                        [f"DP{i}", graph, num_match, cpu_time, peak_mem / 2**20]
                    )

                subprocess.run(['mv', './answers', f"{os.path.join(args.out_dir, 'answers')}/{filename}_{graph}"], check=True)

    df = pd.DataFrame(
        data=run_result,
        columns=[
            "Pattern",
            "Data Graph",
            "Num Results",
            "CPU Time (sec)",
            "Peak Memory (MB)",
        ],
    )
    print(df.to_string(index=False))
    out_file = os.path.join(args.out_dir, "run_result.csv")
    df.to_csv(out_file, index=False)
    print(f"This table is saved to {out_file}")
