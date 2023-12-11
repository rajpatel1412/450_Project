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
#include "cpu/pred/perceptron.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"

PerceptronBP::PerceptronBP(const PerceptronBPParams *params)
    : BPredUnit(params),
      globalHistoryReg(params->numThreads, 0),
      globalHistoryBits(ceilLog2(params->globalPredictorSize)),
      globalPredictorSize(params->globalPredictorSize)
{
    //std::cout << "Perceptron Constructor" << std::endl;
    if (!isPowerOf2(globalPredictorSize))
        fatal("Invalid global history predictor size.\n");

    historyRegisterMask = mask(globalHistoryBits);
    globalHistoryMask = globalPredictorSize - 1;
    perceptrons.resize(globalPredictorSize);
    for (int i = 0; i < globalPredictorSize; i++) {
        perceptrons[i].resize(globalHistoryBits);
        for (int j = 0; j < globalHistoryBits; j++) {
            perceptrons[i][j] = 0;
        }
    }
   counter = 0;
   branchAddress = 0;
   avgPred = 0;
}

/*
 * For an unconditional branch we set its history such that
 * everything is set to taken. I.e., its choice predictor
 * chooses the taken array and the taken array predicts taken.
 */
void
PerceptronBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    //std::cout << "Perceptron uncondBranch" << std::endl;
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->finalPred = true;
    history->weights.resize(globalHistoryBits);
    for (int i = 0; i < history->weights.size(); i++)
    {
        history->weights[i] = 1;
    }
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, true);
}

void
// PerceptronBP::squash(ThreadID tid, void *bpHistory, Addr branchAddr)
PerceptronBP::squash(ThreadID tid, void *bpHistory)
{
    //std::cout << "Perceptron squash" << std::endl;
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    globalHistoryReg[tid] = history->globalHistoryReg;
    // unsigned globalHistoryIdx = (((branchAddr >> instShiftAmt)
    //                             ^ globalHistoryReg[tid])
    //                             & globalHistoryMask);
    // assert(globalHistoryIdx < globalPredictorSize);
    // for (int i = 0; i < perceptrons[globalHistoryIdx].size(); i++)
    // {
    //     perceptrons[globalHistoryIdx][i] = history->weights[i];
    // }
    delete history;
}

/*
 * Here we lookup the actual branch prediction. We use the PC to
 * identify the bias of a particular branch, which is based on the
 * prediction in the choice array. A hash of the global history
 * register and a branch's PC is used to index into both the taken
 * and not-taken predictors, which both present a prediction. The
 * choice array's prediction is used to select between the two
 * direction predictors for the final branch prediction.
 */
bool
PerceptronBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    //std::cout << "Perceptron lookup" << std::endl;
    unsigned globalHistoryIdx = (((branchAddr >> instShiftAmt)
                                ^ globalHistoryReg[tid])
                                & globalHistoryMask);

    assert(globalHistoryIdx < globalPredictorSize);

    int prediction_output = 0;
    for (int i = 0; i < perceptrons[globalHistoryIdx].size(); i++)
    {
        prediction_output += (perceptrons[globalHistoryIdx][i]
                             * ((globalHistoryReg[tid]
                             >> perceptrons[globalHistoryIdx].size()-i-1)
                             & 1));

    }
    // avgPred += prediction_output;
    // counter++;

    // if ((counter % 5000) == 0) {
    //     std::cout << "AVG PREDICITON OUTPUT: " << avgPred / counter << std::endl;
    //     avgPred /= counter;
    //     avgPred /= 4;
    // }
        
    
    bool finalPrediction = (prediction_output >= -200) ? true : false;

    // if ((counter % 10000) == 0) {
    //     avgPred = 0;
    //     counter = 0;
    // }

    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->weights.resize(globalHistoryBits);
    for (int i = 0; i < perceptrons[globalHistoryIdx].size(); i++)
    {
        history->weights[i] = perceptrons[globalHistoryIdx][i];
    }
    history->finalPred = finalPrediction;
    bpHistory = static_cast<void*>(history);

    //TESTING
    // if (counter >= 2000 && counter <= 2050)
    // {
    //     // std::cout << "LOOKUP \t";
    //     // std::cout << "BA: " << branchAddr << "\t";
    //     // std::cout << "IDX: " << globalHistoryIdx << "\t";
    //     // std::cout << "FINAL: " << finalPrediction << "\t";
    //     // std::cout << "GLOBAL HIST: " << globalHistoryReg[tid] << "\t";
    //     // std::cout << "WEIGHTS: \t";
    //     // for (int i = 0; i < perceptrons[globalHistoryIdx].size(); i++)
    //     {
    //         std::cout << history->weights[i] << "\t";
    //     }
    //     std::cout << "OUTPUT: " << prediction_output << "\t";
    //     branchAddress = branchAddr;
    //     std::cout << std::endl;
    // }
    // counter++;
    //TESTING

    updateGlobalHistReg(tid, finalPrediction);

    return finalPrediction;
}

