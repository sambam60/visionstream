/* Stub jerasure.h to avoid dependency */
#ifndef JERASURE_STUB_H
#define JERASURE_STUB_H

// Stub: FEC is not supported in this build
static inline int jerasure_matrix_decode(int k, int m, int w, int *matrix, int row_k_ones, int *erasures, char **data_ptrs, char **coding_ptrs, int size) {
    (void)k; (void)m; (void)w; (void)matrix; (void)row_k_ones; (void)erasures; (void)data_ptrs; (void)coding_ptrs; (void)size;
    return -1;
}

static inline int jerasure_matrix_dotprod(int k, int w, int *matrix_row, int *ids, char **data_ptrs, char *output, int size) {
    (void)k; (void)w; (void)matrix_row; (void)ids; (void)data_ptrs; (void)output; (void)size;
    return -1;
}

#endif // JERASURE_STUB_H

