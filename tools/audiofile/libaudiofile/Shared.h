/*
	Copyright (C) 2010, Michael Pruett. All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

	3. The name of the author may not be used to endorse or promote products
	derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SHARED_H
#define SHARED_H

template <typename T>
class Shared
{
public:
	Shared() : m_refCount(0)
	{
	}
	void retain() { m_refCount++; }
	void release() { if (--m_refCount == 0) delete static_cast<T *>(this); }

protected:
	~Shared()
	{
	}

private:
	int m_refCount;

	Shared(const Shared &);
	Shared &operator =(const Shared &);
};

template <typename T>
class SharedPtr
{
public:
	SharedPtr() : m_ptr(0)
	{
	}
	SharedPtr(T *ptr) : m_ptr(ptr)
	{
		if (m_ptr) m_ptr->retain();
	}
	SharedPtr(const SharedPtr &p) : m_ptr(p.m_ptr)
	{
		if (m_ptr) m_ptr->retain();
	}
	~SharedPtr()
	{
		if (T *p = m_ptr) p->release();
	}

	SharedPtr &operator =(T *ptr)
	{
		if (m_ptr != ptr)
		{
			if (ptr) ptr->retain();
			if (m_ptr) m_ptr->release();
			m_ptr = ptr;
		}
		return *this;
	}
	SharedPtr &operator =(const SharedPtr &p)
	{
		if (m_ptr != p.m_ptr)
		{
			if (p.m_ptr) p.m_ptr->retain();
			if (m_ptr) m_ptr->release();
			m_ptr = p.m_ptr;
		}
		return *this;
	}

	T *get() const { return m_ptr; }
	T &operator *() const { return *m_ptr; }
	T *operator ->() const { return m_ptr; }

	typedef T *SharedPtr::*UnspecifiedBoolType;
	operator UnspecifiedBoolType() const { return m_ptr ? &SharedPtr::m_ptr : 0; }

	bool operator !() const { return !m_ptr; }

private:
	T *m_ptr;
};

#endif
