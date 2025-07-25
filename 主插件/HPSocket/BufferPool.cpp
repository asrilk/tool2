﻿

#include "stdafx.h"
#include "BufferPool.h"
//#include "WaitFor.h"

#pragma warning(push)
#pragma warning(disable: 4458)

const DWORD TItem::DEFAULT_ITEM_CAPACITY			= 4096;
const DWORD CBufferPool::DEFAULT_MAX_CACHE_SIZE		= 0;
const DWORD CBufferPool::DEFAULT_ITEM_CAPACITY		= CItemPool::DEFAULT_ITEM_CAPACITY;
const DWORD CBufferPool::DEFAULT_ITEM_POOL_SIZE		= CItemPool::DEFAULT_POOL_SIZE;
const DWORD CBufferPool::DEFAULT_ITEM_POOL_HOLD		= CItemPool::DEFAULT_POOL_HOLD;
const DWORD CBufferPool::DEFAULT_BUFFER_LOCK_TIME	= (30 * 1000);
const DWORD CBufferPool::DEFAULT_BUFFER_POOL_SIZE	= 600;
const DWORD CBufferPool::DEFAULT_BUFFER_POOL_HOLD	= 600;

TItem* TItem::Construct(CPrivateHeap& heap, int capacity, BYTE* pData, int length)
{
	ASSERT(capacity > 0);

	int item_size	= sizeof(TItem);
	TItem* pItem	= (TItem*)heap.Alloc(item_size + capacity);
	BYTE* pHead		= (BYTE*)pItem + item_size;

	pItem->TItem::TItem(heap, pHead, capacity, pData, length);

	return pItem;
}

void TItem::Destruct(TItem* pItem)
{
	ASSERT(pItem != nullptr);

	CPrivateHeap& heap = pItem->GetPrivateHeap();
	pItem->TItem::~TItem();
	heap.Free(pItem);
}

int TItem::Cat(const BYTE* pData, int length)
{
	ASSERT(pData != nullptr && length >= 0);

	int cat = min(Remain(), length);

	if(cat > 0)
	{
		memcpy(end, pData, cat);
		end += cat;
	}

	return cat;
}

int TItem::Cat(const TItem& other)
{
	ASSERT(this != &other);
	return Cat(other.Ptr(), other.Size());
}

int TItem::Fetch(BYTE* pData, int length)
{
	ASSERT(pData != nullptr && length > 0);

	int fetch = min(Size(), length);
	memcpy(pData, begin, fetch);
	begin	 += fetch;

	return fetch;
}

int TItem::Peek(BYTE* pData, int length)
{
	ASSERT(pData != nullptr && length > 0);

	int peek = min(Size(), length);
	memcpy(pData, begin, peek);

	return peek;
}

int TItem::Increase(int length)
{
	ASSERT(length >= 0);

	int increase = min(Remain(), length);
	end			+= increase;

	return increase;
}

int TItem::Reduce(int length)
{
	ASSERT(length >= 0);

	int reduce   = min(Size(), length);
	begin		+= reduce;

	return reduce;
}

void	TItem::Reset(int first, int last)
{
	ASSERT(first >= -1 && first <= capacity);
	ASSERT(last >= -1 && last <= capacity);

	if(first >= 0)	begin	= head + min(first, capacity);
	if(last >= 0)	end		= head + min(last, capacity);
}

TBuffer* TBuffer::Construct(CBufferPool& pool, ULONG_PTR dwID)
{
	ASSERT(dwID != 0);

	CPrivateHeap& heap	= pool.GetPrivateHeap();
	TBuffer* pBuffer	= (TBuffer*)heap.Alloc(sizeof(TBuffer));

	pBuffer->TBuffer::TBuffer(heap, pool.GetItemPool(), dwID);

	return pBuffer;
}

void TBuffer::Destruct(TBuffer* pBuffer)
{
	ASSERT(pBuffer != nullptr);

	CPrivateHeap& heap = pBuffer->heap;
	pBuffer->TBuffer::~TBuffer();
	heap.Free(pBuffer);
}

void TBuffer::Reset()
{
	id		 = 0;
	length	 = 0;
	freeTime = ::timeGetTime();
}

int TBuffer::Cat(const BYTE* pData, int len)
{
	items.Cat(pData, len);
	return IncreaseLength(len);
}

