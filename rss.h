#ifndef RSS_H
#define RSS_H

#include <stdio.h>
#include "arith.h"

#define N 6
#define N_SHARES 15
#define N_SHARES_P_SERVER 10

unsigned long long factorial(int num);

unsigned long long binomialCoefficient(int n, int k);

void generateTuples(int set[], int tuple[], int tuples[][2], int n, int t, int index, int tupleIndex, int *totalTuples);

void shareDistribution(int shareDistribution[][N_SHARES], int tuples[][2], int n, int n_shares);

void shareCounting(int shareCount[][N_SHARES], int shareDistribution[][N_SHARES], int n, int n_shares);

void shareMapping(int shareMap[][N_SHARES_P_SERVER], int shareDistribution[][N_SHARES], int n, int n_shares);

void secretSharing(f_elm_t shares[], f_elm_t secret, int n_shares);

void secretOpening(f_elm_t share[], f_elm_t secret, int n_shares);

void secretSharingServers(f_elm_t sharesServer[][N_SHARES_P_SERVER], f_elm_t shares[],
                          int shareMap[][N_SHARES_P_SERVER], int n, int n_shares_p_server);

void generateFractions(f_elm_t fractionMatrix[][N_SHARES], int shareCount[][N_SHARES], int n_shares);

void shareMultMask(f_elm_t multMaskServ[][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                   f_elm_t multMask[], int shareDistr[][N_SHARES], int n, int n_shares);

void shareElmMask(f_elm_t elmMaskServ[][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                  int shareDistr[][N_SHARES], int shareCount[][N_SHARES], int n, int n_shares);

void expandAggregateRes(f_elm_t resServ[][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                        f_elm_t expAggrRes[], int shareDistr[][N_SHARES], int n, int n_shares);

void aggregateVector(f_elm_t expAggrRes[], f_elm_t res, int n_shares);

void mergeHashes(f_elm_t hashes[][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                 f_elm_t mergedHashes[], int shareDistr[][N_SHARES], int n, int n_shares);

void compareHashes(f_elm_t expAggrRes[], f_elm_t mergedHashes[], int checkHash[], int n_shares);
#endif