#include "Array.h"
#include "Intrusive.h"1

class TestOK : public Intrusive
{
public:
	void f()
	{

	}
};

class TestOK2 : public Intrusive
{
public:
	void g()
	{

	}
};

class TestKO
{

};

struct Blublu
{
	Blublu() : Blublu( 1, 2.f )
	{
	}

	Blublu(int aa, float bb) : a(aa), b(bb), me(this)
	{
	}

	Blublu( const Blublu& dfdf )
	{
		a = dfdf.a;
		b = dfdf.b;
		me = this;
		//__debugbreak();
	}

	Blublu& operator=( const Blublu& b )
	{
		__debugbreak();
	}

	Blublu( Blublu&& dfdf )
	{
		__debugbreak();
	}

	Blublu& operator=( Blublu&& b )
	{
		__debugbreak();
	}

	int a;
	float b;
	Blublu* me;
};

int main()
{
	StrongPtr< TestOK > xTestOK = new TestOK;
	StrongPtr< TestOK > xTestOK2 = xTestOK;
	TestOK2* ptest = new TestOK2;
	StrongPtr< TestOK2 > xTestOK3 = ptest;
	WeakPtr<TestOK2> xWeak( ptest );
	xTestOK3->g();
	xTestOK->f();
	xWeak->g();

	Blublu b;
	Array< Blublu > aArray( 5 );
	Array< Blublu > aArray2( 5 );
	aArray.PushBack( Blublu( 10, 20.f ) );
	aArray.PushBack( Blublu() );
	aArray.PushBack( Blublu() );

	Array< int, ArrayFlags::FAST_RESIZE > aFast;
	Array< int, ArrayFlags::FAST_RESIZE > aFast2( aFast );
}