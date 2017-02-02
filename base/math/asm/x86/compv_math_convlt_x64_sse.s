;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* inPtr
; arg(1) -> uint8_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const compv_float32_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 9
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define vecZero				xmm8
	%define vecSum0				xmm9
	%define vecSum1				xmm5
	%define vecSum2				xmm6
	%define vecSum3				xmm7
	pxor vecZero, vecZero

	%define argi_inPtr			0
	%define argi_outPtr			1
	%define argi_width			2
	%define argi_height			3
	%define argi_step			4 
	%define argi_pad			5
	%define argi_vthzKernPtr	6
	%define argi_kernSize		7

	mov rax, arg(argi_width)
	mov rcx, arg(argi_pad)
	lea rax, [rax + rcx]
	%define argi_stride 5 ; argi_pad
	mov arg(argi_stride), rax ; stride = (pad + width)
	%undef argi_pad

	%define j		rsi
	%define i		rdi
	%define inPtr	r8
	%define outPtr  r9
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 15; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per16Bytes:
			xorps vecSum0, vecSum0
			xorps vecSum1, vecSum1
			xorps vecSum2, vecSum2 
			xorps vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			xor rcx, rcx ; rcx = k = 0
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Bytes:
				movups xmm0, [rax + rcx] ; xmm0 = vecInPtr
				movss xmm4, [rbx + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				shufps xmm4, xmm4, 0x0 ; xmm4 = vecCoeff
				movaps xmm3, xmm0
				inc rdx
				add rcx, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				punpcklbw xmm0, vecZero
				punpckhbw xmm3, vecZero
				movdqa xmm1, xmm0
				movdqa xmm2, xmm3
				punpcklwd xmm0, vecZero
				punpckhwd xmm1, vecZero
				punpcklwd xmm2, vecZero
				punpckhwd xmm3, vecZero
				cvtdq2ps xmm0, xmm0
				cvtdq2ps xmm1, xmm1
				cvtdq2ps xmm2, xmm2
				cvtdq2ps xmm3, xmm3
				mulps xmm0, xmm4
				mulps xmm1, xmm4
				mulps xmm2, xmm4
				mulps xmm3, xmm4
				addps vecSum0, xmm0
				addps vecSum1, xmm1
				addps vecSum2, xmm2
				addps vecSum3, xmm3	
				jl .LoopKernelSize_Per16Bytes
				; EndOf_LoopKernelSize_Per16Bytes ;
			
			cvttps2dq vecSum0, vecSum0
			cvttps2dq vecSum1, vecSum1
			cvttps2dq vecSum2, vecSum2
			cvttps2dq vecSum3, vecSum3
			mov rcx, arg(argi_width)
			lea rcx, [rcx - 15]
			lea i, [i + 16]
			cmp i, rcx
			packssdw vecSum0, vecSum1
			packssdw vecSum2, vecSum3
			packuswb vecSum0, vecSum2
			movdqu [outPtr + i - 16], vecSum0
			jl .LoopWidth_Per16Bytes
			; EndOf_LoopWidth_Per16Bytes ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width - 3; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		mov rax, arg(argi_width)
		lea rax, [rax - 3]
		cmp i, rax
		jge .EndOf_LoopWidth_Per4Bytes
		.LoopWidth_Per4Bytes:
			xorps vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			xor rcx, rcx ; rcx = k = 0
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Bytes:
				movss xmm2, [rax + rcx] ; xmm2 = vecInPtr
				movss xmm3, [rbx + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc rdx
				add rcx, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				punpcklbw xmm2, vecZero
				punpcklwd xmm2, vecZero
				cvtdq2ps xmm2, xmm2
				shufps xmm3, xmm3, 0x0 ; xmm3 = vecCoeff
				mulps xmm2, xmm3
				addps vecSum0, xmm2
				jl .LoopKernelSize_Per4Bytes
				; EndOf_LoopKernelSize_Per4Bytes ;


			mov rcx, arg(argi_width)
			lea rcx, [rcx - 3]
			lea i, [i + 4]
			cmp i, rcx
			cvttps2dq vecSum0, vecSum0
			packssdw vecSum0, vecSum0
			packuswb vecSum0, vecSum0
			movd [outPtr + i - 4], vecSum0
			jl .LoopWidth_Per4Bytes
			.EndOf_LoopWidth_Per4Bytes
			; EndOf_LoopWidth_Per4Bytes ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		mov rax, arg(argi_width)
		cmp i, rax
		jge .EndOf_LoopWidth_Per1Bytes
		.LoopWidth_Per1Bytes:
			xorps vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			xor rcx, rcx ; rcx = k = 0
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Bytes:
				movss xmm2, [rax + rcx] ; TODO(dmi): use movzx for x64, here we cannot because we''re out of 32b registers
				movss xmm3, [rbx + rdx*COMPV_YASM_FLOAT32_SZ_BYTES] ; xmm3 = vecCoeff
				inc rdx
				add rcx, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				punpcklbw xmm2, vecZero
				punpcklwd xmm2, vecZero
				cvtdq2ps xmm2, xmm2
				mulss xmm2, xmm3
				addss vecSum0, xmm2	
				jl .LoopKernelSize_Per1Bytes
				; EndOf_LoopKernelSize_Per1Bytes ;


			inc i
			mov rcx, arg(argi_width)
			cmp i, rcx
			cvttss2si rax, vecSum0
			mov [outPtr + i - 1], byte al
			jl .LoopWidth_Per1Bytes
			.EndOf_LoopWidth_Per1Bytes
			; EndOf_LoopWidth_Per1Bytes ;
		
		mov rdx, arg(argi_stride)
		dec j
		lea inPtr, [inPtr + rdx]
		lea outPtr, [outPtr + rdx]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef vecZero
	%undef vecSum0
	%undef vecSum1
	%undef vecSum2
	%undef vecSum3

	%undef argi_inPtr
	%undef argi_outPtr
	%undef argi_width
	%undef argi_height
	%undef argi_step
	%undef argi_pad
	%undef argi_vthzKernPtr
	%undef argi_kernSize
	%undef argi_stride

	%undef j
	%undef i
	%undef inPtr
	%undef outPtr

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT
