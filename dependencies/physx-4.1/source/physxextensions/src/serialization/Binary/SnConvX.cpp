//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2019 NVIDIA Corporation. All rights reserved.

/*
- get rid of STL

- NpArticulationLinkArray with more than 4 entries
- big convexes Xbox => PC

- put a cache in "convertClass" function
- remove MD one at a time and check what happens in the converter. Make it robust/report errors
- for xbox, compare against source xbox file if it exists
- maybe put some info in the header to display "File generated by ConvX 1.xx" on screen (to debug)
- report inconsistent naming convention in each class!!!!
- try to automatically discover padding bytes? use "0xcd" pattern?
* do last param of XXXX_ITEMS macro automatically
- what if source files are 64bits? can we safely convert a 64bit ptr to 32bit?
*/

#include "foundation/PxErrorCallback.h"
#include "foundation/PxAllocatorCallback.h"
#include "foundation/PxIO.h"
#include "SnConvX.h"
#include "serialization/SnSerializationRegistry.h"
#include <assert.h>
#include "PsFoundation.h"

using namespace physx;

Sn::ConvX::ConvX() :
    mMetaData_Src		(NULL),
	mMetaData_Dst		(NULL),
	mOutStream			(NULL),
	mMustFlip			(false),
	mOutputSize			(0),
	mSrcPtrSize			(0),
	mDstPtrSize			(0),
	mNullPtr			(false),
	mNoOutput			(false),
	mMarkedPadding		(false),
	mNbErrors			(0),
	mNbWarnings			(0),
	mReportMode			(PxConverterReportMode::eNORMAL),
	mPerformConversion	(true)	
{
	//	memset(mZeros, 0, CONVX_ZERO_BUFFER_SIZE);
	memset(mZeros, 0x42, CONVX_ZERO_BUFFER_SIZE);
}

Sn::ConvX::~ConvX()
{
	resetNbErrors();
	resetConvexFlags();
	releaseMetaData();
	resetUnions();
}

void Sn::ConvX::release()
{
	delete this;
}

bool Sn::ConvX::setMetaData(PxInputStream& inputStream, MetaDataType type)
{
	resetNbErrors();
	return (loadMetaData(inputStream, type) != NULL);		
}

bool Sn::ConvX::setMetaData(PxInputStream& srcMetaData, PxInputStream& dstMetaData)
{
	releaseMetaData();
	resetUnions();

	if(!setMetaData(srcMetaData, META_DATA_SRC))
		return false;
	if(!setMetaData(dstMetaData, META_DATA_DST))
		return false;

	return true;
}

bool Sn::ConvX::compareMetaData() const
{
	if (!mMetaData_Src || !mMetaData_Dst) {
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__,
			"PxBinaryConverter: metadata not defined. Call PxBinaryConverter::setMetaData first.\n");
		return false;
	}

	return mMetaData_Src->compare(*mMetaData_Dst);
}

bool Sn::ConvX::convert(PxInputStream& srcStream, PxU32 srcSize, PxOutputStream& targetStream)
{
	if(!mMetaData_Src || !mMetaData_Dst)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__,
			"PxBinaryConverter: metadata not defined. Call PxBinaryConverter::setMetaData first.\n");
		return false;
	}

	resetConvexFlags();
	resetNbErrors();

	bool conversionStatus = false;
	if(mPerformConversion)
	{	
		if(srcSize == 0)
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
				"PxBinaryConverter: source serialized data size is zero.\n");
			return false;
		}

		void* memory = PX_ALLOC_TEMP(srcSize+ALIGN_FILE, "ConvX source file");
		void* memoryA = reinterpret_cast<void*>((size_t(memory) + ALIGN_FILE)&~(ALIGN_FILE-1));

		const PxU32 nbBytesRead = srcStream.read(memoryA, srcSize);
		if(nbBytesRead != srcSize)
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
				"PxBinaryConverter: failure on reading source serialized data.\n");
			PX_FREE(memory);
			return false;
		}

		displayMessage(PxErrorCode::eDEBUG_INFO, "\n\nConverting...\n\n");

		{
			if(!initOutput(targetStream))
			{
				PX_FREE(memory);
				return false;
			}
			conversionStatus = convert(memoryA, int(srcSize));
			closeOutput();
		}

		PX_FREE(memory);
	}
	return conversionStatus;
}
