#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#include "arith.h"
#include "rss.h"
#include "aux.h"
#include "fips202.h"

#define PORT 8000
#define SERVER_IP "127.0.0.1"

struct ReceiverData
{
    int id;
    f_elm_t *secretShares;
    f_elm_t (*receivedShares)[N_SHARES_P_SERVER * N_SHARES_P_SERVER];
    f_elm_t (*hashes)[N_SHARES_P_SERVER * N_SHARES_P_SERVER];
};

void *receiver_function(void *arg);
void *server_thread(void *arg);

int main()
{
    int set[N]; // Example set

    for (int i = 0; i < N; i++)
    {
        set[i] = i + 1;
    }

    int tuple[T];            // Array to store a tuple
    int tuples[N_SHARES][T]; // 2D array to store all tuples (adjust the size accordingly)
    int totalTuples = 0;     // Variable to keep track of the total number of tuples

    char fileString[30];

    int shareDistr[N][N_SHARES];
    int shareCount[N_SHARES][N_SHARES] = {0};
    int shareMap[N][N_SHARES_P_SERVER] = {0};

    uint64_t sec_val = 0;
    uint64_t key_val = 1;
    uint64_t s2_val = 2;
    uint64_t zero = 0;

    f_elm_t(*fractionMatrix)[N_SHARES] =
        malloc(sizeof(f_elm_t) * N_SHARES * N_SHARES);
    f_elm_t(*shareCountMont)[N_SHARES] =
        malloc(sizeof(f_elm_t) * N_SHARES * N_SHARES);
    // f_elm_t (*checkMatrix)[N_SHARES][N_SHARES];

    f_elm_t secret;
    f_elm_t (*secretShares) = malloc(sizeof(f_elm_t) * N_SHARES);
    f_elm_t (*secretSharesServ)[N_SHARES_P_SERVER] = malloc(sizeof(f_elm_t) * N * N_SHARES_P_SERVER);

    f_elm_t (*keys) = malloc(sizeof(f_elm_t) * N_BITS);
    f_elm_t (*keySharesServ)[N_BITS][N_SHARES_P_SERVER] = 
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER);

    f_elm_t s, s_mont;
    f_elm_t s2_mont, s2;
    f_elm_t (*s2Shares) = malloc(sizeof(f_elm_t) * N_SHARES);
    f_elm_t (*s2SharesServ)[N_SHARES_P_SERVER] = malloc(sizeof(f_elm_t) * N * N_SHARES_P_SERVER);

    f_elm_t(*multMaskServ)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    f_elm_t(*aMaskServ)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t(*bMaskServ)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t(*rMaskServ)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

    f_elm_t(*hashes)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t(*mergedHashes)[N_SHARES * N_SHARES] =
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES * N_SHARES);
    int (*checkHash)[N_SHARES * N_SHARES] = 
        malloc(sizeof(int) * N_BITS * N_SHARES * N_SHARES);
    int hashesPass;

    f_elm_t(*resSharesServ)[N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t(*expAggrRes)[N_SHARES * N_SHARES] =
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES * N_SHARES);
    f_elm_t (*res) = malloc(sizeof(f_elm_t) * N_BITS);
    unsigned char resOPRF[N_BITS];

    generateTuples(set, tuple, tuples, N, T, 0, 0, &totalTuples);
    shareDistribution(shareDistr, tuples);
    shareCounting(shareCount, shareDistr);
    shareMapping(shareMap, shareDistr);
    generateFractions(fractionMatrix, shareCount);

    strcpy(fileString, "bin/fractionMatrix.bin");
    write_elm_arr(fileString, fractionMatrix, N_SHARES * N_SHARES);

    strcpy(fileString, "bin/shareMap.bin");
    write_int_arr(fileString, shareMap, N * N_SHARES_P_SERVER);

    f_rand(secret);
    secretSharing(secretShares, secret, N_SHARES);
    secretSharingServers(secretSharesServ, secretShares, shareMap);

    for (int i = 0; i < N_BITS; i++)
    {
        f_rand(keys[i]);
    }
    keySharingServersN(keySharesServ, keys, shareMap);

    for (int i = 0; i < N; i++)
    {
        snprintf(fileString, 30, "bin/key_%d.bin", i + 1);
        write_elm_arr(fileString, keySharesServ[i], N_BITS * N_SHARES_P_SERVER);
    }

    f_rand(s);
    to_mont(s, s_mont);
    f_mul(s_mont, s_mont, s2_mont);
    from_mont(s2_mont, s2);
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

    pthread_t server_threads[N];
    int server_ids[N];

    for (int i = 0; i < N; ++i)
    {
        server_ids[i] = i;
        if (pthread_create(&server_threads[i], NULL, server_thread, &server_ids[i]) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }

    usleep(1000000);

    pthread_t receiver_threads[N];
    struct ReceiverData receiver_data[N];

    for (int i = 0; i < N; ++i)
    {
        receiver_data[i].id = i;

        receiver_data[i].secretShares = malloc(sizeof(f_elm_t) * N_SHARES_P_SERVER);
        for (int j = 0; j < N_SHARES_P_SERVER; j++)
        {
            f_copy(secretSharesServ[i][j], receiver_data[i].secretShares[j]);
        }

        receiver_data[i].receivedShares = malloc(sizeof(f_elm_t) * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
        receiver_data[i].hashes = malloc(sizeof(f_elm_t) * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);

        if (pthread_create(&receiver_threads[i], NULL, receiver_function, (void *)&receiver_data[i]) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < N; ++i)
    {
        pthread_join(server_threads[i], NULL);
    }

    for (int i = 0; i < N; ++i)
    {
        pthread_join(receiver_threads[i], NULL);
    }

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N_BITS; j++)
        {
            for (int k = 0; k < (N_SHARES_P_SERVER * N_SHARES_P_SERVER); k++)
            {
                f_copy(receiver_data[i].receivedShares[j][k], resSharesServ[i][j][k]);
                f_copy(receiver_data[i].hashes[j][k], hashes[i][j][k]);
            }
        }
    }

    aggregateResN(resSharesServ, expAggrRes, shareDistr);

    mergeHashesN(hashes, mergedHashes, shareDistr);
    compareHashesN(expAggrRes, mergedHashes, checkHash);
    checkHashesN(checkHash);

    aggregateVectorN(expAggrRes, res);
    // printResN(res);

    calculateOPRF(res, resOPRF);
    for (int i = 0; i < N_BITS; i++)
    {
        printf("%d", resOPRF[i]);
    }
    printf("\n");

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
    f_elm_t(*aMask)[N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t(*bMask)[N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
        malloc(sizeof(f_elm_t) * N_BITS * N_SHARES_P_SERVER * N_SHARES_P_SERVER);
    f_elm_t(*rMask)[N_SHARES_P_SERVER * N_SHARES_P_SERVER] =
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
        send(client_sock, resShares[r * N_TRANSMIT], sizeof(f_elm_t) * N_TRANSMIT * N_SHARES_P_SERVER * N_SHARES_P_SERVER, MSG_WAITALL);
    }
    for (int r = 0; r < (N_BITS / N_TRANSMIT); r++)
    {
        send(client_sock, hashes[r * N_TRANSMIT], sizeof(f_elm_t) * N_TRANSMIT * N_SHARES_P_SERVER * N_SHARES_P_SERVER, MSG_WAITALL);
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

void *receiver_function(void *arg)
{
    struct ReceiverData *data = (struct ReceiverData *)arg;
    int id = data->id;
    f_elm_t *secretShares = data->secretShares;
    f_elm_t(*resSharesServ)[N_SHARES_P_SERVER * N_SHARES_P_SERVER] = data->receivedShares;
    f_elm_t(*hashes)[N_SHARES_P_SERVER * N_SHARES_P_SERVER] = data->hashes;

    struct sockaddr_in server_addr;
    int client_sock;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT + id);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error connecting to server");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    send(client_sock, secretShares, sizeof(f_elm_t) * N_SHARES_P_SERVER, 0);

    for (int r = 0; r < (N_BITS / N_TRANSMIT); r++)
    {
        ssize_t bytes_received_1 = recv(client_sock, resSharesServ[r * N_TRANSMIT],
                                        sizeof(f_elm_t) * N_TRANSMIT * N_SHARES_P_SERVER * N_SHARES_P_SERVER, MSG_WAITALL);
    }
    for (int r = 0; r < (N_BITS / N_TRANSMIT); r++)
    {
        ssize_t bytes_received_2 = recv(client_sock, hashes[r * N_TRANSMIT],
                                        sizeof(f_elm_t) * N_TRANSMIT * N_SHARES_P_SERVER * N_SHARES_P_SERVER, MSG_WAITALL);
    }

    close(client_sock);
    return NULL;
}