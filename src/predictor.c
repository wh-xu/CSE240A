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

struct tournament_predictor
{
  uint8_t *global_pred_table;
  uint32_t prev_global_branches;
  uint32_t global_pc_mask;

  uint32_t *local_hist_table;
  uint8_t *local_pred_table;
  uint32_t prev_local_branches;
  uint32_t local_pc_mask;

  uint8_t *chooser;
  uint8_t last_choice;

  uint32_t pc_mask;
  uint8_t last_local_pred;
  uint8_t last_global_pred;
} tour_pred;

#define USE_COMBINED_NN

#ifndef USE_COMBINED_NN
struct nn_predictor
{
  uint32_t pc_mask;
  int8_t *weights;
  uint8_t history_len;

  uint64_t gl_hist_mask;
  uint64_t global_hist_reg;

  uint8_t threshold;
  uint8_t last_pred;
} nn_pred;

const uint32_t nn_hist_len = 31;
const uint32_t pc_ind_len = 11;

#else

struct combined_nn_predictor
{
  uint32_t *BHT;
  uint32_t BHT_mask;

  uint64_t gl_hist_mask;
  uint64_t global_hist_reg;

  uint32_t hist_weight_pc_mask;
  uint32_t addr_weight_pc_mask;

  int8_t *weight_add;
  int8_t *weight_hist;

  uint8_t history_len;
  int8_t threshold;
  int32_t last_pred;
} com_nn_pred;

const uint32_t BHT_addr_len = 11;
const uint32_t _nn_global_hist_len = 30;
const uint32_t _nn_local_hist_len = 11;
#define hist_weight_len _nn_global_hist_len + _nn_local_hist_len

const uint32_t addr_weight_len = 12;

const uint32_t hist_weight_pc_len = 8;
const uint32_t addr_weight_pc_len = 8;

const uint32_t THETA = 64;

