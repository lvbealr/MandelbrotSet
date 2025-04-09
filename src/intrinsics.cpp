#include <stddef.h>
#include <inttypes.h>

#include "intrinsics.h"

void mm256_set1_ps(float m256[ARRAY_SIZE], const float value) {
    FOR_LOOP(m256, m256[idx] = value;);
}

void mm256_set_ps(float m256[ARRAY_SIZE], const float v0, const float v1, const float v2, const float v3,
                                          const float v4, const float v5, const float v6, const float v7) {
    m256[0] = v0; m256[1] = v1; m256[2] = v2; m256[3] = v3;
    m256[4] = v4; m256[5] = v5; m256[6] = v6; m256[7] = v7;
}

void mm256_add_ps(float m256[ARRAY_SIZE], const float m256_1[ARRAY_SIZE], const float m256_2[ARRAY_SIZE]) {
    FOR_LOOP(m256, m256[idx] = m256_1[idx] + m256_2[idx];);
}

void mm256_mul_ps(float m256[ARRAY_SIZE], const float m256_1[ARRAY_SIZE], const float m256_2[ARRAY_SIZE]) {
    FOR_LOOP(m256, m256[idx] = m256_1[idx] * m256_2[idx];);
}

void mm256_setzero_si256(int m256[ARRAY_SIZE]) {
    FOR_LOOP(m256, m256[idx] = 0;);
}

void mm256_cmp_ps(float m256[ARRAY_SIZE], const float m256_l[ARRAY_SIZE], const float m256_r[ARRAY_SIZE], cmpOperand operand) {
    FOR_LOOP(m256,
        {
            if (m256_l[idx] <= m256_r[idx]) {
                m256[idx] = -1.0f;
            } else {
                m256[idx] = 0.0f;
            }
        }
    );
}

void mm256_set1_epi32(int m256i[ARRAY_SIZE], int value) {
    FOR_LOOP(m256i, m256i[idx] = value;);
}

void mm256_add_epi32(int m256i[ARRAY_SIZE], const int m256_1[ARRAY_SIZE], const int m256_2[ARRAY_SIZE]) {
    FOR_LOOP(m256i, m256i[idx] = m256_1[idx] + m256_2[idx];);
}

void mm256_castps_si256(int m256i[ARRAY_SIZE], const float m256[ARRAY_SIZE]) {
    FOR_LOOP(m256i, m256i[idx] = (int)m256[idx];);
}

void mm256_and_si256(int m256i_1[ARRAY_SIZE], const int m256i_2[ARRAY_SIZE]) {
    FOR_LOOP(m256i_1, m256i_1[idx] &= m256i_2[idx];);
}

void mm256_sub_ps(float m256[ARRAY_SIZE], const float m256_1[ARRAY_SIZE], const float m256_2[ARRAY_SIZE]) {
    FOR_LOOP(m256, m256[idx] = m256_1[idx] - m256_2[idx];);
}

void mm256_storeu_si256(int m256i_1[ARRAY_SIZE], const int m256_2[ARRAY_SIZE]) {
    FOR_LOOP(m256i_1, m256i_1[idx] = m256_2[idx];);
}

int mm256_movemask_ps(const float cmp[ARRAY_SIZE]) {
    int mask = 0;

    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        uint32_t bits = *(uint32_t*)&cmp[i];
        
        if (bits & 0x80000000) {
            mask |= (1 << i);
        }
    }

    return mask;
}