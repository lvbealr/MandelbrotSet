#ifndef INTRINSICS_H_
#define INTRINSICS_H_

#include <stddef.h>

const size_t ARRAY_SIZE = 8;

#define FOR_LOOP(ARRAY, CODE) do {                      \
    for (size_t idx = 0; idx < ARRAY_SIZE; idx++) {     \
        CODE                                            \
    }                                                   \
} while (0)

enum cmpOperand {
    CMP_LE_OQ_ = 18
};

void mm256_set1_ps(float m256[ARRAY_SIZE], const float value);
void mm256_set_ps (float m256[ARRAY_SIZE], const float v0, const float v1, const float v2, const float v3,
                                           const float v4, const float v5, const float v6, const float v7);
                                           
void mm256_add_ps (float m256[ARRAY_SIZE], const float m256_1[ARRAY_SIZE], const float m256_2[ARRAY_SIZE]);
void mm256_mul_ps (float m256[ARRAY_SIZE], const float m256_1[ARRAY_SIZE], const float m256_2[ARRAY_SIZE]);

void mm256_setzero_si256(int m256[ARRAY_SIZE]);

void mm256_cmp_ps(float m256[ARRAY_SIZE], const float m256_l[ARRAY_SIZE],
                                          const float m256_r[ARRAY_SIZE], cmpOperand operand);

void mm256_set1_epi32(int m256i[ARRAY_SIZE], int value);
void mm256_add_epi32 (int m256i[ARRAY_SIZE], const int m256_1[ARRAY_SIZE],
                                             const int m256_2[ARRAY_SIZE]);

void mm256_castps_si256(int m256i[ARRAY_SIZE], const float m256[ARRAY_SIZE]);

void mm256_and_si256   (int m256i_1[ARRAY_SIZE], const int m256i_2[ARRAY_SIZE]);

void mm256_sub_ps(float m256[ARRAY_SIZE], const float m256_1[ARRAY_SIZE],
                                          const float m256_2[ARRAY_SIZE]);

void mm256_storeu_si256(int m256i_1[ARRAY_SIZE], const int m256_2[ARRAY_SIZE]);

int mm256_movemask_ps(const float cmp[ARRAY_SIZE]);
