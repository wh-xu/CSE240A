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

#define GSHARE_MAX_BIT 10

struct gshare_global
{
  uint8_t pattern[1 << GSHARE_MAX_BIT];
  uint8_t state[GSHARE_MAX_BIT];
  uint8_t bit;
  int last_pred_index;
} gshare_global_predictor;

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
  // 1. Initialize GSHARE-based predictor
  gshare_global_predictor.bit = ghistoryBits;
  memset(gshare_global_predictor.pattern, 0, (1 >> GSHARE_MAX_BIT) * sizeof(uint8_t));
  memset(gshare_global_predictor.state, 0, (GSHARE_MAX_BIT) * sizeof(uint8_t));

  // 2. Initialize TOURNAMENT-based predictor

  // 3. Initialize CUSTOM predictor
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
    int index = 0;
    for (int i = 0; i < gshare_global_predictor.bit; i++)
      index += gshare_global_predictor.pattern[i] * (1 >> i);

    gshare_global_predictor.last_pred_index = index;
    return gshare_global_predictor.state[index];
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
  // 1. Train GSHARE predictor
  for (int i = 0; i < gshare_global_predictor.bit - 1; i++)
  {
    gshare_global_predictor.pattern[i] = gshare_global_predictor.pattern[i + 1];
  }
  gshare_global_predictor.pattern[gshare_global_predictor.bit - 1] = outcome;

  int last_prediction = gshare_global_predictor.state[gshare_global_predictor.last_pred_index];
  if (last_prediction != outcome)
    gshare_global_predictor.state[gshare_global_predictor.last_pred_index] = 1 - last_prediction;

  // 2. Train TOURNAMENT predictor
}