#endif

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
    memset(gshare_pred.hist_table, WN, (1 << ghistoryBits) * sizeof(uint8_t));

    uint32_t mask = 0;
    for (int i = 0; i < ghistoryBits; i++)
      mask = mask | 1 << i;

    gshare_pred.address_mask = mask;
  }
  else if (bpType == TOURNAMENT)
  {
    // 2. Initialize TOURNAMENT-based predictor
    tour_pred.global_pred_table = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(tour_pred.global_pred_table, 0, (1 << ghistoryBits) * sizeof(uint8_t));
    tour_pred.prev_global_branches = 0;

    tour_pred.local_hist_table = malloc((1 << pcIndexBits) * sizeof(uint32_t));
    memset(tour_pred.local_hist_table, 0, (1 << pcIndexBits) * sizeof(uint32_t));
    tour_pred.local_pred_table = malloc((1 << lhistoryBits) * sizeof(uint8_t));
    memset(tour_pred.local_pred_table, 0, (1 << lhistoryBits) * sizeof(uint8_t));
    tour_pred.prev_local_branches = 0;

    uint32_t mask = 0;
    for (int i = 0; i < pcIndexBits; i++)
      mask = mask | 1 << i;
    tour_pred.pc_mask = mask;

    mask = 0;
    for (int i = 0; i < ghistoryBits; i++)
      mask = mask | 1 << i;
    tour_pred.global_pc_mask = mask;

    mask = 0;
    for (int i = 0; i < lhistoryBits; i++)
      mask = mask | 1 << i;
    tour_pred.local_pc_mask = mask;

    tour_pred.chooser = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(tour_pred.chooser, WN, (1 << ghistoryBits) * sizeof(uint8_t));
  }
  else if (bpType == CUSTOM)
  {
// 3. Initialize CUSTOM predictor
#ifndef USE_COMBINED_NN
    uint64_t mask = 0;
    for (int i = 0; i < pc_ind_len; i++)
      mask = mask | 1 << i;
    nn_pred.pc_mask = mask;

    mask = 0;
    mask = mask | 1;
    nn_pred.gl_hist_mask = mask;

    nn_pred.weights = malloc(((1 << pc_ind_len) * nn_hist_len) * sizeof(int8_t));
    memset(nn_pred.weights, 0, ((1 << pc_ind_len) * nn_hist_len) * sizeof(int8_t));

    nn_pred.global_hist_reg = 0;
    nn_pred.threshold = 1.93 * nn_hist_len + 14;

#else

    com_nn_pred.BHT_mask = 0;
    for (int i = 0; i < BHT_addr_len; i++)
      com_nn_pred.BHT_mask = com_nn_pred.BHT_mask | 1 << i;

    com_nn_pred.BHT = malloc((1 << BHT_addr_len) * sizeof(uint32_t));
    memset(com_nn_pred.BHT, 0, (1 << BHT_addr_len) * sizeof(uint32_t));

    com_nn_pred.gl_hist_mask = 0;
    com_nn_pred.gl_hist_mask = com_nn_pred.gl_hist_mask | 1;

    com_nn_pred.hist_weight_pc_mask = 0;
    for (int i = 0; i < hist_weight_pc_len; i++)
      com_nn_pred.hist_weight_pc_mask = com_nn_pred.hist_weight_pc_mask | 1 << i;

    com_nn_pred.addr_weight_pc_mask = 0;
    for (int i = 0; i < addr_weight_pc_len; i++)
      com_nn_pred.addr_weight_pc_mask = com_nn_pred.addr_weight_pc_mask | 1 << i;

    com_nn_pred.weight_hist = malloc(((1 << hist_weight_pc_len) * hist_weight_len) * sizeof(int8_t));
    memset(com_nn_pred.weight_hist, 0, ((1 << hist_weight_pc_len) * hist_weight_len) * sizeof(int8_t));

    com_nn_pred.weight_add = malloc(((1 << addr_weight_pc_len) * addr_weight_len) * sizeof(int8_t));
    memset(com_nn_pred.weight_add, 0, ((1 << addr_weight_pc_len) * addr_weight_len) * sizeof(int8_t));

    com_nn_pred.global_hist_reg = 0;
    com_nn_pred.threshold = 0.0;
    // com_nn_pred.threshold = 1.93 * hist_weight_len + 14;

#endif
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
    uint32_t local_pred_table_index = tour_pred.local_hist_table[pc & tour_pred.pc_mask];
    uint8_t local_prediction = tour_pred.local_pred_table[local_pred_table_index] >= 2 ? TAKEN : NOTTAKEN;
    tour_pred.last_local_pred = local_prediction;

    uint32_t path_history = tour_pred.prev_global_branches & tour_pred.global_pc_mask;
    uint8_t global_prediction = tour_pred.global_pred_table[path_history] >= 2 ? TAKEN : NOTTAKEN;
    tour_pred.last_global_pred = global_prediction;

    uint8_t predictor_choice = tour_pred.chooser[path_history];
    tour_pred.last_choice = predictor_choice;

    return predictor_choice >= 2 ? global_prediction : local_prediction;
  }
  else if (bpType == CUSTOM)
  {
#ifndef USE_COMBINED_NN

    uint32_t weight_index = pc & nn_pred.pc_mask;
    int8_t *W = nn_pred.weights + weight_index * nn_hist_len;

    // Compute y
    int32_t y = 1;
    for (int i = 0; i < nn_hist_len; i++)
    {
      if ((nn_pred.global_hist_reg >> i) & nn_pred.gl_hist_mask == TAKEN)
        y += W[i];
      else
        y -= W[i];
    }

    uint8_t prediction = y > nn_pred.threshold ? TAKEN : NOTTAKEN;
    nn_pred.last_pred = prediction;
    return prediction;

#else
    // Compute y_addr
    int32_t y_addr = 1;
    uint32_t addr_weight_index = pc & com_nn_pred.addr_weight_pc_mask;
    int8_t *W_addr = com_nn_pred.weight_add + addr_weight_index * addr_weight_len;
    for (int i = 0; i < addr_weight_len; i++)
    {
      if (((addr_weight_index >> i) & ((uint32_t)(1))) == TAKEN)
        y_addr += W_addr[i];
      else
        y_addr -= W_addr[i];
    }

    // Compute y_hist
    uint32_t BHT_index = pc & com_nn_pred.BHT_mask;
    uint32_t local_hist = com_nn_pred.BHT[BHT_index];

    int32_t y_hist = 1;
    uint32_t hist_weight_index = pc & com_nn_pred.hist_weight_pc_mask;
    int8_t *W_hist = com_nn_pred.weight_hist + hist_weight_index * hist_weight_len;
    for (int i = 0; i < hist_weight_len; i++)
    {
      if (i < _nn_global_hist_len)
      {
        if (((com_nn_pred.global_hist_reg >> i) & com_nn_pred.gl_hist_mask) == TAKEN)
          y_hist += W_hist[i];
        else
          y_hist -= W_hist[i];
      }
      else
      {
        if (((local_hist >> (i - _nn_global_hist_len)) & ((uint32_t)(1))) == TAKEN)
          y_hist += W_hist[i];
        else
          y_hist -= W_hist[i];
      }
    }

    int32_t y_pred = y_hist + y_addr;
    com_nn_pred.last_pred = y_pred;
    return y_pred > com_nn_pred.threshold ? TAKEN : NOTTAKEN;

#endif
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
      if (gshare_pred.hist_table[table_index] < ST)
        gshare_pred.hist_table[table_index]++;
    }
    else
    {
      if (gshare_pred.hist_table[table_index] > SN)
        gshare_pred.hist_table[table_index]--;
    }
  }
  else if (bpType == TOURNAMENT)
  {
    // 2. Train TOURNAMENT predictor
    uint32_t local_pred_table_index = tour_pred.local_hist_table[pc & tour_pred.pc_mask];
    uint32_t path_history = tour_pred.prev_global_branches & tour_pred.global_pc_mask;
    uint8_t predictor_choice = tour_pred.chooser[path_history];

    // Updating global/local predictors
    if (outcome == TAKEN)
    {
      if (tour_pred.global_pred_table[path_history] < ST)
        tour_pred.global_pred_table[path_history]++;

      if (tour_pred.local_pred_table[local_pred_table_index] < ST)
        tour_pred.local_pred_table[local_pred_table_index]++;
    }
    else
    {
      if (tour_pred.global_pred_table[path_history] > SN)
        tour_pred.global_pred_table[path_history]--;

      if (tour_pred.local_pred_table[local_pred_table_index] > SN)
        tour_pred.local_pred_table[local_pred_table_index]--;
    }

    // Updating path history / local history table
    tour_pred.prev_global_branches = ((tour_pred.prev_global_branches << 1) + outcome) & tour_pred.global_pc_mask;

    if (tour_pred.last_local_pred == outcome)
      tour_pred.local_hist_table[pc & tour_pred.pc_mask] = (tour_pred.local_hist_table[pc & tour_pred.pc_mask] << 1 + outcome) & tour_pred.local_pc_mask;

    // // Updating chooser
    if (tour_pred.last_local_pred != tour_pred.last_global_pred)
    {
      if (tour_pred.last_global_pred == outcome)
      {
        if (predictor_choice < 3)
          tour_pred.chooser[path_history]++;
      }
      else
      {
        if (predictor_choice > 0)
          tour_pred.chooser[path_history]--;
      }
    }
  }
  else if (bpType == CUSTOM)
  {
#ifndef USE_COMBINED_NN

    if (nn_pred.last_pred != outcome)
    {
      uint32_t weight_index = pc & nn_pred.pc_mask;
      int8_t *W = nn_pred.weights + weight_index * nn_hist_len;

      for (int i = 0; i < nn_hist_len; i++)
      {
        if (outcome == ((nn_pred.global_hist_reg >> i) & nn_pred.gl_hist_mask))
          W[i] += 1;
        else
          W[i] -= 1;
      }
    }

    nn_pred.global_hist_reg = nn_pred.global_hist_reg << 1 | outcome;

#else
    uint8_t last_pred = com_nn_pred.last_pred > com_nn_pred.threshold ? TAKEN : NOTTAKEN;
    uint32_t BHT_index = pc & com_nn_pred.BHT_mask;
    uint32_t local_hist = com_nn_pred.BHT[BHT_index];

    if (last_pred != outcome)
    {
      // Update the history-based perceptron
      uint32_t hist_weight_index = pc & com_nn_pred.hist_weight_pc_mask;
      int8_t *W_hist = com_nn_pred.weight_hist + hist_weight_index * hist_weight_len;
      for (int i = 0; i < hist_weight_len; i++)
      {
        if (i < _nn_global_hist_len)
        {
          if (((com_nn_pred.global_hist_reg >> i) & com_nn_pred.gl_hist_mask) == outcome)
            W_hist[i] += 1;
          else
            W_hist[i] -= 1;
        }
        else
        {
          if (((local_hist >> (i - _nn_global_hist_len)) & ((uint32_t)(1))) == outcome)
            W_hist[i] += 1;
          else
            W_hist[i] -= 1;
        }
      }

      // Update the address-based perceptron
      int32_t y_addr = 1;
      uint32_t addr_weight_index = pc & com_nn_pred.addr_weight_pc_mask;
      int8_t *W_addr = com_nn_pred.weight_add + addr_weight_index * addr_weight_len;
      for (int i = 0; i < addr_weight_len; i++)
      {
        if (((addr_weight_index >> i) & ((uint32_t)(1))) == outcome)
          W_addr[i] += 1;
        else
          W_addr[i] -= 1;
      }
    }

    com_nn_pred.BHT[BHT_index] = com_nn_pred.BHT[BHT_index] << 1 | outcome;
    com_nn_pred.global_hist_reg = com_nn_pred.global_hist_reg << 1 | outcome;

#endif
  }
}
