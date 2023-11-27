#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "arith.h"
#include "rss.h"
#include "aux.h"
#include "fips202.h"

#define PORT 8000
#define SERVER_IP "127.0.0.1"

void *server_thread(void *arg);

int main()
{
    // printf("C(%d, %d) = %llu\n", 2, 1, binomialCoefficient(2, 1));

    // printf("Generated tuples of size %d:\n", t);
    // generateTuples(set, tuple, tuples, N, t, 0, 0, &totalTuples);

    // Print stored tuples
    // for (int i = 0; i < totalTuples; i++) {
    //     printf("(");
    //     for (int j = 0; j < t; j++) {
    //         printf("%d", tuples[i][j]);
    //         if (j < t - 1) {
    //             printf(", ");
    //         }
    //     }
    //     printf(")\n");
    // }

    // Print stored tuples
    // printf("Share distribution matrix\n");
    // shareDistribution(shareDistr, tuples, N, N_SHARES);
    // for (int i = 0; i < N_SHARES; i++) {
    //     printf("(");
    //     for (int j = 0; j < N; j++) {
    //         printf("%d ", shareDistr[j][i]);
    //     }
    //     printf(")\n");
    // }

    // printf("Share count matrix\n");
    // shareCounting(shareCount, shareDistr, N, N_SHARES);
    // for (int i = 0; i < N_SHARES; i++) {
    //     printf("(");
    //     for (int j = 0; j < N_SHARES; j++) {
    //         printf("%d ", shareCount[i][j]);
    //     }
    //     printf(")\n");
    // }

    // printf("Share mapping matrix\n");
    // shareMapping(shareMap, shareDistr, N, N_SHARES);
    // for (int i = 0; i < N; i++) {
    //     printf("(");
    //     for (int j = 0; j < N_SHARES_P_SERVER; j++) {
    //         printf("%d ", shareMap[i][j]);
    //     }
    //     printf(")\n");
    // }

    // printf("Secret : ");
    // f_from_ui(secret, sec_val);
    // print_f_elm(secret);
    // printf("\n");

    // printf("Secret shares : \n");
    // secretSharing(secretShares, secret, N_SHARES);
    // for(int i = 0; i < N_SHARES; i++){
    //     printf("Share %02d ; ", i+1);
    //     print_f_elm(secretShares[i]);
    // }
    // printf("\n");

    // printf("Recovered secret : ");
    // secretOpening(secretShares, rec_secret, N_SHARES);
    // print_f_elm(rec_secret);
    // printf("\n");

    // printf("Server secret shares: \n");
    // secretSharingServers(secretSharesServ, secretShares, shareMap, N, N_SHARES_P_SERVER);
    // for(int i = 0; i < N; i++){
    //     for(int j = 0; j < N_SHARES_P_SERVER; j++){
    //         print_f_elm(secretSharesServ[i][j]);
    //     }
    //     printf("\n");
    // }

    // printf("Fraction matrix check \n");
    // generateFractions(fractionMatrix, shareCount, N_SHARES);
    // for (int i = 0; i < N_SHARES; i++)
    // {
    //     for (int j = 0; j < N_SHARES; j++)
    //     {
    //         f_from_ui(tmp_elm, shareCount[i][j]);
    //         to_mont(tmp_elm, shareCountMont[i][j]);
    //         f_mul(fractionMatrix[i][j], shareCountMont[i][j], checkMatrix[i][j]);
    //         from_mont(checkMatrix[i][j], res_tmp);
    //         print_f_elm(res_tmp);
    //     }
    //     printf("\n");
    // }

    // f_from_ui(key, key_val);
    // secretSharing(keyShares, key, N_SHARES);
    // secretSharingServers(keySharesServ, keyShares, shareMap, N, N_SHARES_P_SERVER);

    // f_from_ui(s2, s2_val);
    // secretSharing(s2Shares, s2, N_SHARES);
    // secretSharingServers(s2SharesServ, s2Shares, shareMap, N, N_SHARES_P_SERVER);

    // for (int i = 0; i < N; i++)
    // {
    //     for(int j = 0; j < N_SHARES_P_SERVER; j++)
    //     {
    //         f_add(secretSharesServ[i][j], keySharesServ[i][j], add_res[i][j]);
    //     }
    // }

    int set[] = {1, 2, 3, 4, 5, 6}; // Example set
    int t = 2;                      // Size of tuples

    int tuple[t];            // Array to store a tuple
    int tuples[N_SHARES][2]; // 2D array to store all tuples (adjust the size accordingly)
    int totalTuples = 0;     // Variable to keep track of the total number of tuples

    char fileString[30];

    int shareDistr[N][N_SHARES];
    int shareCount[N_SHARES][N_SHARES] = {0};
    int shareMap[N][N_SHARES_P_SERVER] = {0};

    uint64_t sec_val = 5;
    uint64_t key_val = 4;
    uint64_t s2_val = 9;
    uint64_t zero = 0;

    f_elm_t fractionMatrix[N_SHARES][N_SHARES];
    f_elm_t shareCountMont[N_SHARES][N_SHARES];
    f_elm_t checkMatrix[N_SHARES][N_SHARES];

    f_elm_t secret;
    f_elm_t secretShares[N_SHARES];
    f_elm_t secretSharesServ[N][N_SHARES_P_SERVER];

    f_elm_t key;
    f_elm_t keyShares[N_SHARES];
    f_elm_t keySharesServ[N][N_SHARES_P_SERVER];

    f_elm_t s2;
    f_elm_t s2Shares[N_SHARES];
    f_elm_t s2SharesServ[N][N_SHARES_P_SERVER];

    f_elm_t multMaskVal;
    f_elm_t multMask[N_SHARES * N_SHARES];
    f_elm_t multMaskServ[N][N_SHARES_P_SERVER * N_SHARES_P_SERVER];

    f_elm_t aMaskServ[N][N_SHARES_P_SERVER * N_SHARES_P_SERVER];
    f_elm_t bMaskServ[N][N_SHARES_P_SERVER * N_SHARES_P_SERVER];
    f_elm_t rMaskServ[N][N_SHARES_P_SERVER * N_SHARES_P_SERVER];

    f_elm_t hashes[N][N_SHARES_P_SERVER * N_SHARES_P_SERVER];
    f_elm_t mergedHashes[N_SHARES * N_SHARES];
    int checkHash[N_SHARES * N_SHARES];
    int hashesPass;

    f_elm_t resSharesServ[N][N_SHARES_P_SERVER * N_SHARES_P_SERVER];
    f_elm_t expAggrRes[N_SHARES * N_SHARES];
    f_elm_t res;

    f_elm_t received_shares[N];
    f_elm_t agg_res_1;
    f_elm_t agg_res_2;

    generateTuples(set, tuple, tuples, N, t, 0, 0, &totalTuples);
    shareDistribution(shareDistr, tuples, N, N_SHARES);
    shareCounting(shareCount, shareDistr, N, N_SHARES);
    shareMapping(shareMap, shareDistr, N, N_SHARES);
    generateFractions(fractionMatrix, shareCount, N_SHARES);

    strcpy(fileString, "bin/fractionMatrix.bin");
    write_elm_arr(fileString, fractionMatrix, N_SHARES * N_SHARES);

    strcpy(fileString, "bin/shareMap.bin");
    write_int_arr(fileString, shareMap, N * N_SHARES_P_SERVER);

    f_from_ui(secret, sec_val);
    secretSharing(secretShares, secret, N_SHARES);
    secretSharingServers(secretSharesServ, secretShares, shareMap, N, N_SHARES_P_SERVER);

    f_from_ui(key, key_val);
    secretSharing(keyShares, key, N_SHARES);
    secretSharingServers(keySharesServ, keyShares, shareMap, N, N_SHARES_P_SERVER);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/key_%d.bin", i + 1);
        write_elm_arr(fileString, keySharesServ[i], N_SHARES_P_SERVER);
    }

    f_from_ui(s2, s2_val);
    secretSharing(s2Shares, s2, N_SHARES);
    secretSharingServers(s2SharesServ, s2Shares, shareMap, N, N_SHARES_P_SERVER);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/s2_%d.bin", i + 1);
        write_elm_arr(fileString, s2SharesServ[i], N_SHARES_P_SERVER);
    }

    f_from_ui(multMaskVal, zero);
    secretSharing(multMask, multMaskVal, N_SHARES * N_SHARES);
    shareMultMask(multMaskServ, multMask, shareDistr, N, N_SHARES);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/mult_mask_%d.bin", i + 1);
        write_elm_arr(fileString, multMaskServ[i], N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    }

    shareElmMask(aMaskServ, shareDistr, shareCount, N, N_SHARES);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/a_mask_%d.bin", i + 1);
        write_elm_arr(fileString, aMaskServ[i], N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    }

    shareElmMask(bMaskServ, shareDistr, shareCount, N, N_SHARES);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/b_mask_%d.bin", i + 1);
        write_elm_arr(fileString, bMaskServ[i], N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    }

    shareElmMask(rMaskServ, shareDistr, shareCount, N, N_SHARES);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/r_mask_%d.bin", i + 1);
        write_elm_arr(fileString, rMaskServ[i], N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    }

    pthread_t threads[N];
    int server_ids[N];

    for (int i = 0; i < N; ++i)
    {
        server_ids[i] = i;
        if (pthread_create(&threads[i], NULL, server_thread, &server_ids[i]) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }

    sleep(5);

    struct sockaddr_in server_addr;
    int client_sock;

    for (int i = 0; i < N; ++i)
    {
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock == -1)
        {
            perror("Error creating socket");
            exit(EXIT_FAILURE);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT + i);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        {
            perror("Error connecting to server");
            close(client_sock);
            exit(EXIT_FAILURE);
        }

        send(client_sock, secretSharesServ[i], sizeof(f_elm_t) * N_SHARES_P_SERVER, 0);

        ssize_t bytes_received_1 = recv(client_sock, resSharesServ[i],
                                        sizeof(f_elm_t) * N_SHARES_P_SERVER * N_SHARES_P_SERVER, 0);

        ssize_t bytes_received_2 = recv(client_sock, hashes[i],
                                        sizeof(f_elm_t) * N_SHARES_P_SERVER * N_SHARES_P_SERVER, 0);

        close(client_sock);
    }

    for (int i = 0; i < 1; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    expandAggregateRes(resSharesServ, expAggrRes, shareDistr, N, N_SHARES);
    
    mergeHashes(hashes, mergedHashes, shareDistr, N, N_SHARES);
    compareHashes(expAggrRes, mergedHashes, checkHash, N_SHARES * N_SHARES);

    hashesPass = 1;
    for(int i = 0; i < (N_SHARES * N_SHARES); i++)
    {
        if(checkHash[i] == 0)
        {
            hashesPass = 0;
        }
    }
    (hashesPass == 1) ? printf("All hashes pass!\n") : printf("Error in hashes");
    

    aggregateVector(expAggrRes, res, N_SHARES * N_SHARES);
    print_f_elm(res);

    return 0;
}

