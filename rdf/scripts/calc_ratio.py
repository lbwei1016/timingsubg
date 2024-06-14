import csv
import numpy as np


def read_DP(filename, truncated: bool = True):
    with open(filename, newline='') as csvfile:
        cpu_time = []
        memory = []
        rows = csv.DictReader(csvfile)
        cnt = 0
        for row in rows:
            if (truncated and 'DP1' in row['Pattern']) or 'DP' not in row['Pattern']:
                continue
            cpu_time.append(float(row['CPU Time (sec)']))
            memory.append(float(row['Peak Memory (MB)']))
            cnt += 1
    return (cpu_time, memory)

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

def print_ratio(
        ratio: list, 
        title: str,
        prefix: str = 'SP',
        range_dataset: range = range(1, 13),
        graphs: list[str] = ['attack', 'mix', 'benign']
):
    print(f'{title} ratios...')
    cnt = 0
    # graphs = ['attack', 'mix', 'benign']
    for i in range_dataset:
        for graph in graphs:
            print(f'{prefix}{i} {graph}: {ratio[cnt]:.1f}')
            cnt += 1

        input('Enter to continue...')

def split(data: list, step: int=3):
    if step == 4:
        return (data[0::4], data[1::4], data[2::4], data[3::4])

    return (data[0::3], data[1::3], data[2::3])



def read_siddhi(file: str):
    data = []
    with open(file, 'r') as f:
        for line in f:
            data.append(float(line))
    return data


def calc_DP(ipmes_file, timing_file, siddhi_cpu_file, siddhi_mem_file, is_timing=True):
    if is_timing:
        ipmes_cpu, ipmes_mem = read_DP(ipmes_file)
    else:
        ipmes_cpu, ipmes_mem = read_DP(ipmes_file, truncated=False)

    timing_cpu, timing_mem = read_DP(timing_file)
    siddhi_cpu, siddhi_mem = read_siddhi(siddhi_cpu_file), read_siddhi(siddhi_mem_file)

    # Dummy value for DP5 dd4
    # timing_cpu.append(1)
    # timing_mem.append(1)

    # To MB
    siddhi_mem = np.array(siddhi_mem) / 10**6

    print(siddhi_cpu)
    print(ipmes_cpu)
    print(siddhi_mem)
    print(ipmes_mem)

    if is_timing:
        cpu_ratio = calc_ratio(timing_cpu, ipmes_cpu)
        mem_ratio = calc_ratio(timing_mem, ipmes_mem)
        range_dataset = range(2, 6)
    else:
        cpu_ratio = calc_ratio(siddhi_cpu, ipmes_cpu)
        mem_ratio = calc_ratio(siddhi_mem, ipmes_mem)
        range_dataset = range(1, 6)

    prefix = 'DP'
    graphs = ['dd1', 'dd2', 'dd3', 'dd4']
    print_ratio(cpu_ratio, 'CPU time', prefix, range_dataset, graphs)
    print_ratio(mem_ratio, 'Peak memory usage', prefix, range_dataset, graphs)

    tc = split(timing_cpu, 4)
    tm = split(timing_mem, 4)
    ic = split(ipmes_cpu, 4)
    im = split(ipmes_mem, 4)
    sc = split(siddhi_cpu, 4)
    sm = split(siddhi_mem, 4)

    for i in range(len(graphs)):
        if is_timing:
            print(f'({graphs[i]}) Average CPU time ratio:', np.average(tc[i]) / np.average(ic[i]))
            print(f'({graphs[i]}) Average peak memory usage ratio:', np.average(tm[i]) / np.average(im[i]))
        else:
            print(f'({graphs[i]}) Average CPU time ratio:', np.average(sc[i]) / np.average(ic[i]))
            print(f'({graphs[i]}) Average peak memory usage ratio:', np.average(sm[i]) / np.average(im[i]))


# Use this with caution. This function is not tested yet.
def calc_SP(ipmes_file, timing_file, siddhi_cpu_file, siddhi_mem_file):
    ipmes_cpu, ipmes_mem = read_csv(ipmes_file)
    timing_cpu, timing_mem = read_csv(timing_file)
    siddhi_cpu, siddhi_mem = read_siddhi(siddhi_cpu_file), read_siddhi(siddhi_mem_file)

    # To MB
    siddhi_mem = np.array(siddhi_mem) / 10**6

    print(siddhi_cpu)
    print(siddhi_mem)

    cpu_ratio = calc_ratio(siddhi_cpu, ipmes_cpu)
    mem_ratio = calc_ratio(siddhi_mem, ipmes_mem)
    print_ratio(cpu_ratio, 'CPU time')
    print_ratio(mem_ratio, 'Peak memory usage')

#    cpu_ratio = calc_ratio(timing_cpu, ipmes_cpu)
#    mem_ratio = calc_ratio(timing_mem, ipmes_mem)
    # print('CPU ratio =', cpu_ratio)
    # print('Peak memory ratio =', mem_ratio)
#    print_ratio(cpu_ratio, 'CPU time')
#    print_ratio(mem_ratio, 'Peak memory usage')


    tc = split(timing_cpu)
    tm = split(timing_mem)
    ic = split(ipmes_cpu)
    im = split(ipmes_mem)
    sc = split(siddhi_cpu)
    sm = split(siddhi_mem)

    graphs = ['attack', 'mix', 'benign']
    for i in range(3):
        print(f'({graphs[i]}) Average CPU time ratio:', np.average(sc[i]) / np.average(ic[i]))
        #print(f'({graphs[i]}) Average CPU time ratio:', np.average(tc[i]) / np.average(ic[i]))
        print(f'({graphs[i]}) Average peak memory usage ratio:', np.average(sm[i]) / np.average(im[i]))



if __name__ == '__main__':
    ipmes_file = 'IPMES/results/ipmes-rust/run_result.csv'
    timing_file = 'results/run_result_almost_all.csv'
    siddhi_sp_cpu_file = 'results/siddhi_sp_cpu'
    siddhi_sp_mem_file = 'results/siddhi_sp_mem'
    siddhi_dp_cpu_file = 'results/siddhi_dp_cpu'
    siddhi_dp_mem_file = 'results/siddhi_dp_mem'

    # calc_DP(ipmes_file, timing_file, siddhi_dp_cpu_file, siddhi_dp_mem_file)
    calc_DP(ipmes_file, timing_file, siddhi_dp_cpu_file, siddhi_dp_mem_file, False)


