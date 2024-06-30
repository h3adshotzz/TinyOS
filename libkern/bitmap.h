#ifndef __LIBKERN_BITMAP_H__
#define __LIBKERN_BITMAP_H__

// this is just a test to see whether xnu's bit.h could be useful in tinyos
// in any way.

#include <tinylibc/stdint.h>
#include <libkern/compiler.h>

// this should be _Atomic, but the arm gcc toolchain doesn't support the right symbols
typedef uint64_t		bitmap_t;
typedef unsigned int			uint_t;

#define BIT(b)						(1ULL << (b))

#define mask(width)					(BIT(width) - 1)
#define extract(x, shift, width)	((((uint64_t)(x)) >> (shift)) & mask(width))
#define bits(x, hi, lo)				extract((x), (lo), (hi) - (lo) + 1)

#define bit_set(x, b)				((x) |= BIT(b))
#define bit_clear(x, b)				((x) &= ~BIT(b))
#define bit_test(x, b)				((bool)((x) & BIT(b)))

/* Returns the most significant '1' bit, or -1 if all zeros */
inline static int
bit_first(uint64_t bitmap)
{
	return (bitmap == 0) ? -1 : 63 - __builtin_clzll(bitmap);
}

inline static int
__bit_next(uint64_t bitmap, int previous_bit)
{
	uint64_t mask = previous_bit ? mask(previous_bit) : ~0ULL;
	return bit_first(bitmap & mask);
}

/**
 * Returns msot significant '1' bit that is less significant than previous_bit,
 * or -1 if no such bit exists
*/
inline static int
bit_next(uint64_t bitmap, int previous_bit)
{
	if (previous_bit == 0) {
		return -1;
	} else {
		return __bit_next(bitmap, previous_bit);
	}
}

/* Returns the least significant '1' bit, or -1 if all zeros */
inline static int
lsb_first(uint64_t bitmap)
{
	return __builtin_ffsll(bitmap) - 1;
}

/**
 * Returns the least significant '1' bit that is more significant than
 * previous_bit, or -1 if no such bit exists. previous_bit may be -1, in which
 * case this is equivalent to lsb_first()
*/
inline static int
lsb_next(uint64_t bitmap, int previous_bit)
{
	uint64_t mask = mask(previous_bit + 1);
	return lsb_first(bitmap & ~mask);
}

inline static int
bit_count(uint64_t x)
{
	return __builtin_popcountll(x);
}

/* Return the highest power of 2 that is <= n, or -1 if n == 0*/
inline static int
bit_floor(uint64_t n)
{
	return bit_first(n);
}

/* Return the lowest power of 2 that is >= n, or -1 if n == 0 */
inline static int
bit_ceiling(uint64_t n)
{
	if (n == 0)
		return -1;
	return bit_first(n - 1) + 1;
}

/* If n is a power of 2, bit_log2(n) == bit_floor(n) == bit_ceiling(n) */
#define bit_log2(n)				bit_floor((uint64_t)(n))

//inline static bool
//atomic_bit_set(bitmap_t *map, int n, int mem_order)
//{
//	bitmap_t prev;
//	prev = __c11_atomic_fetch_or(map, BIT(n), mem_order);
//	return bit_test(prev, n);
//}
//
//inline static bool
//atomic_bit_clear(bitmap_t *map, int n, int mem_order)
//{
//	bitmap_t prev;
//	prev = __c11_atomic_fetch_and(map, ~BIT(n), mem_order);
//	return bit_test(prev, n);
//}

#define BITMAP_LEN(n)				(((uint_t)(n) + 63) >> 6)
#define BITMAP_SIZE(n)				(size_t)(BITMAP_LEN(n) << 3)
#define bitmap_bit(n)				bits(n ,5, 0)
#define bitmap_index(n)				bits(n, 63, 6)

inline static bitmap_t *
bitmap_zero(bitmap_t *map, uint_t nbits)
{
	return (bitmap_t *)memset((void *)map, 0, BITMAP_SIZE(nbits));
}

