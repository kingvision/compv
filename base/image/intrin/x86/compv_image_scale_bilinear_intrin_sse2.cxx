/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/intrin/x86/compv_image_scale_bilinear_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// This function requires outWidth < 0xffff (65k)
void CompVImageScaleBilinear_Intrin_SSE2(
	const uint8_t* inPtr, compv_uscalar_t inWidth, compv_uscalar_t inHeight, COMPV_ALIGNED(SSE) compv_uscalar_t inStride,
	COMPV_ALIGNED(SSE) uint8_t* outPtr, compv_uscalar_t outWidth, compv_uscalar_t outHeight, COMPV_ALIGNED(SSE) compv_uscalar_t outStride,
	compv_uscalar_t sf_x, compv_uscalar_t sf_y)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();

#if 1
	compv_uscalar_t i, j, k, x, y, nearestX, nearestY;
	int sf_x_ = static_cast<int>(sf_x);
	const uint8_t* inPtr_;
	COMPV_ALIGN_SSE() int32_t neighb0[16];
	COMPV_ALIGN_SSE() int32_t neighb1[16];
	COMPV_ALIGN_SSE() int32_t neighb2[16];
	COMPV_ALIGN_SSE() int32_t neighb3[16];
	COMPV_ALIGN_SSE() const int32_t SFX[4][4] = {
		{ sf_x_ * 0, sf_x_ * 1, sf_x_ * 2, sf_x_ * 3 },
		{ sf_x_ * 4, sf_x_ * 5, sf_x_ * 6, sf_x_ * 7 },
		{ sf_x_ * 8, sf_x_ * 9, sf_x_ * 10, sf_x_ * 11 },
		{ sf_x_ * 12, sf_x_ * 13, sf_x_ * 14, sf_x_ * 15 }
	};
	__m128i vecX0, vecX1, vecX2, vecX3, vecY0, vec0, vec1, vec2, vec3;
	__m128i vecret0, vecret1, vecret2, vecret3;
	__m128i vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3;
	const __m128i vec0xff = _mm_set1_epi32(0xff);
	const __m128i vecSfxTimes16 = _mm_set1_epi32(sf_x_ * 16);
	const __m128i vecSFX0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[0]));
	const __m128i vecSFX1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[1]));
	const __m128i vecSFX2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[2]));
	const __m128i vecSFX3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[3]));
	const __m128i vecSFY = _mm_set1_epi32(static_cast<int>(sf_y));

	vecY0 = _mm_setzero_si128();
	for (j = 0, y = 0; j < outHeight; ++j, y += sf_y) {
		nearestY = (y >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));
		vecX0 = vecSFX0, vecX1 = vecSFX1, vecX2 = vecSFX2, vecX3 = vecSFX3;
		for (i = 0, x = 0; i < outWidth; i += 16) {
			for (k = 0; k < 16; ++k, x += sf_x) {
				nearestX = (x >> 8); // nearest x-point (compute for each row but this is faster than storing the values then reading from mem)
				neighb0[k] = static_cast<int32_t>(*(inPtr_ + nearestX));
				neighb1[k] = static_cast<int32_t>(*(inPtr_ + nearestX + 1));
				neighb2[k] = static_cast<int32_t>(*(inPtr_ + nearestX + inStride));
				neighb3[k] = static_cast<int32_t>(*(inPtr_ + nearestX + inStride + 1));
			}
			/* Part #0 */
			vecNeighb0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb0[0]));
			vecNeighb1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb1[0]));
			vecNeighb2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb2[0]));	
			vecNeighb3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb3[0]));
			vec0 = _mm_and_si128(vecX0, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret0 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/* Part #1 */
			vecNeighb0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb0[4]));
			vecNeighb1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb1[4]));
			vecNeighb2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb2[4]));
			vecNeighb3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb3[4]));
			vec0 = _mm_and_si128(vecX1, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret1 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/* Part #2 */
			vecNeighb0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb0[8]));
			vecNeighb1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb1[8]));
			vecNeighb2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb2[8]));
			vecNeighb3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb3[8]));
			vec0 = _mm_and_si128(vecX2, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret2 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/* Part #3 */
			vecNeighb0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb0[12]));
			vecNeighb1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb1[12]));
			vecNeighb2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb2[12]));
			vecNeighb3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&neighb3[12]));
			vec0 = _mm_and_si128(vecX3, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret3 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Packs result and write to outPtr ****/
			vecret0 = _mm_packus_epi32(vecret0, vecret1);
			vecret1 = _mm_packus_epi32(vecret2, vecret3);
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i]), _mm_packus_epi16(vecret0, vecret1));

			/**** move to next indices ****/
			vecX0 = _mm_add_epi32(vecX0, vecSfxTimes16);
			vecX1 = _mm_add_epi32(vecX1, vecSfxTimes16);
			vecX2 = _mm_add_epi32(vecX2, vecSfxTimes16);
			vecX3 = _mm_add_epi32(vecX3, vecSfxTimes16);			
		}
		vecY0 = _mm_add_epi32(vecY0, vecSFY);
		outPtr += outStride;
	}
