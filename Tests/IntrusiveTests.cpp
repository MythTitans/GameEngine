#include "pch.h"
#include "CppUnitTest.h"

#include "Core/Intrusive.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS( IntrusiveTests )
	{
		struct TestIntrusive : public Intrusive
		{
			static uint s_uAliveCount;

			TestIntrusive()
			{
				++s_uAliveCount;
			}

			~TestIntrusive()
			{
				--s_uAliveCount;
			}
		};

	public:
		TEST_METHOD( StrongPtrTest )
		{
			Assert::AreEqual( 0u, TestIntrusive::s_uAliveCount );

			TestIntrusive* pTestIntrusive = new TestIntrusive;
			Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
			Assert::AreEqual( 0u, pTestIntrusive->GetReferenceCount() );

			// Test constructing and destructing StrongPtr
			{
				StrongPtr< TestIntrusive > xStrongPtr( pTestIntrusive );
				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::IsTrue( pTestIntrusive == xStrongPtr.GetPtr() );

				// Test changing StrongPtr to nullptr
				{
					StrongPtr< TestIntrusive > xStrongPtrAdditional( pTestIntrusive );
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 2u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xStrongPtrAdditional.GetPtr() );
					xStrongPtrAdditional = nullptr;
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xStrongPtr.GetPtr() );
					Assert::IsTrue( nullptr == xStrongPtrAdditional.GetPtr() );
				}
				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::IsTrue( pTestIntrusive == xStrongPtr.GetPtr() );

				// Test copying another StrongPtr to StrongPtr and destructing
				{
					TestIntrusive* pTestIntrusiveAdditional = new TestIntrusive;
					StrongPtr< TestIntrusive > xStrongPtrAdditional( pTestIntrusiveAdditional );
					Assert::AreEqual( 2u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 1u, pTestIntrusiveAdditional->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusiveAdditional == xStrongPtrAdditional.GetPtr() );

					StrongPtr< TestIntrusive > xStrongPtrTest( pTestIntrusive );
					xStrongPtrAdditional = xStrongPtrTest;
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 3u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xStrongPtrAdditional.GetPtr() );
				}
				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::IsTrue( pTestIntrusive == xStrongPtr.GetPtr() );

				// Test moving another StrongPtr to StrongPtr and destructing
				{
					TestIntrusive* pTestIntrusiveAdditional = new TestIntrusive;
					StrongPtr< TestIntrusive > xStrongPtrAdditional( pTestIntrusiveAdditional );
					Assert::AreEqual( 2u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 1u, pTestIntrusiveAdditional->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusiveAdditional == xStrongPtrAdditional.GetPtr() );

					StrongPtr< TestIntrusive > xStrongPtrTest( pTestIntrusive );
					xStrongPtrAdditional = std::move( xStrongPtrTest );
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 2u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xStrongPtrAdditional.GetPtr() );
				}
				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::IsTrue( pTestIntrusive == xStrongPtr.GetPtr() );

				// Test constructing, copying, moving and destructing StrongPtr
				{
					StrongPtr< TestIntrusive > xStrongPtrCopyCtor( xStrongPtr );
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 2u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xStrongPtrCopyCtor.GetPtr() );

					StrongPtr< TestIntrusive > xStrongPtrCopyAssign;
					xStrongPtrCopyAssign = xStrongPtr;
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 3u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xStrongPtrCopyAssign.GetPtr() );

					xStrongPtrCopyAssign = xStrongPtrCopyAssign;
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 3u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xStrongPtrCopyAssign.GetPtr() );

					StrongPtr< TestIntrusive > xStrongPtrMoveCtor( std::move( xStrongPtrCopyCtor ) );
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 3u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xStrongPtrMoveCtor.GetPtr() );
					Assert::IsTrue( nullptr == xStrongPtrCopyCtor.GetPtr() );

					StrongPtr< TestIntrusive > xStrongPtrMoveAssign;
					xStrongPtrMoveAssign = std::move( xStrongPtrCopyAssign );
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 3u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xStrongPtrMoveAssign.GetPtr() );
					Assert::IsTrue( nullptr == xStrongPtrCopyAssign.GetPtr() );

					xStrongPtrMoveAssign = std::move( xStrongPtrMoveAssign );
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 3u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xStrongPtrMoveAssign.GetPtr() );
				}
				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::IsTrue( pTestIntrusive == xStrongPtr.GetPtr() );
			}
			
			Assert::AreEqual( 0u, TestIntrusive::s_uAliveCount );
		}

		TEST_METHOD( WeakPtrTest )
		{
			Assert::AreEqual( 0u, TestIntrusive::s_uAliveCount );

			TestIntrusive* pTestIntrusive = new TestIntrusive;
			Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
			Assert::AreEqual( 0u, pTestIntrusive->GetReferenceCount() );

			StrongPtr< TestIntrusive > xStrongPtr( pTestIntrusive );
			Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
			Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
			Assert::AreEqual( 0u, pTestIntrusive->CountWeakReferences() );

			// Test constructing, copying and destructing WeakPtr
			{
				TestIntrusive* pTestIntrusiveAdditional = new TestIntrusive;
				StrongPtr< TestIntrusive > xStrongPtrAdditional( pTestIntrusiveAdditional );
				Assert::AreEqual( 2u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusiveAdditional->GetReferenceCount() );
				Assert::AreEqual( 0u, pTestIntrusive->CountWeakReferences() );

				WeakPtr< TestIntrusive > xWeakPtr( pTestIntrusive );
				Assert::AreEqual( 2u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::AreEqual( 1u, pTestIntrusive->CountWeakReferences() );
				Assert::IsTrue( pTestIntrusive == xWeakPtr.GetPtr() );

				WeakPtr< TestIntrusive > xWeakPtrTest( pTestIntrusiveAdditional );
				Assert::AreEqual( 2u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusiveAdditional->GetReferenceCount() );
				Assert::AreEqual( 1u, pTestIntrusiveAdditional->CountWeakReferences() );
				Assert::IsTrue( pTestIntrusiveAdditional == xWeakPtrTest.GetPtr() );

				xWeakPtrTest = xWeakPtr;
				Assert::AreEqual( 2u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::AreEqual( 2u, pTestIntrusive->CountWeakReferences() );
				Assert::AreEqual( 1u, pTestIntrusiveAdditional->GetReferenceCount() );
				Assert::AreEqual( 0u, pTestIntrusiveAdditional->CountWeakReferences() );
				Assert::IsTrue( pTestIntrusive == xWeakPtrTest.GetPtr() );
			}
			Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
			Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
			Assert::AreEqual( 0u, pTestIntrusive->CountWeakReferences() );

			// Test creating and copying WeakPtr and destructing StrongPtr
			{
				TestIntrusive* pTestIntrusiveAdditional = new TestIntrusive;
				WeakPtr< TestIntrusive > xWeakPtrTest;
				{
					StrongPtr< TestIntrusive > xStrongPtrAdditional( pTestIntrusiveAdditional );
					Assert::AreEqual( 2u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 1u, pTestIntrusiveAdditional->GetReferenceCount() );
					Assert::AreEqual( 0u, pTestIntrusive->CountWeakReferences() );

					xWeakPtrTest = pTestIntrusiveAdditional;
					Assert::AreEqual( 2u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 1u, pTestIntrusiveAdditional->GetReferenceCount() );
					Assert::AreEqual( 1u, pTestIntrusiveAdditional->CountWeakReferences() );
					Assert::IsTrue( pTestIntrusiveAdditional == xWeakPtrTest.GetPtr() );
				}

				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::IsTrue( nullptr == xWeakPtrTest.GetPtr() );
			}
			Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
			Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
			Assert::AreEqual( 0u, pTestIntrusive->CountWeakReferences() );

			// Test creating and destructing WeakPtr and changing StrongPtr affectation
			{
				TestIntrusive* pTestIntrusiveAdditional = new TestIntrusive;
				StrongPtr< TestIntrusive > xStrongPtrAdditional( pTestIntrusiveAdditional );
				Assert::AreEqual( 2u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusiveAdditional->GetReferenceCount() );
				Assert::AreEqual( 0u, pTestIntrusive->CountWeakReferences() );

				WeakPtr< TestIntrusive > xWeakPtrTest( pTestIntrusiveAdditional );
				Assert::AreEqual( 2u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusiveAdditional->GetReferenceCount() );
				Assert::AreEqual( 1u, pTestIntrusiveAdditional->CountWeakReferences() );
				Assert::IsTrue( pTestIntrusiveAdditional == xWeakPtrTest.GetPtr() );

				xStrongPtrAdditional = xStrongPtr;
				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 2u, pTestIntrusive->GetReferenceCount() );
				Assert::AreEqual( 0u, pTestIntrusive->CountWeakReferences() );
				Assert::IsTrue( nullptr == xWeakPtrTest.GetPtr() );
			}
			Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
			Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
			Assert::AreEqual( 0u, pTestIntrusive->CountWeakReferences() );

			// Test creating, copying and destructing WeakPtr and changing StrongPtr to nullptr
			{
				WeakPtr< TestIntrusive > xWeakPtr0( pTestIntrusive );
				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::AreEqual( 1u, pTestIntrusive->CountWeakReferences() );
				Assert::IsTrue( pTestIntrusive == xWeakPtr0.GetPtr() );

				WeakPtr< TestIntrusive > xWeakPtr1( xWeakPtr0 );
				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::AreEqual( 2u, pTestIntrusive->CountWeakReferences() );
				Assert::IsTrue( pTestIntrusive == xWeakPtr1.GetPtr() );

				{
					WeakPtr< TestIntrusive > xWeakPtr2( pTestIntrusive );
					Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
					Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
					Assert::IsTrue( pTestIntrusive == xWeakPtr2.GetPtr() );
					Assert::AreEqual( 3u, pTestIntrusive->CountWeakReferences() );
				}
				Assert::AreEqual( 2u, pTestIntrusive->CountWeakReferences() );

				WeakPtr< TestIntrusive > xWeakPtr3( pTestIntrusive );
				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::AreEqual( 3u, pTestIntrusive->CountWeakReferences() );
				Assert::IsTrue( pTestIntrusive == xWeakPtr3.GetPtr() );

				WeakPtr< TestIntrusive > xWeakPtr4;
				xWeakPtr4 = xWeakPtr3;
				Assert::AreEqual( 1u, TestIntrusive::s_uAliveCount );
				Assert::AreEqual( 1u, pTestIntrusive->GetReferenceCount() );
				Assert::AreEqual( 4u, pTestIntrusive->CountWeakReferences() );
				Assert::IsTrue( pTestIntrusive == xWeakPtr4.GetPtr() );

				xWeakPtr0 = nullptr;
				Assert::AreEqual( 3u, pTestIntrusive->CountWeakReferences() );
				xWeakPtr3 = nullptr;
				Assert::AreEqual( 2u, pTestIntrusive->CountWeakReferences() );

				xStrongPtr = nullptr;

				Assert::AreEqual( 0u, TestIntrusive::s_uAliveCount );
				Assert::IsTrue( nullptr == xWeakPtr0.GetPtr() );
				Assert::IsTrue( nullptr == xWeakPtr1.GetPtr() );
				Assert::IsTrue( nullptr == xWeakPtr3.GetPtr() );
				Assert::IsTrue( nullptr == xWeakPtr4.GetPtr() );
			}

			Assert::AreEqual( 0u, TestIntrusive::s_uAliveCount );
		}
	};

	uint IntrusiveTests::TestIntrusive::s_uAliveCount = 0;
}
