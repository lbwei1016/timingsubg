import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read the CSV file into a DataFrame
# timingsubg_df = pd.read_csv('./results/run_result.csv')
timingsubg_df = pd.read_csv('./results/run_result_almost_all.csv')
ipmes_df = pd.read_csv('./IPMES/results/ipmes-rust/run_result.csv')

darpa_graphs = ['dd1', 'dd2', 'dd3', 'dd4']
spade_graphs = ['attack', 'mix', 'benign']
all_graphs = spade_graphs + darpa_graphs

metrics = ['CPU Time (sec)', 'Peak Memory (MB)']

def plot_cpu_mem():
    for metric in metrics:
        timingsubg_avgs = []
        ipmes_avgs = []
        for graph in all_graphs:
            data = timingsubg_df[timingsubg_df['Data Graph'] == graph][metric]
            avg = data.sum() / data.size
            timingsubg_avgs.append(avg)

            data = ipmes_df[ipmes_df['Data Graph'] == graph][metric]
            avg = data.sum() / data.size
            ipmes_avgs.append(avg)
        
        bar_width = 0.35
        x_axis = np.arange(len(all_graphs))

        plt.figure(figsize=(10, 6))
        plt.bar(all_graphs, timingsubg_avgs, bar_width, color='skyblue', label='timingsubg')
        plt.bar(x_axis + bar_width, ipmes_avgs, bar_width, color='salmon', label='IPMES')
        plt.xticks(x_axis, all_graphs)
        plt.title(f'Average {metric} comparison between timingsubg and IPMES')
        plt.xlabel('Graph')
        plt.ylabel(f'{metric}')
        plt.legend()
        plt.tight_layout()
        plt.show()
        plt.savefig(f'./results/img/{metric}.png')

        ratio = np.array(timingsubg_avgs) / np.array(ipmes_avgs)

        plt.figure(figsize=(10, 6))
        bars = plt.bar(all_graphs, ratio, color='skyblue')
        
        for bar in bars:
            yval = bar.get_height()
            plt.text(bar.get_x() + bar.get_width()/2, yval, round(yval, 2), va='bottom')

        plt.xticks(x_axis, all_graphs)
        plt.title(f'Average {metric} ratio (timingsubg / IPMES)')
        plt.xlabel('Graph')
        plt.ylabel(f'{metric} ratio (timingsubg / IPMES)')
        plt.tight_layout()
        plt.show()
        plt.savefig(f'./results/img/{metric}_ratio_new.png')


# There are redundant answers in timingsubg
# def plot_num_results_diff():
#     num_results_diff = ipmes_df['Num Results'].to_numpy() - timingsubg_df['Num Results'].to_numpy()
#     patterns = ipmes_df['Pattern'].to_numpy()
#     data = ipmes_df['Data Graph'].to_numpy()

#     print(num_results_diff)
    
#     pattern_and_data = [p + d for (p, d) in zip(patterns, data)]

#     num_results_diff = np.delete(num_results_diff, num_results_diff.size - 1)
#     pattern_and_data = np.delete(pattern_and_data, len(pattern_and_data) - 1)

#     plt.figure(figsize=(10, 6))
#     plt.bar(pattern_and_data, num_results_diff, color='skyblue')
#     plt.title(f'Difference in Num Results (IPMES - timingsubg)')
#     plt.xlabel('Graph')
#     plt.ylabel(f'Num Results')
#     plt.tight_layout()
#     plt.show()
#     plt.savefig(f'./results/img/num_results_diff.png')


if __name__ == '__main__':
    # plot_cpu_mem()
    plot_cpu_mem()