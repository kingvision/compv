/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/intrinsics/x86/compv_convlt_intrin_sse.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/compv_simd_globals.h"
#include "compv/compv_math.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// This function requires sizeof(float) = 4byte = 32bits
void Convlt1_hz_float32_minpack4_Intrin_SSE2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t pad, const float* hkern_ptr, compv_scalar_t kern_size)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // ASM
	compv_scalar_t i, j, col;
	__m128 xmmCoeff, xmmF0, xmmSF0, xmmSF1, xmmSF2, xmmSF3;
	__m128i xmmI0, xmmI1, xmmI2, xmmI3, xmmZero;

	xmmZero = _mm_setzero_si128();

	pad += (width & 3) ; // 3 = (minpack - 1) = (4 - 1)

	// (abcd conv 0123) = a0+b1+c2+d3
	
	for (j = 0; j < height; ++j) {
		i = width;
		while (i > 15) {
			xmmSF0 = _mm_setzero_ps();
			xmmSF1 = _mm_setzero_ps();
			xmmSF2 = _mm_setzero_ps();
			xmmSF3 = _mm_setzero_ps();
			for (col = 0; col < kern_size; ++col) {
				xmmI0 = _mm_loadu_si128((__m128i*)&in_ptr[col]);
				xmmCoeff = _mm_set1_ps(hkern_ptr[col]); // 0000

				// TODO(dmi): Very good candidate for FMA3 / AVX
				
				xmmI1 = _mm_unpacklo_epi8(xmmI0, xmmZero); // Low(U8) -> Low(I16)

				xmmF0 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(xmmI1, xmmZero)); // I16 -> I32 -> F32
				xmmF0 = _mm_mul_ps(xmmF0, xmmCoeff); // a0b0c0d0
				xmmSF0 = _mm_add_ps(xmmSF0, xmmF0);

				xmmF0 = _mm_cvtepi32_ps(_mm_unpackhi_epi16(xmmI1, xmmZero)); // I16 -> I32 -> F32
				xmmF0 = _mm_mul_ps(xmmF0, xmmCoeff); // e0f0g0h0
				xmmSF1 = _mm_add_ps(xmmSF1, xmmF0);

				xmmI1 = _mm_unpackhi_epi8(xmmI0, xmmZero); // High(U8) -> High(I16)

				xmmF0 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(xmmI1, xmmZero)); // I16 -> I32 -> F32
				xmmF0 = _mm_mul_ps(xmmF0, xmmCoeff); // i0j0k0l0
				xmmSF2 = _mm_add_ps(xmmSF2, xmmF0);

				xmmF0 = _mm_cvtepi32_ps(_mm_unpackhi_epi16(xmmI1, xmmZero)); // I16 -> I32 -> F32
				xmmF0 = _mm_mul_ps(xmmF0, xmmCoeff); // m0n000p0
				xmmSF3 = _mm_add_ps(xmmSF3, xmmF0);
			}

			xmmI0 = _mm_cvtps_epi32(xmmSF0);
			xmmI1 = _mm_cvtps_epi32(xmmSF1);
			xmmI2 = _mm_cvtps_epi32(xmmSF2);
			xmmI3 = _mm_cvtps_epi32(xmmSF3);
			xmmI0 = _mm_packs_epi32(xmmI0, xmmI1);
			xmmI2 = _mm_packs_epi32(xmmI2, xmmI3);
			xmmI0 = _mm_packus_epi16(xmmI0, xmmI2);

			_mm_storeu_si128((__m128i*)out_ptr, xmmI0);
			
			i -= 16;
			in_ptr += 16;
			out_ptr += 16;
		} // while (i > 15)

		while (i > 3) {
			xmmSF0 = _mm_setzero_ps();
			for (col = 0; col < kern_size; ++col) {
				xmmI0 = _mm_cvtsi32_si128(*((uint32_t*)&in_ptr[col]));
				xmmCoeff = _mm_set1_ps(hkern_ptr[col]);

				xmmF0 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(xmmI0, xmmZero), xmmZero));
				xmmF0 = _mm_mul_ps(xmmF0, xmmCoeff);
				xmmSF0 = _mm_add_ps(xmmSF0, xmmF0);
			}
			xmmI0 = _mm_cvtps_epi32(xmmSF0);
			xmmI0 = _mm_packs_epi32(xmmI0, xmmI0);
			xmmI0 = _mm_packus_epi16(xmmI0, xmmI0);

			*((uint32_t*)out_ptr) = (uint32_t)_mm_cvtsi128_si32(xmmI0);

			i -= 4;
			in_ptr += 4;
			out_ptr += 4;
		} // while (i > 4)

		in_ptr += pad;
		out_ptr += pad;
	} // for (j...
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */