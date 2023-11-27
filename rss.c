#include <string.h>

#include "rss.h"
#include "arith.h"
#include "fips202.h"


unsigned long long factorial(int num)
{
    if (num == 0 || num == 1)
    {
        return 1;
    }
    else
    {
        return num * factorial(num - 1);
    }
}

unsigned long long binomialCoefficient(int n, int k)
{
    if (k < 0 || k > n)
    {
        return 0; // Invalid input
    }

    return factorial(n) / (factorial(k) * factorial(n - k));
}

void generateTuples(int set[], int tuple[], int tuples[][2], int n, int t, int index, int tupleIndex, int *totalTuples)
{
    if (tupleIndex == t)
    {
        for (int i = 0; i < t; i++)
        {
            tuples[*totalTuples][i] = tuple[i];
        }
        (*totalTuples)++;
        return;
    }

    for (int i = index; i < n; i++)
    {
        tuple[tupleIndex] = set[i];
        generateTuples(set, tuple, tuples, n, t, i + 1, tupleIndex + 1, totalTuples);
    }
}

void shareDistribution(int shareDistribution[][N_SHARES], int tuples[][2], int n, int n_shares)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n_shares; j++)
        {
            if (((i + 1) != tuples[j][0]) && ((i + 1) != tuples[j][1]))
            {
                shareDistribution[i][j] = 1;
            }
            else
            {
                shareDistribution[i][j] = 0;
            }
        }
    }
}

void shareCounting(int shareCount[][N_SHARES], int shareDistribution[][N_SHARES], int n, int n_shares)
{

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n_shares; j++)
        {
            if (shareDistribution[i][j] == 1)
            {
                shareCount[j][j] += 1;

                for (int k = j + 1; k < n_shares; k++)
                {
                    if (shareDistribution[i][k] == 1)
                    {
                        shareCount[j][k] += 1;
                    }
                }
            }
        }
    }

    for (int i = 1; i < n_shares; i++)
    {
        for (int j = 0; j < i; j++)
        {
            shareCount[i][j] = shareCount[j][i];
        }
    }
}

void shareMapping(int shareMap[][N_SHARES_P_SERVER], int shareDistribution[][N_SHARES], int n, int n_shares)
{
    int ind;

    for (int i = 0; i < n; i++)
    {
        ind = 0;
        for (int j = 0; j < n_shares; j++)
        {
            if (shareDistribution[i][j] == 1)
            {
                shareMap[i][ind] = j + 1;
                ind++;
            }
        }
    }
}

void secretSharing(f_elm_t shares[], f_elm_t secret, int n_shares)
{
    f_elm_t partial_res[n_shares];

    for (int i = 0; i < (n_shares - 1); i++)
    {
        f_rand(shares[i]);
    }

    f_copy(secret, partial_res[0]);

    for (int i = 0; i < (n_shares - 1); i++)
    {
        f_sub(partial_res[i], shares[i], partial_res[i + 1]);
    }

    f_copy(partial_res[n_shares - 1], shares[n_shares - 1]);
}

void secretOpening(f_elm_t share[], f_elm_t secret, int n_shares)
{
    f_elm_t partial_res[n_shares - 1];

    f_add(share[0], share[1], partial_res[0]);

    for (int i = 2; i < n_shares; i++)
    {
        f_add(share[i], partial_res[i - 2], partial_res[i - 1]);
    }

    f_copy(partial_res[n_shares - 2], secret);
}

void secretSharingServers(f_elm_t sharesServer[][N_SHARES_P_SERVER], f_elm_t shares[],
                          int shareMap[][N_SHARES_P_SERVER], int n, int n_shares_p_server)
{
    int ind;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n_shares_p_server; j++)
        {
            ind = shareMap[i][j];
            f_copy(shares[ind - 1], sharesServer[i][j]);
        }
    }
}

void generateFractions(f_elm_t fractionMatrix[][N_SHARES], int shareCount[][N_SHARES], int n_shares)
{
    uint64_t one = 1;
    f_elm_t tmp_mtrx[N_SHARES][N_SHARES];

    for (int i = 0; i < n_shares; i++)
    {
        for (int j = 0; j < n_shares; j++)
        {
            f_from_ui(tmp_mtrx[i][j], shareCount[i][j]);
            to_mont(tmp_mtrx[i][j], fractionMatrix[i][j]);
            f_inv(fractionMatrix[i][j]);
        }
    }
}

