//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Weihong Xu";
const char *studentID = "A59002183";
const char *email = "wexu@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare",
                         "Tournament", "Custom"};

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

struct gshare_predictor
{
  uint32_t hist_address;
  uint8_t *hist_table;
  uint32_t address_mask;
  uint8_t last_pred_outcome;
} gshare_pred;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  if (bpType == GSHARE)
  {
    // 1. Initialize GSHARE-based predictor
    gshare_pred.hist_table = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(gshare_pred.hist_table, 0, (1 << ghistoryBits) * sizeof(uint8_t));

    uint32_t mask = 0;
    for (int i = 0; i < ghistoryBits; i++)
      mask = mask | 1 << i;

    gshare_pred.address_mask = mask;
  }
  else if (bpType == TOURNAMENT)
  {
    // 2. Initialize TOURNAMENT-based predictor
  }
  else if (bpType == CUSTOM)
  {
    // 3. Initialize CUSTOM predictor
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //
  if (bpType == GSHARE)
  {
    uint32_t masked_pc = pc & gshare_pred.address_mask;
    uint32_t masked_hist = gshare_pred.hist_address & gshare_pred.address_mask;
    uint32_t table_index = masked_pc ^ masked_hist;

    uint8_t pred_outcome = gshare_pred.hist_table[table_index] >= 2 ? TAKEN : NOTTAKEN;
    gshare_pred.last_pred_outcome = pred_outcome;
    return pred_outcome;
  }
  else if (bpType == TOURNAMENT)
  {
  }
  else if (bpType == CUSTOM)
  {
  }

  // Make a prediction based on the bpType
  switch (bpType)
  {
  case STATIC:
    return TAKEN;
  case GSHARE:
  case TOURNAMENT:
  case CUSTOM:
  default:
    break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  if (bpType == GSHARE)
  {
    // 1. Train GSHARE predictor
    uint32_t masked_pc = pc & gshare_pred.address_mask;
    uint32_t masked_hist = gshare_pred.hist_address & gshare_pred.address_mask;
    uint32_t table_index = masked_pc ^ masked_hist;

    gshare_pred.hist_address = masked_hist;

    if (outcome == TAKEN)
    {
      if (gshare_pred.hist_table[table_index] < 2)
        gshare_pred.hist_table[table_index]++;
    }
    else
    {
      if (gshare_pred.hist_table[table_index] > 0)
        gshare_pred.hist_table[table_index]--;
    }
  }
  else if (bpType == TOURNAMENT)
  {
    // 2. Train TOURNAMENT predictor
  }
}
