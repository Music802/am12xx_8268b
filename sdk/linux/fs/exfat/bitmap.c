/*
 * Copyright (C) 2008, OGAWA Hirofumi
 * Released under GPL v2.
 */

#include <linux/buffer_head.h>
#include "exfat.h"
#include "bitmap.h"
#include "debug.h"

//#define BIT_WORD1(nr)		((nr) / BITS_PER_LONG)

/* Carry-Save Adder */
#define CSA(h, l, a, b, c)	do {		\
	unsigned long u = a ^ b;		\
	unsigned long v = c;			\
	h = (a & b) | (u & v);			\
	l = u ^ v;				\
} while (0)

/* TODO: maybe, should merge to bitmap_weight(). */
static int hweight_array(const unsigned long *bitmap, int n)
{
	unsigned long ones, twos, twosA, twosB, fours, foursA, foursB, eights;
	int counts, i;

	counts = 0;
	ones = twos = fours = 0;
	for (i = 0; i <= n - 8; i += 8) {
		CSA(twosA, ones, ones, bitmap[i], bitmap[i + 1]);
		CSA(twosB, ones, ones, bitmap[i + 2], bitmap[i + 3]);
		CSA(foursA, twos, twos, twosA, twosB);
		CSA(twosA, ones, ones, bitmap[i + 4], bitmap[i + 5]);
		CSA(twosB, ones, ones, bitmap[i + 6], bitmap[i + 7]);
		CSA(foursB, twos, twos, twosA, twosB);
		CSA(eights, fours, fours, foursA, foursB);
		counts += hweight_long(eights);
	}
	counts = 8 * counts + 4 * hweight_long(fours)
		+ 2 * hweight_long(twos) + hweight_long(ones);
	for (; i < n; i++)
		counts += hweight_long(bitmap[i]);
	return counts;
}

#if BITS_PER_LONG == 64
typedef __le64			__lelong;
#define lelong_to_cpu(x)	le64_to_cpu(x);
#else
typedef __le32			__lelong;
#define lelong_to_cpu(x)	le32_to_cpu(x);
#endif

#ifdef EXFAT_MODIFY
unsigned long find_bit_and_set(uint8_t* bitmap, unsigned long start,unsigned long end)
{
	const unsigned long mid_start = (start + 7) / 8 * 8;
	const unsigned long mid_end = end / 8 * 8;
	unsigned long c;
	int byte;

	for (c = start; c < mid_start; c++)
		if (BMAP_GET(bitmap, c) == 0)
		{
			BMAP_SET(bitmap, c);
			return c + EXFAT_START_ENT;
		}

	for (byte = mid_start / 8; byte < mid_end / 8; byte++)
		if (bitmap[byte] != 0xff)
		{
			int bit;

			for (bit = 0; bit < 8; bit++)
				if (!(bitmap[byte] & (1u << bit)))
				{
					bitmap[byte] |= (1u << bit);
					return byte * 8 + bit + EXFAT_START_ENT;
				}
		}

	for (c = mid_end; c < end; c++)
		if (BMAP_GET(bitmap, c) == 0)
		{
			BMAP_SET(bitmap, c);
			return c + EXFAT_START_ENT;
		}

	return EXFAT_ENT_EOF;
}
#endif

/* Hack: want bitmap_weight_le() */
static int  exfat_hweight_le(const void *buf, int bits)
{
	const unsigned long *bitmap = buf;
	int counts;

	/* counts = bitmap_weight(bitmap, bits & ~(BITS_PER_LONG - 1)); */
	counts = hweight_array(bitmap, BIT_WORD(bits & ~(BITS_PER_LONG - 1)));
	if (bits & (BITS_PER_LONG - 1)) {
		unsigned long word;
		word = lelong_to_cpu(*(__lelong *)&bitmap[BIT_WORD(bits) + 1]);
		counts += hweight_long(word & BITMAP_LAST_WORD_MASK(bits));
	}
	return counts;
}

/*�����Ŀ��д���������sbi->free_clusters*/
static int exfat_count_free_clusters(struct inode *inode)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct address_space *mapping = inode->i_mapping;
	pgoff_t index;
	u32 bits, bits_left, used_bits;

	/* FIXME: readahead all data of bitmap */

	index = 0;
	used_bits = 0;
	bits_left = sbi->total_clusters;
	while (bits_left > 0) {
		struct page *page;
		void *kaddr;

		page = read_mapping_page(mapping, index, NULL);
		if (IS_ERR(page))
			return PTR_ERR(page);

		bits = min_t(u32, bits_left, PAGE_CACHE_SIZE * 8);
		kaddr = kmap_atomic(page, KM_USER0);

		used_bits += exfat_hweight_le(kaddr, bits);

		kunmap_atomic(kaddr, KM_USER0);
		page_cache_release(page);

		bits_left -= bits;
		index++;
	}

	sbi->free_clusters = sbi->total_clusters - used_bits;
	exfat_debug("free_clusters %u", sbi->free_clusters);

	return 0;
}

/*����bitmap�ڵ�*/
int exfat_setup_bitmap(struct super_block *sb, u32 clusnr, u64 i_size)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct inode *inode;
	int err;

	if (!clusnr) {
		exfat_error("free space bitmap isn't available");
		return -EINVAL;
	}
	if (i_size < DIV_ROUND_UP(sbi->total_clusters, 8)) {
		exfat_error("free space bitmap is too small");
		return -EINVAL;
	}

	inode = exfat_new_internal_inode(sb, EXFAT_BITMAP_INO, 0, clusnr,
					 i_size);
	if (!inode)
		return -ENOMEM;

	err = exfat_count_free_clusters(inode);
	if (err)
		return err;

	sbi->bitmap_inode = inode;

	return 0;
}

void exfat_free_bitmap(struct exfat_sb_info *sbi)
{
	if (sbi->bitmap_inode) {
		iput(sbi->bitmap_inode);
		sbi->bitmap_inode = NULL;
	}
}