void shareMultMask(f_elm_t multMaskServ[][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                   f_elm_t multMask[], int shareDistr[][N_SHARES], int n, int n_shares)
{
    int servInd[n];
    memset(servInd, 0, sizeof(servInd));

    for (int i = 0; i < n_shares; i++)
    {
        for (int j = 0; j < n_shares; j++)
        {
            for (int k = 0; k < n; k++)
            {
                if ((shareDistr[k][i] == 1) && (shareDistr[k][j] == 1))
                {
                    f_copy(multMask[i * n_shares + j], multMaskServ[k][servInd[k]]);
                    servInd[k] += 1;
                }
            }
        }
    }
}

void shareElmMask(f_elm_t elmMaskServ[][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                  int shareDistr[][N_SHARES], int shareCount[][N_SHARES], int n, int n_shares)
{
    int servInd[n];
    memset(servInd, 0, sizeof(servInd));

    uint64_t zero_ui = 0;
    f_elm_t zero;
    f_from_ui(zero, zero_ui);

    int n_row_shares;
    int indRowShare;

    for (int i = 0; i < n_shares; i++)
    {
        for (int j = 0; j < n_shares; j++)
        {
            n_row_shares = shareCount[i][j];
            f_elm_t rowShares[n_row_shares];
            secretSharing(rowShares, zero, n_row_shares);

            indRowShare = 0;
            for (int k = 0; k < n; k++)
            {
                if ((shareDistr[k][i] == 1) && (shareDistr[k][j] == 1))
                {
                    f_copy(rowShares[indRowShare], elmMaskServ[k][servInd[k]]);
                    servInd[k] += 1;
                    indRowShare += 1;
                }
            }
        }
    }
}

void expandAggregateRes(f_elm_t resServ[][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                        f_elm_t expAggrRes[], int shareDistr[][N_SHARES], int n, int n_shares)
{
    int servInd[n];
    memset(servInd, 0, sizeof(servInd));

    uint64_t zero_ui = 0;
    f_elm_t zero;
    f_from_ui(zero, zero_ui);

    f_elm_t expRes[n][n_shares * n_shares];
    f_elm_t sum_1;
    f_elm_t sum_2;

    for (int i = 0; i < n_shares; i++)
    {
        for (int j = 0; j < n_shares; j++)
        {
            for (int k = 0; k < n; k++)
            {
                if ((shareDistr[k][i] == 1) && (shareDistr[k][j] == 1))
                {
                    f_copy(resServ[k][servInd[k]], expRes[k][i * n_shares + j]);
                    servInd[k] += 1;
                }
                else
                {
                    f_copy(zero, expRes[k][i * n_shares + j]);
                }
            }
        }
    }

    for (int i = 0; i < (n_shares * n_shares); i++)
    {
        f_copy(zero, sum_1);
        for (int j = 0; j < n; j++)
        {
            f_add(expRes[j][i], sum_1, sum_2);
            f_copy(sum_2, sum_1);
        }

        f_copy(sum_2, expAggrRes[i]);
    }
}

void aggregateVector(f_elm_t expAggrRes[], f_elm_t res, int n_shares)
{
    f_elm_t sum_1, sum_2;

    uint64_t zero_ui = 0;
    f_elm_t zero;
    f_from_ui(zero, zero_ui);

    f_copy(zero, sum_1);
    for (int i = 0; i < n_shares; i++)
    {
        f_add(expAggrRes[i], sum_1, sum_2);
        f_copy(sum_2, sum_1);
    }
    f_copy(sum_2, res);
}

void mergeHashes(f_elm_t hashes[][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                 f_elm_t mergedHashes[], int shareDistr[][N_SHARES], int n, int n_shares)
{
    int servInd[n];
    memset(servInd, 0, sizeof(servInd));

    for (int i = 0; i < n_shares; i++)
    {
        for (int j = 0; j < n_shares; j++)
        {
            for (int k = 0; k < n; k++)
            {
                if ((shareDistr[k][i] == 1) && (shareDistr[k][j] == 1))
                {
                    f_copy(hashes[k][servInd[k]], mergedHashes[i * N_SHARES + j]);
                    servInd[k] += 1;
                }
            }
        }
    }
}

void compareHashes(f_elm_t expAggrRes[], f_elm_t mergedHashes[], int checkHash[], int n_shares)
{
    f_elm_t curr_hash;
    char *ptr1, *ptr2;
    int equal;

    for (int i = 0; i < n_shares; i++)
    {
        shake128((unsigned char *) curr_hash, WORDS_FIELD * 8,
                 (unsigned char *) expAggrRes[i], WORDS_FIELD * 8);

        equal = 1;
        ptr1 = (unsigned char *)curr_hash;
        ptr2 = (unsigned char *)mergedHashes[i];
        for (int j = 0; j < 8; ++j)
        {
            if (ptr1[j] != ptr2[j])
            {
                equal = 0; // Set to 0 if elements are not equal
                break;     // No need to continue checking
            }
        }
        checkHash[i] = equal;
    }
}