int TBuffer::Cat(const TItem* pItem)
{
	items.Cat(pItem);
	return IncreaseLength(pItem->Size());
}

int TBuffer::Cat(const TItemList& other)
{
	ASSERT(&items != &other);

	for(TItem* pItem = other.Front(); pItem != nullptr; pItem = pItem->next)
		Cat(pItem);

	return length;
}

int TBuffer::Fetch(BYTE* pData, int len)
{
	int fetch = items.Fetch(pData, len);
	DecreaseLength(fetch);

	return fetch;
}

int TBuffer::Peek(BYTE* pData, int len)
{
	return items.Peek(pData, len);
}

int TBuffer::Reduce(int len)
{
	int reduce = items.Reduce(len);
	DecreaseLength(reduce);

	return reduce;
}

void CBufferPool::PutFreeBuffer(ULONG_PTR dwID)
{
	ASSERT(dwID != 0);

	TBuffer* pBuffer = FindCacheBuffer(dwID);

	if(pBuffer != nullptr)
		PutFreeBuffer(pBuffer);
}

void CBufferPool::PutFreeBuffer(TBuffer* pBuffer)
{
	ASSERT(pBuffer != nullptr);

	if(!pBuffer->IsValid())
		return;

	m_bfCache.RemoveEx(pBuffer->ID());

	BOOL bOK = FALSE;

	{
		CCriSecLock locallock(pBuffer->cs);

		if(pBuffer->IsValid())
		{
			pBuffer->Reset();
			bOK = TRUE;
		}
	}

	if(bOK)
	{
		m_itPool.PutFreeItem(pBuffer->items);

		ReleaseGCBuffer();

		if(!m_lsFreeBuffer.TryPut(pBuffer))
			m_lsGCBuffer.PushBack(pBuffer);
	}
}

void CBufferPool::ReleaseGCBuffer(BOOL bForce)
{
	::ReleaseGCObj(m_lsGCBuffer, m_dwBufferLockTime, bForce);
}

TBuffer* CBufferPool::PutCacheBuffer(ULONG_PTR dwID)
{
	ASSERT(dwID != 0);

	TBuffer* pBuffer = PickFreeBuffer(dwID);
	m_bfCache.SetEx(dwID, pBuffer);

	return pBuffer;
}

TBuffer* CBufferPool::PickFreeBuffer(ULONG_PTR dwID)
{
	ASSERT( dwID != 0);

	DWORD dwIndex;
	TBuffer* pBuffer = nullptr;

	if(m_lsFreeBuffer.TryLock(&pBuffer, dwIndex))
	{
		if(::GetTimeGap32(pBuffer->freeTime) >= m_dwBufferLockTime)
			ENSURE(m_lsFreeBuffer.ReleaseLock(nullptr, dwIndex));
		else
		{
			ENSURE(m_lsFreeBuffer.ReleaseLock(pBuffer, dwIndex));
			pBuffer = nullptr;
		}
	}

	if(pBuffer)	pBuffer->id	= dwID;
	else		pBuffer		= TBuffer::Construct(*this, dwID);

	ASSERT(pBuffer);
	return pBuffer;
}

TBuffer* CBufferPool::FindCacheBuffer(ULONG_PTR dwID)
{
	ASSERT(dwID != 0);

	TBuffer* pBuffer = nullptr;

	if(m_bfCache.GetEx(dwID, &pBuffer) != TBufferCache::GR_VALID)
		pBuffer = nullptr;

	return pBuffer;
}

void CBufferPool::Prepare()
{
	m_itPool.Prepare();

	m_bfCache.Reset(m_dwMaxCacheSize);
	m_lsFreeBuffer.Reset(m_dwBufferPoolSize);
}

void CBufferPool::Clear()
{
	TBufferCache::IndexSet& indexes = m_bfCache.Indexes();

	for(auto it = indexes.begin(), end = indexes.end(); it != end; ++it)
	{
		TBuffer* pBuffer = FindCacheBuffer(*it);
		if(pBuffer) TBuffer::Destruct(pBuffer);
	}

	m_bfCache.Reset();

	m_lsFreeBuffer.Clear();

	ReleaseGCBuffer(TRUE);
	ENSURE(m_lsGCBuffer.IsEmpty());

	m_itPool.Clear();
	m_heap.Reset();
}

#pragma warning(pop)
