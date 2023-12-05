#!/bin/bash
#
#SBATCH --cpus-per-task=8
#SBATCH --time=60:00
#SBATCH --mem=40G

scons -j 8 build/X86/gem5.opt CPU_MODELS='O3CPU,TimingSimpleCPU,MinorCPU' --gold-linker