#include "pch.h"
#include "CppUnitTest.h"

#include "Core/ArrayUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS( ArrayUtilsTests )
	{
		struct TestStruct
		{
			explicit TestStruct( const int iValue )
				: m_iValue( iValue )
			{
			}

			bool operator==( const TestStruct& oOther ) const
			{
				return m_iValue == oOther.m_iValue;
			}

			bool operator<( const TestStruct& oOther ) const
			{
				return m_iValue < oOther.m_iValue;
			}

			int m_iValue;
		};

	public:
		TEST_METHOD( ContainsTest )
		{
			Array< TestStruct > aArray;
			aArray.PushBack( TestStruct( 5 ) );
			aArray.PushBack( TestStruct( 10 ) );
			aArray.PushBack( TestStruct( 5 ) );

			Assert::IsTrue( Contains( aArray, TestStruct( 5 ) ) );
			Assert::IsTrue( Contains( aArray, TestStruct( 10 ) ) );
			Assert::IsFalse( Contains( aArray, TestStruct( 15 ) ) );
		}

		TEST_METHOD( FindTest )
		{
			Array< TestStruct > aArray;
			aArray.PushBack( TestStruct( 5 ) );
			aArray.PushBack( TestStruct( 10 ) );
			aArray.PushBack( TestStruct( 5 ) );

			Assert::AreEqual( 0, Find( aArray, TestStruct( 5 ) ) );
			Assert::AreEqual( 1, Find( aArray, TestStruct( 10 ) ) );
			Assert::AreEqual( -1, Find( aArray, TestStruct( 15 ) ) );
		}

		TEST_METHOD( SortTest )
		{
			Array< TestStruct > aArray;
			aArray.PushBack( TestStruct( 5 ) );
			aArray.PushBack( TestStruct( 10 ) );
			aArray.PushBack( TestStruct( 5 ) );

			Sort( aArray, []( const TestStruct& oA, const TestStruct& oB ) { return oA < oB; } );

			Assert::AreEqual( 5, aArray[ 0 ].m_iValue );
			Assert::AreEqual( 5, aArray[ 1 ].m_iValue );
			Assert::AreEqual( 10, aArray[ 2 ].m_iValue );

			Sort( aArray, []( const TestStruct& oA, const TestStruct& oB ) { return oB < oA; } );

			Assert::AreEqual( 10, aArray[ 0 ].m_iValue );
			Assert::AreEqual( 5, aArray[ 1 ].m_iValue );
			Assert::AreEqual( 5, aArray[ 2 ].m_iValue );
		}
	};
}