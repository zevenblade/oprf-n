#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>

#include "arith.h"
#include "rss.h"
#include "aux.h"
#include "fips202.h"

#define PORT 8000
#define SERVER_IP "127.0.0.1"

void *server_thread(void *arg);

int main()
{
    int set[] = {1, 2, 3, 4, 5, 6}; // Example set
    int t = 2;                      // Size of tuples

    int tuple[t];            // Array to store a tuple
    int tuples[N_SHARES][2]; // 2D array to store all tuples (adjust the size accordingly)
    int totalTuples = 0;     // Variable to keep track of the total number of tuples

    char fileString[30];

    int shareDistr[N][N_SHARES];
    int shareCount[N_SHARES][N_SHARES] = {0};
    int shareMap[N][N_SHARES_P_SERVER] = {0};

    uint64_t sec_val = 0;
    uint64_t key_val = 1;
    uint64_t s2_val = 2;
    uint64_t zero = 0;

    f_elm_t fractionMatrix[N_SHARES][N_SHARES];
    f_elm_t shareCountMont[N_SHARES][N_SHARES];
    f_elm_t checkMatrix[N_SHARES][N_SHARES];

    f_elm_t secret;
    f_elm_t secretShares[N_SHARES];
    f_elm_t secretSharesServ[N][N_SHARES_P_SERVER];

    f_elm_t keys[N_BITS];
    f_elm_t keySharesServ[N][N_BITS][N_SHARES_P_SERVER];

    f_elm_t s2;
    f_elm_t s2Shares[N_SHARES];
    f_elm_t s2SharesServ[N][N_SHARES_P_SERVER];

    f_elm_t(*multMaskServ)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    f_elm_t (*aMaskServ)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER]=
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t (*bMaskServ)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER]=
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t (*rMaskServ)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER]=
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    f_elm_t(*hashes)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t(*mergedHashes)[N_SHARES * N_SHARES] =
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES * N_SHARES);
    int checkHash[N_BITS][N_SHARES * N_SHARES];
    int hashesPass;

    f_elm_t(*resSharesServ)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t(*expAggrRes)[N_SHARES * N_SHARES] =
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES * N_SHARES);
    f_elm_t res[N_BITS];

    generateTuples(set, tuple, tuples, N, t, 0, 0, &totalTuples);
    shareDistribution(shareDistr, tuples);
    shareCounting(shareCount, shareDistr);
    shareMapping(shareMap, shareDistr);
    generateFractions(fractionMatrix, shareCount);

    strcpy(fileString, "bin/fractionMatrix.bin");
    write_elm_arr(fileString, fractionMatrix, N_SHARES * N_SHARES);

    strcpy(fileString, "bin/shareMap.bin");
    write_int_arr(fileString, shareMap, N * N_SHARES_P_SERVER);

    f_from_ui(secret, sec_val);
    secretSharing(secretShares, secret, N_SHARES);
    secretSharingServers(secretSharesServ, secretShares, shareMap);

    for (int i = 0; i < N_BITS; i++)
    {
        f_from_ui(keys[i], key_val);
        key_val += 1;
    }
    keySharingServersN(keySharesServ, keys, shareMap);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/key_%d.bin", i + 1);
        write_elm_arr(fileString, keySharesServ[i], N_BITS * N_SHARES_P_SERVER);
    }

    f_from_ui(s2, s2_val);
    secretSharing(s2Shares, s2, N_SHARES);
    secretSharingServers(s2SharesServ, s2Shares, shareMap);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/s2_%d.bin", i + 1);
        write_elm_arr(fileString, s2SharesServ[i], N_SHARES_P_SERVER);
    }

    shareMultMaskN(multMaskServ, shareDistr);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/mult_mask_%d.bin", i + 1);
        write_elm_arr(fileString, multMaskServ[i], N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    }

    shareElmMaskN(aMaskServ, shareDistr, shareCount);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/a_mask_%d.bin", i + 1);
        write_elm_arr(fileString, aMaskServ[i], N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    }

    shareElmMaskN(bMaskServ, shareDistr, shareCount);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/b_mask_%d.bin", i + 1);
        write_elm_arr(fileString, bMaskServ[i], N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    }

    shareElmMaskN(rMaskServ, shareDistr, shareCount);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/r_mask_%d.bin", i + 1);
        write_elm_arr(fileString, rMaskServ[i], N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
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

    sleep(3);

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

        for (int r = 0; r < (N_BITS / N_TRANSMIT); r++)
        {
            ssize_t bytes_received_1 = recv(client_sock, resSharesServ[i][r * N_TRANSMIT],
                                            sizeof(f_elm_t) * N_TRANSMIT * N_SHARES_P_SERVER * N_SHARES_P_SERVER, 0);
            
            ssize_t bytes_received_2 = recv(client_sock, hashes[i][r * N_TRANSMIT],
                                            sizeof(f_elm_t) * N_TRANSMIT * N_SHARES_P_SERVER * N_SHARES_P_SERVER, 0);
        }

        close(client_sock);
    }

    for (int i = 0; i < 1; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    aggregateResN(resSharesServ, expAggrRes, shareDistr);

    mergeHashesN(hashes, mergedHashes, shareDistr);
    compareHashesN(expAggrRes, mergedHashes, checkHash);
    checkHashesN(checkHash);
    
    aggregateVectorN(expAggrRes, res);
    printResN(res);

    free(multMaskServ);
    free(aMaskServ);
    free(bMaskServ);
    free(rMaskServ);

    free(hashes);
    free(mergedHashes);
    free(resSharesServ);
    free(expAggrRes);

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

    int flags = fcntl(server_sock, F_GETFL, 0);
    fcntl(server_sock, F_SETFL, flags & ~O_NONBLOCK);


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
    f_elm_t keyShares[N_BITS][N_SHARES_P_SERVER];
    f_elm_t s2Shares[N_SHARES_P_SERVER];

    f_elm_t(*multMask)[N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t (*aMask)[N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t (*bMask)[N_SHARES_P_SERVER * N_SHARES_P_SERVER]=
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t (*rMask)[N_SHARES_P_SERVER * N_SHARES_P_SERVER]=
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

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

    f_elm_t(*hashes)[N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t(*resShares)[N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    strcpy(fileString, "bin/fractionMatrix.bin");
    read_elm_arr(fileString, fractionMatrix, N_SHARES * N_SHARES);

    strcpy(fileString, "bin/shareMap.bin");
    read_int_arr(fileString, shareMap, N * N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/key_%d.bin", server_id + 1);
    read_elm_arr(fileString, keyShares, N_BITS * N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/s2_%d.bin", server_id + 1);
    read_elm_arr(fileString, s2Shares, N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/mult_mask_%d.bin", server_id + 1);
    read_elm_arr(fileString, multMask, N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/a_mask_%d.bin", server_id + 1);
    read_elm_arr(fileString, aMask, N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/b_mask_%d.bin", server_id + 1);
    read_elm_arr(fileString, bMask, N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    snprintf(fileString, 30, "bin/r_mask_%d.bin", server_id + 1);
    read_elm_arr(fileString, rMask, N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    for (int r = 0; r < N_BITS; r++)
    {
        for (int i = 0; i < N_SHARES_P_SERVER; i++)
        {
            f_add(secretShares[i], keyShares[r][i], add_res[i]);
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
                f_add(ab, multMask[r][i * N_SHARES_P_SERVER + j], ab_r);

                shake128((unsigned char *)hashes[r][i * N_SHARES_P_SERVER + j], WORDS_FIELD * 8,
                         (unsigned char *)ab_r, WORDS_FIELD * 8);

                to_mont(ab_r, ab_r_mont);
                f_mul(fractionMatrix[ind_i][ind_j], ab_r_mont, f_ab_r_mont);
                from_mont(f_ab_r_mont, f_ab_r);

                to_mont(aMask[r][i * N_SHARES_P_SERVER + j], a_mask_mont);
                f_mul(a_mont, a_mask_mont, aa_mask_mont);
                from_mont(aa_mask_mont, aa_mask);

                f_add(f_ab_r, aa_mask, f_ab_r_aa_mask);

                to_mont(bMask[r][i * N_SHARES_P_SERVER + j], b_mask_mont);
                f_mul(b_mont, b_mask_mont, bb_mask_mont);
                from_mont(bb_mask_mont, bb_mask);

                f_add(f_ab_r_aa_mask, bb_mask, f_ab_r_aa_mask_bb_mask);

                f_add(f_ab_r_aa_mask_bb_mask, rMask[r][i * N_SHARES_P_SERVER + j], f_ab_r_aa_mask_bb_mask_r_mask);

                f_copy(f_ab_r_aa_mask_bb_mask_r_mask, resShares[r][i * N_SHARES_P_SERVER + j]);
            }
        }
    }

    for (int r = 0; r < (N_BITS / N_TRANSMIT); r++)
    {
        send(client_sock, resShares[r * N_TRANSMIT], sizeof(f_elm_t) * N_TRANSMIT * N_SHARES_P_SERVER * N_SHARES_P_SERVER, 0);
        usleep(500);
        send(client_sock, hashes[r * N_TRANSMIT], sizeof(f_elm_t) * N_TRANSMIT * N_SHARES_P_SERVER * N_SHARES_P_SERVER, 0);
        usleep(500);
    }

    close(client_sock);
    close(server_sock);

    free(multMask);
    free(aMask);
    free(bMask);
    free(rMask);

    free(hashes);
    free(resShares);

    pthread_exit(NULL);
}
