#include "pch.h"
#include "CppUnitTest.h"

#include "Array.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS( ArrayTests )
	{
		struct TestStruct
		{
			static uint s_uAliveCount;

			TestStruct()
				: m_iValue( 0 )
			{
				++s_uAliveCount;
			}

			explicit TestStruct( const int iValue )
				: m_iValue( iValue )
			{
				++s_uAliveCount;
			}

			TestStruct( const TestStruct& oOther )
				: m_iValue( oOther.m_iValue )
			{
				++s_uAliveCount;
			}

			TestStruct( const TestStruct&& oOther ) noexcept
				: m_iValue( oOther.m_iValue )
			{
				++s_uAliveCount;
			}

			~TestStruct()
			{
				--s_uAliveCount;
			}

			int m_iValue;
		};

	public:
		TEST_METHOD( ConstructionTest )
		{
			// Empty array
			{
				Array< TestStruct > aArray;
				Assert::IsNull( aArray.Data() );
				Assert::AreEqual( 0u, aArray.Count() );
				Assert::AreEqual( 0u, aArray.Capacity() );
				Assert::AreEqual( 0u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );
			
			// Array with default elements
			{
				Array< TestStruct > aArray( 3 );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 3u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				for( uint u = 0; u < aArray.Count(); ++u )
					Assert::AreEqual( 0, aArray[ u ].m_iValue );
				Assert::AreEqual( 3u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );
			
			// Array with non default elements
			{
				Array< TestStruct > aArray( 3, TestStruct( 3 ) );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 3u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				for( uint u = 0; u < aArray.Count(); ++u )
					Assert::AreEqual( 3, aArray[ u ].m_iValue );
				Assert::AreEqual( 3u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Copy array
			{
				Array< TestStruct > aFromArray( 3, TestStruct( 3 ) );

				Array< TestStruct > aArray( aFromArray );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 3u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				for( uint u = 0; u < aArray.Count(); ++u )
					Assert::AreEqual( 3, aArray[ u ].m_iValue );
				Assert::AreEqual( 6u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Move array
			{
				Array< TestStruct > aFromArray( 3, TestStruct( 3 ) );

				Array< TestStruct > aArray( std::move( aFromArray ) );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 3u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				for( uint u = 0; u < aArray.Count(); ++u )
					Assert::AreEqual( 3, aArray[ u ].m_iValue );
				Assert::AreEqual( 3u, TestStruct::s_uAliveCount );

				Assert::IsNull( aFromArray.Data() );
				Assert::AreEqual( 0u, aFromArray.Count() );
				Assert::AreEqual( 0u, aFromArray.Capacity() );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );
		}

		TEST_METHOD( ManipulationTest )
		{
			// Push/pop back
			{
				Array< TestStruct > aArray;

				aArray.PushBack( TestStruct( 1 ) );
				aArray.PushBack( TestStruct( 2 ) );
				aArray.PushBack( TestStruct( 3 ) );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 3u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				Assert::AreEqual( 1, aArray.Front().m_iValue );
				Assert::AreEqual( 2, aArray[ 1 ].m_iValue );
				Assert::AreEqual( 3, aArray.Back().m_iValue );
				Assert::AreEqual( 3u, TestStruct::s_uAliveCount );

				aArray.PopBack();
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 2u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				Assert::AreEqual( 1, aArray.Front().m_iValue );
				Assert::AreEqual( 2, aArray.Back().m_iValue );
				Assert::AreEqual( 2u, TestStruct::s_uAliveCount );

				aArray.PopBack();
				aArray.PopBack();
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 0u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				Assert::AreEqual( 0u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Push/pop front
			{
				Array< TestStruct > aArray;

				aArray.PushFront( TestStruct( 1 ) );
				aArray.PushFront( TestStruct( 2 ) );
				aArray.PushFront( TestStruct( 3 ) );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 3u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				Assert::AreEqual( 3, aArray.Front().m_iValue);
				Assert::AreEqual( 2, aArray[ 1 ].m_iValue );
				Assert::AreEqual( 1, aArray.Back().m_iValue );
				Assert::AreEqual( 3u, TestStruct::s_uAliveCount );

				aArray.PopFront();
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 2u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				Assert::AreEqual( 2, aArray.Front().m_iValue );
				Assert::AreEqual( 1, aArray.Back().m_iValue );
				Assert::AreEqual( 2u, TestStruct::s_uAliveCount );

				aArray.PopFront();
				aArray.PopFront();
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 0u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				Assert::AreEqual( 0u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Remove
			{
				Array< TestStruct > aArray;

				aArray.PushBack( TestStruct( 1 ) );
				aArray.PushBack( TestStruct( 2 ) );
				aArray.PushBack( TestStruct( 3 ) );
				aArray.Remove( 1 );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 2u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				Assert::AreEqual( 1, aArray.Front().m_iValue );
				Assert::AreEqual( 3, aArray.Back().m_iValue );
				Assert::AreEqual( 2u, TestStruct::s_uAliveCount );

				aArray.Remove( 0 );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 1u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				Assert::AreEqual( 3, aArray.Front().m_iValue );
				Assert::AreEqual( 1u, TestStruct::s_uAliveCount );

				aArray.Remove( 0 );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 0u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				Assert::AreEqual( 0u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Copy assign
			{

			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Move assign
			{

			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Clear / shrink to fit
			{

			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Reserve / resize
			{

			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );
		}
	};

	uint ArrayTests::TestStruct::s_uAliveCount = 0;
}