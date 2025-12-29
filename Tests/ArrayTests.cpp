#include "pch.h"
#include "CppUnitTest.h"

#include "Core/Array.h"

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

			TestStruct& operator=( const TestStruct& oOther )
			{
				m_iValue = oOther.m_iValue;

				++s_uAliveCount;

				return *this;
			}

			TestStruct( const TestStruct&& oOther ) noexcept
				: m_iValue( oOther.m_iValue )
			{
				++s_uAliveCount;
			}

			TestStruct& operator=( TestStruct&& oOther ) noexcept
			{
				m_iValue = oOther.m_iValue;

				++s_uAliveCount;

				return *this;
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
				Array< TestStruct > aFromArray( 3, TestStruct( 3 ) );

				Array< TestStruct > aArray;
				aArray = aFromArray;
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 3u, aArray.Count() );
				Assert::AreEqual( 3u, aArray.Capacity() );
				for( uint u = 0; u < aArray.Count(); ++u )
					Assert::AreEqual( 3, aArray[ u ].m_iValue );
				Assert::AreEqual( 6u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Move assign
			{
				Array< TestStruct > aFromArray( 3, TestStruct( 3 ) );

				Array< TestStruct > aArray;
				aArray = std::move( aFromArray );
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

			// Swap
			{
				Array< TestStruct > aArrayA( 3, TestStruct( 3 ) );
				Array< TestStruct > aArrayB( 4, TestStruct( 4 ) );

				aArrayA.Swap( aArrayB );
				Assert::IsNotNull( aArrayA.Data() );
				Assert::AreEqual( 4u, aArrayA.Count() );
				Assert::AreEqual( 4u, aArrayA.Capacity() );
				Assert::IsNotNull( aArrayB.Data() );
				Assert::AreEqual( 3u, aArrayB.Count() );
				Assert::AreEqual( 3u, aArrayB.Capacity() );
				for( uint u = 0; u < aArrayA.Count(); ++u )
					Assert::AreEqual( 4, aArrayA[ u ].m_iValue );
				for( uint u = 0; u < aArrayB.Count(); ++u )
					Assert::AreEqual( 3, aArrayB[ u ].m_iValue );
				Assert::AreEqual( 7u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Grab
			{
				Array< TestStruct > aArrayA( 3, TestStruct( 3 ) );
				Array< TestStruct > aArrayB( 4, TestStruct( 4 ) );

				aArrayA.Grab( aArrayB );
				Assert::IsNotNull( aArrayA.Data() );
				Assert::AreEqual( 4u, aArrayA.Count() );
				Assert::AreEqual( 4u, aArrayA.Capacity() );
				Assert::IsNotNull( aArrayB.Data() );
				Assert::AreEqual( 0u, aArrayB.Count() );
				Assert::AreEqual( 3u, aArrayB.Capacity() );
				for( uint u = 0; u < aArrayA.Count(); ++u )
					Assert::AreEqual( 4, aArrayA[ u ].m_iValue );
				Assert::AreEqual( 4u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Clear / shrink to fit
			{
				Array< TestStruct > aArray( 3, TestStruct( 3 ) );

				aArray.PopBack();
				aArray.ShrinkToFit();
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 2u, aArray.Count() );
				Assert::AreEqual( 2u, aArray.Capacity() );
				Assert::AreEqual( 2u, TestStruct::s_uAliveCount );

				aArray.Clear();
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 0u, aArray.Count() );
				Assert::AreEqual( 2u, aArray.Capacity() );
				Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

				aArray.ShrinkToFit();
				Assert::IsNull( aArray.Data() );
				Assert::AreEqual( 0u, aArray.Count() );
				Assert::AreEqual( 0u, aArray.Capacity() );
				Assert::AreEqual( 0u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

			// Reserve / resize
			{
				Array< TestStruct > aArray;

				aArray.Reserve( 2 );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 0u, aArray.Count() );
				Assert::AreEqual( 2u, aArray.Capacity() );
				Assert::AreEqual( 0u, TestStruct::s_uAliveCount );

				aArray.PushBack( TestStruct() );
				aArray.PushBack( TestStruct() );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 2u, aArray.Count() );
				Assert::AreEqual( 2u, aArray.Capacity() );
				Assert::AreEqual( 2u, TestStruct::s_uAliveCount );

				aArray.Reserve( 1 );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 2u, aArray.Count() );
				Assert::AreEqual( 2u, aArray.Capacity() );
				Assert::AreEqual( 2u, TestStruct::s_uAliveCount );

				aArray.Resize( 3 );
				aArray.Resize( 5, TestStruct( 10 ) );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 5u, aArray.Count() );
				Assert::AreEqual( 5u, aArray.Capacity() );
				Assert::AreEqual( 0, aArray[ 2 ].m_iValue );
				Assert::AreEqual( 10, aArray[ 3 ].m_iValue );
				Assert::AreEqual( 10, aArray[ 4 ].m_iValue );
				Assert::AreEqual( 5u, TestStruct::s_uAliveCount );

				aArray.Resize( 3 );
				Assert::IsNotNull( aArray.Data() );
				Assert::AreEqual( 3u, aArray.Count() );
				Assert::AreEqual( 5u, aArray.Capacity() );
				Assert::AreEqual( 3u, TestStruct::s_uAliveCount );
			}
			Assert::AreEqual( 0u, TestStruct::s_uAliveCount );
		}

		TEST_METHOD( FastType )
		{
			Array< int > aFromArray;
			aFromArray.PushBack( 1 );
			aFromArray.PushBack( 2 );
			aFromArray.PushBack( 3 );

			Array< int > aArray( aFromArray );
			Assert::IsNotNull( aArray.Data() );
			Assert::AreEqual( 3u, aArray.Count() );
			Assert::AreEqual( 3u, aArray.Capacity() );
			Assert::AreEqual( 1, aArray[ 0 ] );
			Assert::AreEqual( 2, aArray[ 1 ] );
			Assert::AreEqual( 3, aArray[ 2 ] );

			aArray.PushFront( 0 );
			Assert::IsNotNull( aArray.Data() );
			Assert::AreEqual( 4u, aArray.Count() );
			Assert::AreEqual( 4u, aArray.Capacity() );
			Assert::AreEqual( 0, aArray[ 0 ] );
			Assert::AreEqual( 1, aArray[ 1 ] );
			Assert::AreEqual( 2, aArray[ 2 ] );
			Assert::AreEqual( 3, aArray[ 3 ] );
			
			aArray = aFromArray;
			Assert::IsNotNull( aArray.Data() );
			Assert::AreEqual( 3u, aArray.Count() );
			Assert::AreEqual( 3u, aArray.Capacity() );
			Assert::AreEqual( 1, aArray[ 0 ] );
			Assert::AreEqual( 2, aArray[ 1 ] );
			Assert::AreEqual( 3, aArray[ 2 ] );

			aArray.Reserve( 4 );
			Assert::IsNotNull( aArray.Data() );
			Assert::AreEqual( 3u, aArray.Count() );
			Assert::AreEqual( 4u, aArray.Capacity() );
			Assert::AreEqual( 1, aArray[ 0 ] );
			Assert::AreEqual( 2, aArray[ 1 ] );
			Assert::AreEqual( 3, aArray[ 2 ] );

			aArray.Remove( 1 );
			Assert::IsNotNull( aArray.Data() );
			Assert::AreEqual( 2u, aArray.Count() );
			Assert::AreEqual( 4u, aArray.Capacity() );
			Assert::AreEqual( 1, aArray[ 0 ] );
			Assert::AreEqual( 3, aArray[ 1 ] );

			aArray.Resize( 5, 0 );
			Assert::IsNotNull( aArray.Data() );
			Assert::AreEqual( 5u, aArray.Count() );
			Assert::AreEqual( 5u, aArray.Capacity() );
			Assert::AreEqual( 1, aArray[ 0 ] );
			Assert::AreEqual( 3, aArray[ 1 ] );
			Assert::AreEqual( 0, aArray[ 2 ] );
			Assert::AreEqual( 0, aArray[ 3 ] );
			Assert::AreEqual( 0, aArray[ 4 ] );

			aArray.Resize( 2, 0 );
			Assert::IsNotNull( aArray.Data() );
			Assert::AreEqual( 2u, aArray.Count() );
			Assert::AreEqual( 5u, aArray.Capacity() );
			Assert::AreEqual( 1, aArray[ 0 ] );
			Assert::AreEqual( 3, aArray[ 1 ] );
		}

		TEST_METHOD( ResizeSpeedTest )
		{
			Array< uint8 > aVerySimpleArray;
			Array< int > aSimpleArray;
			Array< TestStruct > aComplexArray;

			const uint uCount = 1000000;

			auto t1 = std::chrono::high_resolution_clock::now();
			aVerySimpleArray.Resize( uCount, 1 );
			auto t2 = std::chrono::high_resolution_clock::now();
			aSimpleArray.Resize( uCount, 1 );
			auto t3 = std::chrono::high_resolution_clock::now();
			aComplexArray.Resize( uCount, TestStruct( 1 ) );
			auto t4 = std::chrono::high_resolution_clock::now();

			auto oVerySimpleTime = ( t2 - t1 ).count();
			auto oSimpleTime = ( t3 - t2 ).count();
			auto oComplexTime = ( t4 - t3 ).count();

			// Suspicious if not, but not a hard truth
			Assert::IsTrue( oVerySimpleTime < oSimpleTime );
			Assert::IsTrue( oSimpleTime < oComplexTime );

			// Just check a few because this takes a lot of time
			for( uint u = 0; u < 5; ++u )
			{
				Assert::AreEqual( 1, ( int )aVerySimpleArray[ u ] );
				Assert::AreEqual( 1, aSimpleArray[ u ] );
				Assert::AreEqual( 1, aComplexArray[ u ].m_iValue );
			}
		}
	};

	uint ArrayTests::TestStruct::s_uAliveCount = 0;
}