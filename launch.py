from run import gem5Run
import os
import sys
from uuid import UUID
from itertools import starmap
from itertools import product
import multiprocessing as mp
import argparse

def experiment_1(args,bm_list):
    bp_types = ['LTAGE', 'LocalBP', 'TournamentBP', 'BiModeBP']
    jobs = []
    for bm in bm_list:
        for bp in bp_types:
            run = gem5Run.createSERun(
                'microbench_tests',
                'build/X86/gem5.opt',
                'gem5-config/run_micro.py',
                'results/X86/run_micro/{}/{}'.format(bm,bp),
                bp,
                os.path.join('microbenchmark',bm,'bench.X86'))
            jobs.append(run)

    with mp.Pool(args.N) as pool:
        pool.map(worker,jobs)

def worker(run):
    run.run()
    json = run.dumpsJson()
    print(json)

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument('N', action="store",
                      default=1, type=int,
                      help = """Number of cores used for simulation""")
    parser.add_argument('Experiment', action="store",
                      default='1', type=str,
                      help = """Experiment number""")
    args  = parser.parse_args()
    bm_list = []

    # iterate through files in microbench dir to
    # create a list of all microbenchmarks

    for filename in os.listdir('microbenchmark'):
        if os.path.isdir(f'microbenchmark/{filename}') and filename != '.git':
            bm_list.append(filename)
    globals()["experiment_"+args.Experiment](args, bm_list)