void
PerceptronBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    //std::cout << "Perceptron btbUpdate" << std::endl;
    globalHistoryReg[tid] &= (historyRegisterMask & ~ULL(1));
}

/* Only the selected direction predictor will be updated with the final
 * outcome; the status of the unselected one will not be altered. The choice
 * predictor is always updated with the branch outcome, except when the
 * choice is opposite to the branch outcome but the selected counter of
 * the direction predictors makes a correct final prediction.
 */
void
PerceptronBP::update(ThreadID tid, Addr branchAddr, bool taken,
                    void *bpHistory, bool squashed, const StaticInstPtr &
                    inst, Addr corrTarget)
{
    //std::cout << "Perceptron update" << std::endl;
    assert(bpHistory);

    BPHistory *history = static_cast<BPHistory*>(bpHistory);

    // We do not update the counters speculatively on a squash.
    // We just restore the global history register.
    if (squashed) {
        //std::cout << "squashed" << std::endl;
        globalHistoryReg[tid] = (history->globalHistoryReg << 1) | taken;
        return;
    }

    unsigned globalHistoryIdx = (((branchAddr >> instShiftAmt)
                                ^ history->globalHistoryReg)
                                & globalHistoryMask);

    assert(globalHistoryIdx < globalPredictorSize);


    // if (history->finalPred == taken) {
    //    /* If the final prediction matches the actual branch's
    //     * outcome and the choice predictor matches the final
    //     * outcome, we update the choice predictor, otherwise it
    //     * is not updated. While the designers of the bi-mode
    //     * predictor don't explicity say why this is done, one
    //     * can infer that it is to preserve the choice predictor's
    //     * bias with respect to the branch being predicted; afterall,
    //     * the whole point of the bi-mode predictor is to identify the
    //     * atypical case when a branch deviates from its bias.
    //     */
    //     //std::cout << "taken" << std::endl;
    //     //std::cout << perceptrons[globalHistoryIdx].size() << " " << history->weights.size() << std::endl;
        
    //     for (int i = 0; i < perceptrons[globalHistoryIdx].size(); i++)
    //     {
    //         perceptrons[globalHistoryIdx][i] = history->weights[i] + 1;
    //     }
    // }
    // else
    // {
    //     //std::cout << "not taken" << std::endl;
    //     for (int i = 0; i < perceptrons[globalHistoryIdx].size(); i++)
    //     {
    //         perceptrons[globalHistoryIdx][i] = history->weights[i] - 1;
    //     }
    // }

    for (int i = 0; i < perceptrons[globalHistoryIdx].size(); i++)
    {
        int ghb = ((globalHistoryReg[tid] >> perceptrons[globalHistoryIdx].size()-i-1) & 1);
        if (history->finalPred == ghb) {
            perceptrons[globalHistoryIdx][i] = history->weights[i] + 1;
        }
        else {
            perceptrons[globalHistoryIdx][i] = history->weights[i] - 1;
        }
    }

    // //TESTING
    // if (counter >= 2000 && counter <= 2050)
    // {
    //     std::cout << "UPDATE \t";
    //     std::cout << "BA: " << branchAddr << "\t";
    //     std::cout << "IDX: " << globalHistoryIdx << "\t";
    //     std::cout << "PRED: " << history->finalPred << "\t";
    //     std::cout << "TAKEN: " << taken << "\t";
    //     std::cout << "GLOBAL HIST: " << globalHistoryReg[tid] << "\t";
    //     std::cout << "OLD WEIGHTS: \t";
    //     for (int i = 0; i < perceptrons[globalHistoryIdx].size(); i++)
    //     {
    //         std::cout << history->weights[i] << "\t";
    //     }
    //     std::cout << std::endl;
    //     std::cout << "NEW WEIGHTS: \t";
    //     for (int i = 0; i < perceptrons[globalHistoryIdx].size(); i++)
    //     {
    //         std::cout << perceptrons[globalHistoryIdx][i] << "\t";
    //     }
    //     std::cout << std::endl;
    // }
    // counter++;
    // //TESTING

    delete history;
}

void
PerceptronBP::updateGlobalHistReg(ThreadID tid, bool taken)
{
    //std::cout << "Perceptron updateGlobalHistReg" << std::endl;
    globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    globalHistoryReg[tid] &= historyRegisterMask;
}

PerceptronBP*
PerceptronBPParams::create()
{
    return new PerceptronBP(this);
}
