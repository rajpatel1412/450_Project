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
 * Implementation of a perceptron branch predictor
 */

#ifndef __CPU_PRED_PERCEPTRON_PRED_HH__
#define __CPU_PRED_PERCEPTRON_PRED_HH__

#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/btb.hh"
#include "params/PerceptronBP.hh"

/**
 * Implements a perceptron branch predictor. The perceptron predictor uses a 
 * branch predictor table and assigns a set of weights to each entry. 
 * These weights can be multiplied with each bit in the global history register 
 * and summed up together to make a prediction. Essentially, this is a dot product 
 * between the weights and the corresponding bits on the global history register. 
 * The table is indexed by a combination of the branch address and the global history register. 
 * Upon receiving the actual outcome of the branch the weights are updated; each weight on the branch is 
 * compared with each global history bit, if they are equal the corresponding weight is increased by 1; 
 * if not the weight is decreased by 1.
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
    long avgPred;

    // unsigned takenThreshold;
};

#endif // __CPU_PRED_BI_MODE_PRED_HH__
