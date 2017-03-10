/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/edges/intrin/x86/compv_core_feature_canny_dete_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/core/features/edges/compv_core_feature_canny_dete.h" /* kCannyTangentPiOver8Int and kCannyTangentPiTimes3Over8Int */
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// "g" and "tLow" are unsigned but we're using "epi16" instead of "epu16" because "g" is always < 0xFFFF (from u8 convolution operation)
// 8mpw -> minpack 8 for words (int16)
void CompVCannyNMSGatherRow_8mpw_Intrin_SSSE3(uint8_t* nms, const uint16_t* g, const int16_t* gx, const int16_t* gy, const uint16_t* tLow1, compv_uscalar_t width, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CHECK_SSSE3();

	__m128i vecNMS, vecG, vecGX, vecAbsGX0, vecAbsGX1, vecGY, vecAbsGY0, vecAbsGY1, vec0, vec1, vec2, vec3, vec4, vec5, vec6;
	const __m128i vecTLow = _mm_set1_epi16(*tLow1);
	static const __m128i vecZero = _mm_setzero_si128();
	static const __m128i vecTangentPiOver8Int = _mm_set1_epi32(kCannyTangentPiOver8Int);
	static const __m128i vecTangentPiTimes3Over8Int = _mm_set1_epi32(kCannyTangentPiTimes3Over8Int);
	compv_uscalar_t col;
	const int stride_ = static_cast<const int>(stride);
	const int c0 = 1 - stride_, c1 = 1 + stride_;

	for (col = 1; col < width - 7; col += 8) { // up to the caller to check that width is >= 8
		vecG = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col]));
		vec0 = _mm_cmpgt_epi16(vecG, vecTLow);
		if (_mm_movemask_epi8(vec0)) {
			vecNMS = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&nms[col]));
			vecGX = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&gx[col]));
			vecGY = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&gy[col]));

			vec1 = _mm_abs_epi16(vecGY);
			vec2 = _mm_abs_epi16(vecGX);

			vecAbsGY0 = _mm_unpacklo_epi16(vecZero, vec1);
			vecAbsGX0 = _mm_unpacklo_epi16(vec2, vecZero);
			vecAbsGY1 = _mm_unpackhi_epi16(vecZero, vec1);
			vecAbsGX1 = _mm_unpackhi_epi16(vec2, vecZero);

			// angle = "0� / 180�"
			vec1 = _mm_cmplt_epi32(vecAbsGY0, _mm_mullo_epi32(vecTangentPiOver8Int, vecAbsGX0));
			vec2 = _mm_cmplt_epi32(vecAbsGY1, _mm_mullo_epi32(vecTangentPiOver8Int, vecAbsGX1));
			vec3 = _mm_and_si128(vec0, _mm_packs_epi32(vec1, vec2));
			if (_mm_movemask_epi8(vec3)) {
				vec1 = _mm_cmpgt_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(&g[col - 1])), vecG);
				vec2 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col + 1])), vecG);
				vec1 = _mm_and_si128(vec3, _mm_or_si128(vec1, vec2));
				vecNMS = _mm_or_si128(_mm_packs_epi16(vec1, vec1), vecNMS);
			}

			// angle = "45� / 225�" or "135 / 315"
			vec4 = _mm_andnot_si128(vec3, vec0);
			if (_mm_movemask_epi8(vec4)) {
				vec1 = _mm_cmplt_epi32(vecAbsGY0, _mm_mullo_epi32(vecTangentPiTimes3Over8Int, vecAbsGX0));
				vec2 = _mm_cmplt_epi32(vecAbsGY1, _mm_mullo_epi32(vecTangentPiTimes3Over8Int, vecAbsGX1));
				vec4 = _mm_and_si128(vec4, _mm_packs_epi32(vec1, vec2));
				if (_mm_movemask_epi8(vec4)) {
					vec1 = _mm_cmplt_epi16(_mm_xor_si128(vecGX, vecGY), vecZero);
					vec1 = _mm_and_si128(vec4, vec1);
					vec2 = _mm_andnot_si128(vec1, vec4);
					if (_mm_movemask_epi8(vec1)) {
						vec5 = _mm_cmpgt_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(&g[col - c0])), vecG);
						vec6 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col + c0])), vecG);
						vec1 = _mm_and_si128(vec1, _mm_or_si128(vec5, vec6));
					}
					if (_mm_movemask_epi8(vec2)) {
						vec5 = _mm_cmpgt_epi16(_mm_load_si128(reinterpret_cast<const __m128i*>(&g[col - c1])), vecG);
						vec6 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col + c1])), vecG);
						vec2 = _mm_and_si128(vec2, _mm_or_si128(vec5, vec6));
					}
					vec1 = _mm_or_si128(vec1, vec2);
					vecNMS = _mm_or_si128(vecNMS, _mm_packs_epi16(vec1, vec1));
				}
			}

			// angle = "90� / 270�"
			vec5 = _mm_andnot_si128(vec3, _mm_andnot_si128(vec4, vec0));
			if (_mm_movemask_epi8(vec5)) {
				vec1 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col - stride])), vecG);
				vec2 = _mm_cmpgt_epi16(_mm_loadu_si128(reinterpret_cast<const __m128i*>(&g[col + stride])), vecG);
				vec1 = _mm_and_si128(vec5, _mm_or_si128(vec1, vec2));
				vecNMS = _mm_or_si128(_mm_packs_epi16(vec1, vec1), vecNMS);
			}

			_mm_storel_epi64(reinterpret_cast<__m128i*>(&nms[col]), vecNMS);
		}
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */