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
        subpattern_file: str
):
    run_cmd = ['./bin/tirdf', data_graph, pattern_file, str(window_size), str(max_thread_num), runtime_record, subpattern_file]
    print('Running: `{}`'.format(' '.join(run_cmd)))

    proc = Popen(run_cmd, stdout=PIPE, stderr=PIPE, encoding='utf-8')
    outs, errs = proc.communicate()

    print('---------')
    print(outs, errs)
    print('---------')

    num_match_res = re.search('Num match: (\d+)', outs)
    if num_match_res is None:
        num_match = 0
    else:
        num_match = num_match_res.group(1)
        num_match = int(num_match)
    cpu_time = re.search('CPU time elapsed: (\d+\.\d+) secs', outs).group(1)
    cpu_time = float(cpu_time)

    peak_mem_result = re.search('Peak memory usage: (\d+) (.)B', outs)
    if peak_mem_result is not None:
        peak_mem = peak_mem_result.group(1)
        peak_mem_unit = peak_mem_result.group(2)

        multiplier = 1
        if peak_mem_unit == 'k':
            multiplier = 2**10
        elif peak_mem_unit == 'M':
            multiplier = 2**20
        elif peak_mem_unit == 'G':
            multiplier = 2**30
        else:
            print(f'Encounter unknown memory unit: {peak_mem_unit}')
        
        peak_mem = int(peak_mem) * multiplier
    else:
        peak_mem = None
    
    return num_match, cpu_time, peak_mem

if __name__ == '__main__':
    parser = parser = argparse.ArgumentParser(
                formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                description='Run all pattern on all graph')
    parser.add_argument('-d', '--data-graph',
                    default='./IPMES/data/preprocessed/',
                    type=str,
                    help='the folder of data graphs')
    parser.add_argument('-p', '--pattern-dir',
                    default='./IPMES/data/universal_patterns/',
                    type=str,
                    help='the folder of patterns')
    parser.add_argument('-o', '--out-dir',
                    default='./results/',
                    type=str,
                    help='the output folder')
    parser.add_argument('-n', '--no-darpa',
                        help='Do not run darpa dataset',
                        action='store_true')
    args = parser.parse_args()

    
    if os.getcwd().endswith('scripts'):
        os.chdir('..')

    subprocess.run(['make', 'clean'], check=True)
    subprocess.run(['make', '-j'], check=True)

    os.makedirs(args.out_dir, exist_ok=True)
    os.makedirs(os.path.join(args.out_dir, 'runtime_records'), exist_ok=True)
    os.makedirs(os.path.join(args.out_dir, 'answers'), exist_ok=True)
    
    darpa_graphs = ['dd1', 'dd2', 'dd3', 'dd4']
    spade_graphs = ['attack', 'mix', 'benign']

    run_result = []

    for i in range(1, 13):
        for graph in spade_graphs:
            filename = f'SP{i}_regex'
            window_size = 1800
            num_threads = 1

            pattern = os.path.join(args.pattern_dir, f'{filename}.json')
            subpattern = os.path.join(args.pattern_dir, f'subpatterns/{filename}.json')
            data_graph = os.path.join(args.data_graph, graph + '.csv')
            runtime_record = os.path.join(args.out_dir, f'runtime_records/{filename}.txt')

            num_match, cpu_time, peak_mem = run(data_graph, pattern, window_size, num_threads, runtime_record, subpattern)
            run_result.append([f'SP{i}', graph, num_match, cpu_time, peak_mem / 2**20])

            # No answer outputs
            subprocess.run(['mv', './answers', f"{os.path.join(args.out_dir, 'answers')}/{filename}_{graph}"], check=True)
        
    if args.no_darpa is False: 
        for i in range(1, 6):
            for graph in darpa_graphs:
                filename = f'DP{i}_regex'
                window_size = 1000
                num_threads = 1

                pattern = os.path.join(args.pattern_dir, f'{filename}.json')
                subpattern = os.path.join(args.pattern_dir, f'subpatterns/{filename}.json')
                data_graph = os.path.join(args.data_graph, graph + '.csv')
                runtime_record = os.path.join(args.out_dir, f'runtime_records/{filename}.txt')

                num_match, cpu_time, peak_mem = run(data_graph, pattern, window_size, num_threads, runtime_record, subpattern)
                run_result.append([f'DP{i}', graph, num_match, cpu_time, peak_mem / 2**20])
        
                # No answer outputs
                subprocess.run(['mv', './answers', f"{os.path.join(args.out_dir, 'answers')}/{filename}_{graph}"], check=True)

    df = pd.DataFrame(data=run_result, columns=['Pattern', 'Data Graph', 'Num Results', 'CPU Time (sec)', 'Peak Memory (MB)'])
    print(df.to_string(index=False))
    df.to_csv(os.path.join(args.out_dir, 'run_result.csv'), index=False)

