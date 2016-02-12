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
#if !defined(_COMPV_PRALLEL_THREADDISP_H_)
#define _COMPV_PRALLEL_THREADDISP_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"
#include "compv/parallel/compv_asynctask.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVThreadDispatcher : public CompVObj
{
protected:
	CompVThreadDispatcher(int32_t numThreads);
public:
	virtual ~CompVThreadDispatcher();
	virtual COMPV_INLINE const char* getObjectId() { return "CompVThreadDispatcher"; };
	COMPV_INLINE int32_t getThreadsCount() { return m_nTasksCount; }

	COMPV_ERROR_CODE execute(uint32_t threadIdx, compv_asynctoken_id_t tokenId, compv_asynctoken_f f_func, ...);
	COMPV_ERROR_CODE wait(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t u_timeout = 86400000/* 1 day */);
	COMPV_ERROR_CODE getIdleTime(uint32_t threadIdx, compv_asynctoken_id_t tokenId, uint64_t* timeIdle);
	uint32_t getThreadIdxByCoreId(compv_core_id_t coreId);
	uint32_t getThreadIdxForCurrentCore();
	uint32_t getThreadIdxForNextToCurrentCore();
	uint32_t getThreadIdxCurrent();
	bool isMotherOfTheCurrentThread();
	int32_t guessNumThreadsDividingAcrossY(int xcount, int ycount, int minSamplesPerThread);

	static COMPV_ERROR_CODE newObj(CompVObjWrapper<CompVThreadDispatcher*>* disp, int32_t numThreads = -1);

private:
	CompVObjWrapper<CompVAsyncTask *>* m_pTasks;
	int32_t m_nTasksCount;
	bool m_bValid;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PRALLEL_THREADDISP_H_ */