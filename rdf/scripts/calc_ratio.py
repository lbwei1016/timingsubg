import csv
import numpy as np

def read_csv(filename: str) -> tuple[list[int], list[int]]:
    with open(filename, newline='') as csvfile:
        cpu_time = []
        memory = []
        rows = csv.DictReader(csvfile)
        cnt = 0
        for row in rows:
            # only calculate SPADE
            if cnt >= 12 * 3:
                break
            cpu_time.append(float(row['CPU Time (sec)']))
            memory.append(float(row['Peak Memory (MB)']))
            cnt += 1
    return (cpu_time, memory)

def calc_ratio(list1: list, list2: list) -> list: 
    ratio = []
    for (x, y) in zip(list1, list2):
       ratio.append(float(x) / float(y))
    return ratio

def print_ratio(ratio: list, title: str):
    print(f'{title} ratios...')
    cnt = 0
    graphs = ['attack', 'mix', 'benign']
    for i in range(1, 13):
        for graph in graphs:
            print(f'SP{i} {graph}: {ratio[cnt]}')
            cnt += 1

        # input('Enter to continue...')

def split(data: list):
    return (data[0::3], data[1::3], data[2::3])

if __name__ == '__main__':
    ipmes_file = 'IPMES/results/ipmes-rust/run_result.csv'
    # timing_file = 'results/run_results_all_SP_old.csv'
    timing_file = 'results/run_result.csv'

    ipmes_cpu, ipmes_mem = read_csv(ipmes_file)
    timing_cpu, timing_mem = read_csv(timing_file)

    cpu_ratio = calc_ratio(timing_cpu, ipmes_cpu)
    mem_ratio = calc_ratio(timing_mem, ipmes_mem)
    # print('CPU ratio =', cpu_ratio)
    # print('Peak memory ratio =', mem_ratio)
    print_ratio(cpu_ratio, 'CPU time')
    print_ratio(mem_ratio, 'Peak memory usage')


    tc = split(timing_cpu)
    tm = split(timing_mem)
    ic = split(ipmes_cpu)
    im = split(ipmes_mem)

    graphs = ['attack', 'mix', 'benign']
    for i in range(3):
        print(f'({graphs[i]}) Average CPU time ratio:', np.average(tc[i]) / np.average(ic[i]))
        print(f'({graphs[i]}) Average peak memory usage ratio:', np.average(tm[i]) / np.average(im[i]))

    # timing_avg_cpu = np.average(timing_cpu)
    # ipmes_avg_cpu = np.average(ipmes_cpu)
    # timing_avg_mem = np.average(timing_mem)
    # ipmes_avg_mem = np.average(ipmes_mem)

    # print('Average CPU time ratio:', timing_avg_cpu / ipmes_avg_cpu)
    # print('Average Peak memory usage ratio:', timing_avg_mem / ipmes_avg_mem)