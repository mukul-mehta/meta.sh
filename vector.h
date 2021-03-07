#ifndef VECTOR_H_
#define VECTOR_H_

#include <assert.h> /* for assert */
#include <stdlib.h> /* for malloc/realloc/free */

/**
 * @brief vector_vector_type - The vector type used in this library
 */
#define vector_vector_type(type) type*

/**
 * @brief vector_set_capacity - For internal use, sets the capacity variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define vector_set_capacity(vec, size)                                                             \
    do {                                                                                           \
        if (vec) {                                                                                 \
            ((size_t*)(vec))[-1] = (size);                                                         \
        }                                                                                          \
    } while (0)

/**
 * @brief vector_set_size - For internal use, sets the size variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define vector_set_size(vec, size)                                                                 \
    do {                                                                                           \
        if (vec) {                                                                                 \
            ((size_t*)(vec))[-2] = (size);                                                         \
        }                                                                                          \
    } while (0)

/**
 * @brief vector_capacity - gets the current capacity of the vector
 * @param vec - the vector
 * @return the capacity as a size_t
 */
#define vector_capacity(vec) ((vec) ? ((size_t*)(vec))[-1] : (size_t)0)

/**
 * @brief vector_size - gets the current size of the vector
 * @param vec - the vector
 * @return the size as a size_t
 */
#define vector_size(vec) ((vec) ? ((size_t*)(vec))[-2] : (size_t)0)

/**
 * @brief vector_empty - returns non-zero if the vector is empty
 * @param vec - the vector
 * @return non-zero if empty, zero if non-empty
 */
#define vector_empty(vec) (vector_size(vec) == 0)

/**
 * @brief vector_grow - For internal use, ensures that the vector is at least <count> elements big
 * @param vec - the vector
 * @param count - the new capacity to set
 * @return void
 */
#define vector_grow(vec, count)                                                                    \
    do {                                                                                           \
        const size_t cv_sz = (count) * sizeof(*(vec)) + (sizeof(size_t) * 2);                      \
        if (!(vec)) {                                                                              \
            size_t* cv_p = malloc(cv_sz);                                                          \
            assert(cv_p);                                                                          \
            (vec) = (void*)(&cv_p[2]);                                                             \
            vector_set_capacity((vec), (count));                                                   \
            vector_set_size((vec), 0);                                                             \
        } else {                                                                                   \
            size_t* cv_p1 = &((size_t*)(vec))[-2];                                                 \
            size_t* cv_p2 = realloc(cv_p1, (cv_sz));                                               \
            assert(cv_p2);                                                                         \
            (vec) = (void*)(&cv_p2[2]);                                                            \
            vector_set_capacity((vec), (count));                                                   \
        }                                                                                          \
    } while (0)

/**
 * @brief vector_pop_back - removes the last element from the vector
 * @param vec - the vector
 * @return void
 */
#define vector_pop_back(vec)                                                                       \
    do {                                                                                           \
        vector_set_size((vec), vector_size(vec) - 1);                                              \
    } while (0)

/**
 * @brief vector_erase - removes the element at index i from the vector
 * @param vec - the vector
 * @param i - index of element to remove
 * @return void
 */
#define vector_erase(vec, i)                                                                       \
    do {                                                                                           \
        if (vec) {                                                                                 \
            const size_t cv_sz = vector_size(vec);                                                 \
            if ((i) < cv_sz) {                                                                     \
                vector_set_size((vec), cv_sz - 1);                                                 \
                size_t cv_x;                                                                       \
                for (cv_x = (i); cv_x < (cv_sz - 1); ++cv_x) {                                     \
                    (vec)[cv_x] = (vec)[cv_x + 1];                                                 \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/**
 * @brief vector_free - frees all memory associated with the vector
 * @param vec - the vector
 * @return void
 */
#define vector_free(vec)                                                                           \
    do {                                                                                           \
        if (vec) {                                                                                 \
            size_t* p1 = &((size_t*)(vec))[-2];                                                    \
            free(p1);                                                                              \
        }                                                                                          \
    } while (0)

/**
 * @brief vector_begin - returns an iterator to first element of the vector
 * @param vec - the vector
 * @return a pointer to the first element (or NULL)
 */
#define vector_begin(vec) (vec)

/**
 * @brief vector_end - returns an iterator to one past the last element of the vector
 * @param vec - the vector
 * @return a pointer to one past the last element (or NULL)
 */
#define vector_end(vec) ((vec) ? &((vec)[vector_size(vec)]) : NULL)

/**
 * @brief vector_push_back - adds an element to the end of the vector
 * @param vec - the vector
 * @param value - the value to add
 * @return void
 */
#define vector_push_back(vec, value)                                                               \
    do {                                                                                           \
        size_t cv_cap = vector_capacity(vec);                                                      \
        if (cv_cap <= vector_size(vec)) {                                                          \
            vector_grow((vec), !cv_cap ? cv_cap + 1 : cv_cap * 2);                                 \
        }                                                                                          \
        vec[vector_size(vec)] = (value);                                                           \
        vector_set_size((vec), vector_size(vec) + 1);                                              \
    } while (0)

/**
 * @brief vector_copy - copy a vector
 * @param from - the original vector
 * @param to - destination to which the function copy to
 * @return void
 */
#define vector_copy(from, to)                                                                      \
    do {                                                                                           \
        for (size_t i = 0; i < vector_size(from); i++) {                                           \
            vector_push_back(to, from[i]);                                                         \
        }                                                                                          \
    } while (0)

#endif /* VECTOR_H_ */
