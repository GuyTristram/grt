#ifndef SHARED_H
#define SHARED_H

#include "uncopyable.h"


template< typename T >
class SharedPtr
{
public:
	explicit SharedPtr( T *obj = 0 ) : m_obj( obj )
	{
		if( m_obj ) m_obj->inc_ref();
	}

	SharedPtr( SharedPtr const &other )
	{
		m_obj = other.m_obj;
		if( m_obj ) m_obj->inc_ref();
	}

	SharedPtr<T> &operator=( SharedPtr const &other )
	{
		SharedPtr<T> temp( other );
		swap( temp );
		return *this;
	}

	template< typename Other >
	SharedPtr( SharedPtr<Other> const &other )
	{
		m_obj = other.get();
		if( m_obj ) m_obj->inc_ref();
	}

	template< typename Other >
	SharedPtr<T> &operator=( SharedPtr<Other> const &other )
	{
		SharedPtr<T> temp( other );
		swap( temp );
		return *this;
	}

	~SharedPtr()
	{
		if( m_obj )
		{
			m_obj->dec_ref();
			if( m_obj->is_forgotten() ) delete m_obj;
		}
	}

	void swap( SharedPtr &other )
	{
		T *temp = other.m_obj;
		other.m_obj = m_obj;
		m_obj = temp;
	}

	void set( T *obj ) {SharedPtr<T> p( obj ); swap( p );}
	T *get() const {return m_obj;}
	T *operator->() const {return m_obj;}
	T &operator*() const {return *m_obj;}

	template< typename U >
	bool operator ==( SharedPtr< U > const &other ) const {return m_obj == other.m_obj;}
	template< typename U >
	bool operator !=( SharedPtr< U > const &other ) const {return m_obj != other.m_obj;}

	operator void*() {return m_obj;}
private:
	T *m_obj;
};

class Shared : public Uncopyable
{
public:
	Shared() : m_count( 0 ) {}
	virtual ~Shared() {}

private:
	template<typename T>
	friend class SharedPtr;
	void inc_ref() {++m_count;}
	void dec_ref() {--m_count;}
	bool is_forgotten() {return m_count == 0;}
	int m_count;
};



#endif // SHARED_H