void *server_thread(void *arg)
{
    int server_id = *(int *)arg;
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT + server_id);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error binding socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    listen(server_sock, 1);

    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
    if (client_sock == -1)
    {
        perror("Error accepting connection");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    int shareMap[N][N_SHARES_P_SERVER] = {0};

    char fileString[30];

    f_elm_t secretShares[N_SHARES_P_SERVER];

    ssize_t bytes_received = recv(client_sock, secretShares, sizeof(secretShares), 0);

    int ind_i;
    int ind_j;

    uint64_t zero = 0;

    f_elm_t fractionMatrix[N_SHARES][N_SHARES];
    f_elm_t keyShares[N_SHARES_P_SERVER];
    f_elm_t s2Shares[N_SHARES_P_SERVER];
    f_elm_t multMask[N_SHARES_P_SERVER * N_SHARES_P_SERVER];
    f_elm_t aMask[N_SHARES_P_SERVER * N_SHARES_P_SERVER];
    f_elm_t bMask[N_SHARES_P_SERVER * N_SHARES_P_SERVER];
    f_elm_t rMask[N_SHARES_P_SERVER * N_SHARES_P_SERVER];

    f_elm_t add_res[N_SHARES_P_SERVER];

    f_elm_t a, b;
    f_elm_t a_mont, b_mont;
    f_elm_t ab_mont;
    f_elm_t ab;
    f_elm_t ab_r, ab_r_mont;
    f_elm_t f_ab_r_mont, f_ab_r;
    f_elm_t a_mask, a_mask_mont;
    f_elm_t aa_mask_mont, aa_mask;
    f_elm_t f_ab_r_aa_mask;
    f_elm_t b_mask, b_mask_mont;
    f_elm_t bb_mask_mont, bb_mask;
    f_elm_t f_ab_r_aa_mask_bb_mask;
    f_elm_t f_ab_r_aa_mask_bb_mask_r_mask;

    f_elm_t hashes[N_SHARES_P_SERVER * N_SHARES_P_SERVER];

    f_elm_t resShares[N_SHARES_P_SERVER * N_SHARES_P_SERVER];

    strcpy(fileString, "bin/fractionMatrix.bin");
    read_elm_arr(fileString, fractionMatrix, N_SHARES * N_SHARES);

    strcpy(fileString, "bin/shareMap.bin");
    read_int_arr(fileString, shareMap, N * N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/key_%d.bin", server_id + 1);
    read_elm_arr(fileString, keyShares, N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/s2_%d.bin", server_id + 1);
    read_elm_arr(fileString, s2Shares, N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/mult_mask_%d.bin", server_id + 1);
    read_elm_arr(fileString, multMask, N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/a_mask_%d.bin", server_id + 1);
    read_elm_arr(fileString, aMask, N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/b_mask_%d.bin", server_id + 1);
    read_elm_arr(fileString, bMask, N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/r_mask_%d.bin", server_id + 1);
    read_elm_arr(fileString, rMask, N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    for (int i = 0; i < N_SHARES_P_SERVER; i++)
    {
        f_add(secretShares[i], keyShares[i], add_res[i]);
    }

    for (int i = 0; i < N_SHARES_P_SERVER; i++)
    {
        for (int j = 0; j < N_SHARES_P_SERVER; j++)
        {
            ind_i = shareMap[server_id][i] - 1;
            ind_j = shareMap[server_id][j] - 1;

            f_copy(add_res[i], a);
            f_copy(s2Shares[j], b);

            to_mont(a, a_mont);
            to_mont(b, b_mont);
            f_mul(a_mont, b_mont, ab_mont);

            from_mont(ab_mont, ab);
            f_add(ab, multMask[i * N_SHARES_P_SERVER + j], ab_r);

            shake128((unsigned char *) hashes[i * N_SHARES_P_SERVER + j], WORDS_FIELD * 8,
                     (unsigned char *) ab_r, WORDS_FIELD * 8);

            to_mont(ab_r, ab_r_mont);
            f_mul(fractionMatrix[ind_i][ind_j], ab_r_mont, f_ab_r_mont);
            from_mont(f_ab_r_mont, f_ab_r);

            to_mont(aMask[i * N_SHARES_P_SERVER + j], a_mask_mont);
            f_mul(a_mont, a_mask_mont, aa_mask_mont);
            from_mont(aa_mask_mont, aa_mask);

            f_add(f_ab_r, aa_mask, f_ab_r_aa_mask);

            to_mont(bMask[i * N_SHARES_P_SERVER + j], b_mask_mont);
            f_mul(b_mont, b_mask_mont, bb_mask_mont);
            from_mont(bb_mask_mont, bb_mask);

            f_add(f_ab_r_aa_mask, bb_mask, f_ab_r_aa_mask_bb_mask);

            f_add(f_ab_r_aa_mask_bb_mask, rMask[i * N_SHARES_P_SERVER + j], f_ab_r_aa_mask_bb_mask_r_mask);

            f_copy(f_ab_r, resShares[i * N_SHARES_P_SERVER + j]);
        }
    }

    send(client_sock, resShares, sizeof(f_elm_t) * N_SHARES_P_SERVER * N_SHARES_P_SERVER, 0);
    send(client_sock, hashes, sizeof(f_elm_t) * N_SHARES_P_SERVER * N_SHARES_P_SERVER, 0);

    close(client_sock);
    close(server_sock);

    pthread_exit(NULL);
}