#endif

#if 0
	compv_uscalar_t i, j, y;
	const uint8_t* inPtr_;
	int sf_x_ = static_cast<int>(sf_x);
	COMPV_ALIGN_SSE() const int32_t SFX[4][4] = {
		{ sf_x_ * 0, sf_x_ * 1, sf_x_ * 2, sf_x_ * 3 },
		{ sf_x_ * 4, sf_x_ * 5, sf_x_ * 6, sf_x_ * 7 },
		{ sf_x_ * 8, sf_x_ * 9, sf_x_ * 10, sf_x_ * 11 },
		{ sf_x_ * 12, sf_x_ * 13, sf_x_ * 14, sf_x_ * 15 }
	};
	__m128i vecX0, vecX1, vecX2, vecX3, vecY0, vecIndicesLow, vecIndicesHigh, vecNeighb0, vecNeighb1, vecNeighb2, vecNeighb3, vec0, vec1, vec2, vec3;
	__m128i vecret0, vecret1, vecret2, vecret3;
	const __m128i vec0xff = _mm_set1_epi32(0xff);
	const __m128i vec0x1 = _mm_set1_epi32(0x1);
	const __m128i vecInStride = _mm_set1_epi32(static_cast<int>(inStride));
	const __m128i vecInStridePlusOne = _mm_add_epi32(vecInStride, vec0x1);
	const __m128i vecSfxTimes16 = _mm_set1_epi32(sf_x_ * 16);
	const __m128i vecSFX0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[0]));
	const __m128i vecSFX1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[1]));
	const __m128i vecSFX2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[2]));
	const __m128i vecSFX3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&SFX[3]));
	const __m128i vecSFY = _mm_set1_epi32(static_cast<int>(sf_y));

	// TODO(dmi): next code is used to avoid "uninitialized local variable 'vecNeighbx' used" error message
	// must not use in ASM
	vecNeighb0 = vecNeighb1 = vecNeighb2 = vecNeighb3 = _mm_setzero_si128();

	// TODO(dmi): SS41 have _mm_insert_epi8 
	// FIXME: you can use 'vecSFX0' only and add 'vecSFX'

	vecY0 = _mm_setzero_si128();
	for (j = 0, y = 0; j < outHeight; ++j, y += sf_y) { // FIXME: use for (y ... (sf_y*outHeight) and remove j
		inPtr_ = (inPtr + ((y >> 8) * inStride)); // FIXME: use SIMD (vecY0) and remove y += sf_y
		vecX0 = vecSFX0, vecX1 = vecSFX1, vecX2 = vecSFX2, vecX3 = vecSFX3;
		for (i = 0; i < outWidth; i += 16) {

			// FIXME: when we have #4 low or #4 high we dont need the rest, just add 1 -> do not convert to epi16

			/**** Part - #0 ****/
			// compute indices
			vec0 = _mm_srli_epi32(vecX0, 8);
			_mm_store_si128(&vecIndicesLow, _mm_packs_epi32(vec0, _mm_add_epi32(vec0, vec0x1)));
			_mm_store_si128(&vecIndicesHigh, _mm_packs_epi32(_mm_add_epi32(vec0, vecInStride), _mm_add_epi32(vec0, vecInStridePlusOne)));
			// load neighbs
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 0)], 0);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 1)], 1);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 2)], 2);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 3)], 3);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 4)], 0);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 5)], 1);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 6)], 2);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 7)], 3);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 0)], 0);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 1)], 1);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 2)], 2);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 3)], 3);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 4)], 0);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 5)], 1);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 6)], 2);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 7)], 3);
			// convert neighbs from epi16 to epi32
			vecNeighb0 = _mm_unpacklo_epi16(vecNeighb0, _mm_setzero_si128());
			vecNeighb1 = _mm_unpacklo_epi16(vecNeighb1, _mm_setzero_si128());
			vecNeighb2 = _mm_unpacklo_epi16(vecNeighb2, _mm_setzero_si128());
			vecNeighb3 = _mm_unpacklo_epi16(vecNeighb3, _mm_setzero_si128());
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX0, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret0 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Part - #1 ****/
			// compute indices
			vec0 = _mm_srli_epi32(vecX1, 8);
			_mm_store_si128(&vecIndicesLow, _mm_packs_epi32(vec0, _mm_add_epi32(vec0, vec0x1)));
			_mm_store_si128(&vecIndicesHigh, _mm_packs_epi32(_mm_add_epi32(vec0, vecInStride), _mm_add_epi32(vec0, vecInStridePlusOne)));
			// load neighbs
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 0)], 0);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 1)], 1);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 2)], 2);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 3)], 3);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 4)], 0);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 5)], 1);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 6)], 2);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 7)], 3);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 0)], 0);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 1)], 1);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 2)], 2);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 3)], 3);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 4)], 0);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 5)], 1);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 6)], 2);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 7)], 3);
			// convert neighbs from epi16 to epi32
			vecNeighb0 = _mm_unpacklo_epi16(vecNeighb0, _mm_setzero_si128());
			vecNeighb1 = _mm_unpacklo_epi16(vecNeighb1, _mm_setzero_si128());
			vecNeighb2 = _mm_unpacklo_epi16(vecNeighb2, _mm_setzero_si128());
			vecNeighb3 = _mm_unpacklo_epi16(vecNeighb3, _mm_setzero_si128());
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX1, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret1 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Part - #2 ****/
			// compute indices
			vec0 = _mm_srli_epi32(vecX2, 8);
			_mm_store_si128(&vecIndicesLow, _mm_packs_epi32(vec0, _mm_add_epi32(vec0, vec0x1)));
			_mm_store_si128(&vecIndicesHigh, _mm_packs_epi32(_mm_add_epi32(vec0, vecInStride), _mm_add_epi32(vec0, vecInStridePlusOne)));
			// load neighbs
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 0)], 0);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 1)], 1);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 2)], 2);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 3)], 3);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 4)], 0);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 5)], 1);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 6)], 2);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 7)], 3);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 0)], 0);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 1)], 1);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 2)], 2);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 3)], 3);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 4)], 0);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 5)], 1);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 6)], 2);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 7)], 3);
			// convert neighbs from epi16 to epi32
			vecNeighb0 = _mm_unpacklo_epi16(vecNeighb0, _mm_setzero_si128());
			vecNeighb1 = _mm_unpacklo_epi16(vecNeighb1, _mm_setzero_si128());
			vecNeighb2 = _mm_unpacklo_epi16(vecNeighb2, _mm_setzero_si128());
			vecNeighb3 = _mm_unpacklo_epi16(vecNeighb3, _mm_setzero_si128());
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX2, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret2 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** Part - #3 ****/
			// compute indices
			vec0 = _mm_srli_epi32(vecX3, 8);
			_mm_store_si128(&vecIndicesLow, _mm_packs_epi32(vec0, _mm_add_epi32(vec0, vec0x1)));
			_mm_store_si128(&vecIndicesHigh, _mm_packs_epi32(_mm_add_epi32(vec0, vecInStride), _mm_add_epi32(vec0, vecInStridePlusOne)));
			// load neighbs
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 0)], 0);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 1)], 1);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 2)], 2);
			vecNeighb0 = _mm_insert_epi16(vecNeighb0, inPtr_[_mm_extract_epi16(vecIndicesLow, 3)], 3);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 4)], 0);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 5)], 1);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 6)], 2);
			vecNeighb1 = _mm_insert_epi16(vecNeighb1, inPtr_[_mm_extract_epi16(vecIndicesLow, 7)], 3);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 0)], 0);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 1)], 1);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 2)], 2);
			vecNeighb2 = _mm_insert_epi16(vecNeighb2, inPtr_[_mm_extract_epi16(vecIndicesHigh, 3)], 3);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 4)], 0);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 5)], 1);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 6)], 2);
			vecNeighb3 = _mm_insert_epi16(vecNeighb3, inPtr_[_mm_extract_epi16(vecIndicesHigh, 7)], 3);
			// convert neighbs from epi16 to epi32
			vecNeighb0 = _mm_unpacklo_epi16(vecNeighb0, _mm_setzero_si128());
			vecNeighb1 = _mm_unpacklo_epi16(vecNeighb1, _mm_setzero_si128());
			vecNeighb2 = _mm_unpacklo_epi16(vecNeighb2, _mm_setzero_si128());
			vecNeighb3 = _mm_unpacklo_epi16(vecNeighb3, _mm_setzero_si128());
			// compute x0, y0, x1, y1
			vec0 = _mm_and_si128(vecX3, vec0xff);
			vec1 = _mm_and_si128(vecY0, vec0xff);
			vec2 = _mm_sub_epi32(vec0xff, vec0);
			vec3 = _mm_sub_epi32(vec0xff, vec1);
			// compute ret0
			vecNeighb0 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb0, vec2), _mm_mullo_epi32(vecNeighb1, vec0)), vec3);
			vecNeighb1 = _mm_mullo_epi32(_mm_add_epi32(_mm_mullo_epi32(vecNeighb2, vec2), _mm_mullo_epi32(vecNeighb3, vec0)), vec1);
			vecret3 = _mm_srli_epi32(_mm_add_epi32(vecNeighb0, vecNeighb1), 16);

			/**** move to next indices ****/
			vecX0 = _mm_add_epi32(vecX0, vecSfxTimes16);
			vecX1 = _mm_add_epi32(vecX1, vecSfxTimes16);
			vecX2 = _mm_add_epi32(vecX2, vecSfxTimes16);
			vecX3 = _mm_add_epi32(vecX3, vecSfxTimes16);

			/**** Packs result and write to outPtr ****/
			vecret0 = _mm_packus_epi32(vecret0, vecret1);
			vecret1 = _mm_packus_epi32(vecret2, vecret3);
			_mm_store_si128(reinterpret_cast<__m128i*>(&outPtr[i]), _mm_packus_epi16(vecret0, vecret1));
		}
		outPtr += outStride;
		vecY0 = _mm_add_epi32(vecY0, vecSFY);
	}
#endif

#if 0
	compv_uscalar_t i, j, x, y, nearestX, nearestY;
	unsigned int neighb0, neighb1, neighb2, neighb3, x0, y0, x1, y1;
	const uint8_t* inPtr_;

	for (j = 0, y = 0; j < outHeight; ++j, y +=sf_y) {
		nearestY = (y >> 8); // nearest y-point
		inPtr_ = (inPtr + (nearestY * inStride));
		for (i = 0, x = 0; i < outWidth; ++i, x += sf_x) {
			nearestX = (x >> 8); // nearest x-point

			neighb0 = inPtr_[nearestX];
			neighb1 = inPtr_[nearestX + 1];
			neighb2 = inPtr_[nearestX + inStride];
			neighb3 = inPtr_[nearestX + inStride + 1];

			x0 = x & 0xff;
			y0 = y & 0xff;
			x1 = 0xff - x0;
			y1 = 0xff - y0;

			outPtr[i] = static_cast<uint8_t>((y1 * ((neighb0 * x1) + (neighb1 * x0)) + y0 * ((neighb2 * x1) + (neighb3 * x0))) >> 16); // no need for saturation after >> 16
		}
	}
#endif
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */