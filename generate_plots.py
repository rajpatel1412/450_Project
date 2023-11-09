import sys
import os
import re
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

datadir = 'results/X86/run_micro'

def gem5GetStat(filename, stat):
    filename = os.path.join(datadir, '', filename, 'stats.txt').replace('\\','/')
    with open(filename) as f:
        r = f.read()
        if len(r) < 10: return 0.0
        if (r.find(stat) != -1) :
            start = r.find(stat) + len(stat) + 1
            end = r.find('#', start)
            #print(r[start:end])
            return float(r[start:end])
        else:
            return float(0.0)

output_file = sys.argv[len(sys.argv)-1]
benchmarks = ["CCa", "CCl", "DP1f", "ED1", "EI", "MI"]
predictors = ["BiModeBP", "LocalBP", "LTAGE", "TournamentBP"]

rows = []
for bm in benchmarks: 
    for predictor in predictors:
        rows.append([bm,predictor,
            gem5GetStat(bm+"/"+predictor, 'system.cpu.numCycles'),
            gem5GetStat(bm+"/"+predictor, 'sim_insts'),
            gem5GetStat(bm+"/"+predictor, 'sim_ops'),
            gem5GetStat(bm+"/"+predictor, 'sim_ticks')/1e9,
            gem5GetStat(bm+"/"+predictor, 'host_op_rate'),
            gem5GetStat(bm+"/"+predictor, 'system.cpu.iew.branchMispredicts'),
            gem5GetStat(bm+"/"+predictor, 'system.cpu.iew.exec_branches'),
            gem5GetStat(bm+"/"+predictor, 'system.cpu.branchPred.condPredicted'),
            gem5GetStat(bm+"/"+predictor, 'system.cpu.branchPred.condIncorrect')
        ])
df = pd.DataFrame(rows, columns=['benchmark','predictor', 'cycles','instructions', 'Ops', 'Ticks','Host', 'branchMispredicts', 'execBranches', 'condPredicted', 'condIncorrect'])
df['ipc'] = df['instructions']/df['cycles']
df['cpi']= 1/df['ipc']
print(df)