inline static bitmap_t *
bitmap_full(bitmap_t *map, uint_t nbits)
{
	return (bitmap_t *)memset((void *)map, ~0, BITMAP_SIZE(nbits));
}

inline static bitmap_t *
bitmap_alloc(uint_t nbits)
{
	assert(nbits > 0);
	bitmap_t *map = (bitmap_t *)kalloc(BITMAP_SIZE(nbits));
	if (map) {
		bitmap_zero(map, nbits);
	}
	return map;
}

inline static void
bitmap_free(bitmap_t *map, uint_t nbits)
{
	assert(nbits > 0);
	kfree(map, BITMAP_SIZE(nbits));
}

inline static void
bitmap_set(bitmap_t *map, uint_t n)
{
	bit_set(map[bitmap_index(n)], bitmap_bit(n));
}

inline static void
bitmap_clear(bitmap_t *map, uint_t n)
{
	bit_clear(map[bitmap_index(n)], bitmap_bit(n));
}

inline static bool
atomic_bitmap_set(bitmap_t *map, uint_t n, int mem_order)
{
	return atomic_bit_set(&map[bitmap_index(n)], bitmap_bit(n), mem_order);
}

inline static bool
atomic_bitmap_clear(bitmap_t *map, uint_t n, int mem_order)
{
	return atomic_bit_clear(&map[bitmap_index(n)], bitmap_bit(n), mem_order);
}

inline static bool
bitmap_test(bitmap_t *map, uint_t n)
{
	return bit_test(map[bitmap_index(n)], bitmap_bit(n));
}

inline static int
bitmap_first(bitmap_t *map, uint_t nbits)
{
	for (int i = (int)bitmap_index(nbits - 1); i >= 0; i--) {
		if (map[i] == 0) {
			continue;
		}
		return (i << 6) + bit_first(map[i]);
	}

	return -1;
}

inline static int
bitmap_and_not_mask_first(bitmap_t *map, bitmap_t *mask, uint_t nbits)
{
	for (int i = (int)bitmap_index(nbits - 1); i >= 0; i--) {
		if ((map[i] & ~mask[i]) == 0) {
			continue;
		}
		return (i << 6) + bit_first(map[i] & ~mask[i]);
	}

	return -1;
}

inline static int
bitmap_lsb_first(bitmap_t *map, uint_t nbits)
{
	for (uint_t i = 0; i <= bitmap_index(nbits - 1); i++) {
		if (map[i] == 0) {
			continue;
		}
		return (int)((i << 6) + (uint32_t)lsb_first(map[i]));
	}

	return -1;
}

inline static int
bitmap_next(bitmap_t *map, uint_t prev)
{
	if (prev == 0) {
		return -1;
	}

	int64_t i = bitmap_index(prev - 1);
	int res = __bit_next(map[i], bits(prev, 5, 0));
	if (res >= 0) {
		return (int)(res + (i << 6));
	}

	for (i = i - 1; i >= 0; i--) {
		if (map[i] == 0) {
			continue;
		}
		return (int)((i << 6) + bit_first(map[i]));
	}

	return -1;
}

inline static int
bitmap_lsb_next(bitmap_t *map, uint_t nbits, uint_t prev)
{
	if ((prev + 1) >= nbits) {
		return -1;
	}

	uint64_t i = bitmap_index(prev + 1);
	uint_t b = bits((prev + 1), 5, 0) - 1;
	int32_t res = lsb_next((uint64_t)map[i], (int)b);
	if (res >= 0) {
		return (int)((uint64_t)res + (i << 6));
	}

	for (i = i + 1; i <= bitmap_index(nbits - 1); i++) {
		if (map[i] == 0) {
			continue;
		}
		return (int)((i << 6) + (uint64_t)lsb_first(map[i]));
	}

	return -1;
}

#endif /* __libkern_bitmap_h__ */