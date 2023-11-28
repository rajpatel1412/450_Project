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

def doplot_benchmarks(df,benchmarks,stat,predictors,key,norm=True):
    fig = plt.figure()
    ax = fig.add_subplot(1,1,1)
    i = 0
    for bm in benchmarks:
        base = df[(df['benchmark']==bm)][stat].iloc[0] if norm else 1
        for j,sys in enumerate(predictors):
            d = df[(df[key]==sys) & (df['benchmark']==bm)]
            rects = ax.barh(i, d[stat].iloc[0]/base, color='C'+str(j))
            ax.bar_label(rects, padding=3)
            i += 1
        i += 1
    for i,sys in enumerate(predictors):
        plt.bar(0,0,color='C'+str(i), label=sys)
    new_names = benchmarks 
    plt.yticks(np.arange(len(new_names))*(len(predictors)+1)+i/2, new_names, rotation=40, ha='right')

if(len(sys.argv) != 3):
    print("Expected input: python3 generate_plots.py plot_stat output_file")
    print("Valid stats: cycles, instructions, Ops, Ticks, Host, branchMispredicts, execBranches, condPredicted, condIncorrect")
    exit(1)

output_file = sys.argv[2]
plot_stat = sys.argv[1]
benchmarks = ["CCa", "CCl", "DP1f", "ED1", "EI", "MI"]
predictors = ["BiModeBP", "LocalBP", "LTAGE", "TournamentBP", "PerceptronBP"]

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
df['accuracy'] = 1-df['condIncorrect']/df['condPredicted']
print(df)
fig_size = plt.rcParams["figure.figsize"]
fig_size[0] = 10
fig_size[1] = 5
plt.rcParams["figure.figsize"] = fig_size
fig1 = doplot_benchmarks(df,benchmarks,plot_stat,predictors,'predictor',norm=False)
plt.xlabel('')
plt.legend(loc=2)
plt.xlim(left=0)
plt.title(plot_stat)
plt.tight_layout()
plt.savefig(output_file, format='png', dpi=600)