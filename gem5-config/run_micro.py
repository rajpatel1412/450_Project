# Derived from gem5-art

from __future__ import print_function

import argparse
import m5
from m5.objects import DerivO3CPU
from m5.objects import LTAGE, LocalBP, TournamentBP, BiModeBP, PerceptronBP
from m5.objects import Root
from m5.objects import *
import time
from system import BaseTestSystem

class LTAGECPU(DerivO3CPU):
    branchPred = LTAGE()

class LocalBPCPU(DerivO3CPU):
    branchPred = LocalBP()

class TournamentBPCPU(DerivO3CPU):
    branchPred = TournamentBP()

class BiModeBPCPU(DerivO3CPU):
    branchPred = BiModeBP()

class PerceptronBPCPU(DeriveO3CPU):
    branchPred = PerceptronBP()

# Add more CPUs under test before this
valid_cpus = [LTAGECPU, LocalBPCPU, TournamentBPCPU, BiModeBPCPU,
              PerceptronBPCPU]
valid_cpus = {cls.__name__[:-3]:cls for cls in valid_cpus}

parser = argparse.ArgumentParser()
parser.add_argument('cpu', choices = valid_cpus.keys())
parser.add_argument('binary', type = str, help = "Path to binary to run")

args  = parser.parse_args()
class MySystem(BaseTestSystem):
    _CPUModel = valid_cpus[args.cpu]
system = MySystem()
system.setTestBinary(args.binary)
root = Root(full_system = False, system = system)
m5.instantiate()

start_tick = m5.curTick()
start_insts = system.totalInsts()
globalStart = time.time()
exit_event = m5.simulate()

print("Exit Event" + exit_event.getCause())
if exit_event.getCause() == "workbegin":
    # Reached the start of ROI
    # start of ROI is marked by an
    # m5_work_begin() call
    m5.stats.reset()
    start_tick = m5.curTick()
    start_insts = system.totalInsts()
    print("Resetting stats at the start of ROI!")
    exit_event = m5.simulate()

# Reached the end of ROI
# Finish executing the benchmark with kvm cpu
if exit_event.getCause() == "workend":
    # Reached the end of ROI
    # end of ROI is marked by an
    # m5_work_end() call
    print("Dump stats at the end of the ROI!")
    m5.stats.dump()
    end_tick = m5.curTick()
    end_insts = system.totalInsts()
    m5.stats.reset()
else:
    print("Terminated simulation before reaching ROI!")
    m5.stats.dump()
    end_tick = m5.curTick()
    end_insts = system.totalInsts()
    print("Performance statistics:")
    print("Simulated time: %.2fs" % ((end_tick-start_tick)/1e12))
    print("Instructions executed: %d" % ((end_insts-start_insts)))
    print("Ran a total of", m5.curTick()/1e12, "simulated seconds")
    print("Total wallclock time: %.2fs, %.2f min" % \
        (time.time()-globalStart, (time.time()-globalStart)/60))
exit()