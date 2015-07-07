#pragma once

#include <Windows.h>
#include <malloc.h>

template<class T>
class CLdList
{
public:
	CLdList()
	{
		m_List = NULL;
		m_Count = 0;
		m_Capacity = 100;
		ResetList();
	};
	CLdList(int size)
	{
		m_List = NULL;
		m_Count = 0;
		m_Capacity = size;
		RestList();
	};
	~CLdList()
	{
		Clear();
	};
	void Clear()
	{
		m_Capacity = 0;
		free(m_List);
	};
	int GetCount()
	{
		return m_Count;
	};
	T operator [](int nidx)
	{
		return m_List[nidx];
	};
	int Add( T item )
	{
		m_Count++;
		if(m_Count>=m_Capacity){
			m_Capacity += 100;
			ResetList();
		}
		//m_List[m_Count] = item;
		CopyMemory(&m_List[m_Count-1], &item, sizeof(item));
		return m_Count;
	};
	int Insert(int nidx, T item)
	{
		if(nidx<0)
			return -1;
		if(nidx>=m_Count)
			return Add(item);

		m_Count++;
		if(m_Count>=m_Capacity){
			m_Capacity += 100;
			ResetList();
		}

		CopyMemory(PCHAR(m_List)+(nidx+1)*sizeof(T), PCHAR(m_List)+(nidx)*sizeof(T), (m_Count-nidx-1)*sizeof(T));
		m_List[nidx] = item;

		return m_Count;
	};

	T Remove(int nidx)
	{
		if(nidx<0 || nidx>=m_Count)
			return NULL;
		T result = m_List[nidx];
		CopyMemory(PCHAR(m_List)+(nidx)*sizeof(T), PCHAR(m_List)+(nidx+1)*sizeof(T), (m_Count-nidx-1)*sizeof(T));
		return result;
	};
private:
	int m_Count;
	int m_Capacity;
	T* m_List;
	void ResetList()
	{
		m_List = (T*)realloc(m_List, m_Capacity*sizeof(T));
	};
};
