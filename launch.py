from run import gem5Run
import os
import sys
from uuid import UUID
from itertools import starmap
from itertools import product
import multiprocessing as mp
import argparse

def main(args):
    bp_types = ['LTAGE', 'LocalBP', 'TournamentBP', 'BiModeBP',
    'PerceptronBP']
    jobs = []
    bm_list = ['600.perlbench_s', '602.gcc_s', '625.x264_s', '641.leela_s', '648.exchange2_s']
    for bm in bm_list:
        for bp in bp_types:
            run = gem5Run.createSERun(
                'spec_cpu_2017_tests',
                'build/X86/gem5.opt',
                'gem5-config/run_micro.py',
                'results/X86/run_micro/{}/{}'.format(bm,bp),
                bp,
                os.path.join('/data/shared/spec2017/spec2017/benchspec/CPU',bm))
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
    args  = parser.parse_args()
    main(args)
