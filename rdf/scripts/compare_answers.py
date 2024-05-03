import re
import os

def timingsubg_extract_matches(filename: str) -> set[str]:
    matches = set()
    with open(filename, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if line == '\n': continue
            match = ''
            pairs = line.split('; ')
            for pair in pairs:
                if pair == '\n': break
                entity_id = re.sub(r'\[.*, \(\(data\) id: (\d+)\)\]', r'\1', pair)
                match += entity_id + ', '
            match = match.rstrip(', ')
            matches.add(match)
    return matches


def ipmes_extract_matches(filename: str) -> set[str]:
    matches = set()
    with open(filename, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if 'Pattern Match' not in line: continue
            entity_ids = re.sub(r'.* Pattern Match: \[(.*)\]', r'\1', line)
            matches.add(entity_ids.strip('\n'))
    return matches   

if __name__ == '__main__':
    if os.getcwd().endswith('scripts'):
        os.chdir('..')
    timingsubg_answer_path = './results/answers'
    ipmes_answer_path = 'IPMES/ipmes-rust/output_logs'

    Darpa = [f'DP{i}_regex' for i in range(1, 6)]
    Spade = [f'SP{i}_regex' for i in range(1, 13)]
    darpa_graphs = ['dd1', 'dd2', 'dd3', 'dd4']
    spade_graphs = ['attack', 'mix', 'benign']

    for i in range(1, 13):
        for graph in spade_graphs:
            timingsubg_matches = timingsubg_extract_matches(os.path.join(timingsubg_answer_path, f'SP{i}_regex_{graph}'))
            ipmes_matches = ipmes_extract_matches(os.path.join(ipmes_answer_path, f'SP{i}_regex_{graph}.log'))

            target = f'SP{i}-{graph}'
            if timingsubg_matches == ipmes_matches:
                print(f'{target}: Identical answers!')
            elif timingsubg_matches.issubset(ipmes_matches):
                print(f'{target}: timingsubg is a subset of ipmes')
                print(f'Differences: {ipmes_matches.difference(timingsubg_matches)}')
            elif ipmes_matches.issubset(timingsubg_matches):
                print(f'{target}: ipmes is a subset of timingsubg')
            else:
                print(f'{target}: ipmes\\timingsubg non-empty!')
            print(f'(timingsubg, ipmes): {len(timingsubg_matches), len(ipmes_matches)}')
            print('--------------------')

            input("Press enter to continue...")

    for i in range(1, 6):
        for graph in darpa_graphs:
            timingsubg_matches = timingsubg_extract_matches(os.path.join(timingsubg_answer_path, f'DP{i}_regex_{graph}'))
            if len(timingsubg_matches) > 0:
                ipmes_matches = ipmes_extract_matches(os.path.join(ipmes_answer_path, f'DP{i}_regex_{graph}.log'))
            else: 
                ipmes_matches = {1}

            target = f'DP{i}-{graph}'
            if timingsubg_matches == ipmes_matches:
                print(f'{target}: Identical answers!')
            elif timingsubg_matches.issubset(ipmes_matches):
                print(f'{target}: timingsubg is a subset of ipmes')
                print(f'Differences: {ipmes_matches.difference(timingsubg_matches)}')
            elif ipmes_matches.issubset(timingsubg_matches):
                print(f'{target}: ipmes is a subset of timingsubg')
            else:
                print(f'{target}: ipmes\\timingsubg non-empty!')
