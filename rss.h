#ifndef RSS_H
#define RSS_H

#include <stdio.h>
#include "arith.h"

#define N 6
#define N_SHARES 15
#define N_SHARES_P_SERVER 10
#define N_BITS 4
#define N_TRANSMIT 1

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

void keySharingServersN(f_elm_t sharesServer[][N_BITS][N_SHARES_P_SERVER], f_elm_t keys[],
                        int shareMap[][N_SHARES_P_SERVER], int n, int n_shares_p_server);

void generateFractions(f_elm_t fractionMatrix[][N_SHARES], int shareCount[][N_SHARES], int n_shares);

void shareMultMaskN(f_elm_t multMaskServ[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                    int shareDistr[][N_SHARES], int n, int n_shares);

void shareElmMaskN(f_elm_t elmMaskServ[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                   int shareDistr[][N_SHARES], int shareCount[][N_SHARES], int n, int n_shares);

void expandAggregateResN(f_elm_t resServ[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                         f_elm_t expAggrRes[][N_SHARES * N_SHARES], int shareDistr[][N_SHARES],
                         int n, int n_bits, int n_shares);

void aggregateVectorN(f_elm_t expAggrRes[][N_SHARES * N_SHARES], f_elm_t res[], int n_bits, int n_shares);

void mergeHashesN(f_elm_t hashes[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                  f_elm_t mergedHashes[][N_SHARES * N_SHARES], int shareDistr[][N_SHARES],
                  int n, int n_bits, int n_shares);

void compareHashesN(f_elm_t expAggrRes[][N_SHARES * N_SHARES], f_elm_t mergedHashes[][N_SHARES * N_SHARES],
                    int checkHash[][N_SHARES * N_SHARES], int n_bits, int n_shares);
#endif