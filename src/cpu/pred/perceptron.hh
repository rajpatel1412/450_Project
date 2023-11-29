/*
 * Copyright (c) 2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* @file
 * Implementation of a bi-mode branch predictor
 */

#ifndef __CPU_PRED_PERCEPTRON_PRED_HH__
#define __CPU_PRED_PERCEPTRON_PRED_HH__

#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/btb.hh"
#include "params/PerceptronBP.hh"

/**
 * Implements a bi-mode branch predictor. The bi-mode predictor is a two-level
 * branch predictor that has three seprate history arrays: a taken array, a
 * not-taken array, and a choice array. The taken/not-taken arrays are indexed
 * by a hash of the PC and the global history. The choice array is indexed by
 * the PC only. Because the taken/not-taken arrays use the same index, they
 * be the same size.
 *
 * The bi-mode branch predictor aims to eliminate the destructive aliasing that
 * occurs when two branches of opposite biases share the same global history
 * pattern. By separating the predictors into taken/not-taken arrays, and using
 * the branch's PC to choose between the two, destructive aliasing is reduced.
 */

class PerceptronBP : public BPredUnit
{
  public:
    PerceptronBP(const PerceptronBPParams *params);
    void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
    // void squash(ThreadID tid, void *bp_history, Addr branchAddr);
    void squash(ThreadID tid, void *bp_history);
    bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);
    void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
    void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed, const StaticInstPtr & inst, Addr corrTarget);

  private:
    void updateGlobalHistReg(ThreadID tid, bool taken);
    struct BPHistory {
        unsigned globalHistoryReg;
        // was the taken array's prediction used?
        // true: takenPred used
        // false: notPred used
        bool finalPred;
        std::vector<long> weights;
    };

    std::vector<unsigned> globalHistoryReg;
    std::vector<std::vector<long>> perceptrons;
    unsigned globalHistoryBits;
    unsigned historyRegisterMask;

    unsigned globalPredictorSize;
    unsigned globalHistoryMask;
    //TESTING
    int counter;
    Addr branchAddress;


    // unsigned takenThreshold;
};

#endif // __CPU_PRED_BI_MODE_PRED_HH__
