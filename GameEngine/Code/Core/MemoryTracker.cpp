#include "MemoryTracker.h"

#include "Array.h"
#include "Game/ComponentManager.h"
#include "Game/InputHandler.h"
#include "ImGui/imgui.h"
#include "Intrusive.h"
#include "Profiler.h"
#include "StringUtils.h"

static std::string GetDisplayableTypeName( const std::type_index oTypeIndex )
{
	std::string sDisplayableType = oTypeIndex.name();

	Replace( sDisplayableType, "struct ", "" );
	Replace( sDisplayableType, "class ", "" );
	Replace( sDisplayableType, " * __ptr64", "*" );
	Replace( sDisplayableType, "std::basic_string<char,std::char_traits<char>,std::allocator<char> >", "std::string" );
	Replace( sDisplayableType, "glm::vec<3,float,0>", "glm::vec3" );
	Replace( sDisplayableType, "glm::qua<float,0>", "glm::quat" );
	Replace( sDisplayableType, "glm::mat<4,4,float,0>", "glm::mat4" );

	return sDisplayableType;
}

static std::string GetDisplayableMemory( const uint64 uMemory )
{
	if( uMemory < 1024 )
	{
		return std::format( "{} B", uMemory );
	}
	
	if( uMemory < 1024 * 1024 )
	{
		return std::format( "{:.3f} KB", uMemory / 1024.f );
	}
	
	if( uMemory < 1024 * 1024 * 1024 )
	{
		return std::format( "{:.3f} MB", uMemory / ( 1024.f * 1024.f ) );
	}

	return std::format( "{:.3f} GB", uMemory / ( 1024.f * 1024.f * 1024.f ) );
}

IntrusiveMemory::IntrusiveMemory()
	: m_uBytes( 0 )
	, m_uCount( 0 )
{
}

ComponentMemory::ComponentMemory( const std::type_index oComponentType, const uint64 uComponentSize, const ComponentsHolderBase* pComponentHolder )
	: m_oComponentType( oComponentType )
	, m_uComponentSize( uComponentSize )
	, m_pComponentHolder( pComponentHolder )
{
}

ArrayMemory::ArrayMemory()
	: m_uUsedBytes( 0 )
	, m_uReservedBytes( 0 )
	, m_uArrayCount( 0 )
	, m_uElementCount( 0 )
{
}

ArrayReference::ArrayReference( const ArrayBase* pArray, const std::type_index oArrayType, const uint64 uArrayTypeSize )
	: m_pArray( pArray )
	, m_oArrayType( oArrayType )
	, m_uArrayTypeSize( uArrayTypeSize )
{
}

MemoryTracker* g_pMemoryTracker = nullptr;

MemoryTracker::MemoryTracker()
	: m_bDisplayMemoryTracker( false )
{
	g_pMemoryTracker = this;
}

MemoryTracker::~MemoryTracker()
{
	g_pMemoryTracker = nullptr;
}

void MemoryTracker::Display()
{
	ProfilerBlock oBlock( "MemoryTracker" );

	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_TOGGLE_MEMORY_TRACKER ) )
		m_bDisplayMemoryTracker = !m_bDisplayMemoryTracker;

	if( m_bDisplayMemoryTracker )
	{
		ImGui::Begin( "Memory tracker" );

		if( ImGui::CollapsingHeader( "Intrusives" ) )
		{
			uint64 uTotalBytes = 0;
			uint uTotalCount = 0;

			std::unordered_map< std::type_index, IntrusiveMemory > mObjects;

			for( const Intrusive* pIntrusive : m_lIntrusives )
			{
				IntrusiveMemory& oObjectMemory = mObjects[ typeid( *pIntrusive ) ];
				oObjectMemory.m_uBytes += sizeof( *pIntrusive );
				oObjectMemory.m_uCount += 1;
			}

			if( ImGui::BeginTable( "IntrusiveTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp ) )
			{
				ImGui::TableSetupColumn( "Type" );
				ImGui::TableSetupColumn( "Memory" );
				ImGui::TableSetupColumn( "Count" );
				ImGui::TableHeadersRow();

				for( const auto& oPair : mObjects )
				{
					const IntrusiveMemory& oObjectMemory = oPair.second;

					uTotalBytes += oObjectMemory.m_uBytes;
					uTotalCount += oObjectMemory.m_uCount;

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex( 0 );
					ImGui::Text( GetDisplayableTypeName( oPair.first ).c_str() );
					ImGui::TableSetColumnIndex( 1 );
					ImGui::Text( GetDisplayableMemory( oObjectMemory.m_uBytes ).c_str() );
					ImGui::TableSetColumnIndex( 2 );
					ImGui::Text( "%d", oObjectMemory.m_uCount);
				}
				ImGui::EndTable();
			}

			ImGui::Text( "Total : Memory %s, Count %d", GetDisplayableMemory( uTotalBytes ).c_str(), uTotalCount );
		}

		if( ImGui::CollapsingHeader( "Components" ) )
		{
			uint64 uTotalBytes = 0;

			if( ImGui::BeginTable( "ComponentsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp ) )
			{
				ImGui::TableSetupColumn( "Type" );
				ImGui::TableSetupColumn( "Memory" );
				ImGui::TableSetupColumn( "Count" );
				ImGui::TableHeadersRow();

				for( const ComponentMemory& oComponentMemory : m_aComponents )
				{
					const uint64 uBytes = oComponentMemory.m_uComponentSize * oComponentMemory.m_pComponentHolder->GetCount();
					uTotalBytes += uBytes;

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex( 0 );
					ImGui::Text( GetDisplayableTypeName( oComponentMemory.m_oComponentType ).c_str() );
					ImGui::TableSetColumnIndex( 1 );
					ImGui::Text( GetDisplayableMemory( uBytes ).c_str() );
					ImGui::TableSetColumnIndex( 2 );
					ImGui::Text( "%d", oComponentMemory.m_pComponentHolder->GetCount() );
				}
				ImGui::EndTable();
			}

			ImGui::Text( "Total : Memory %s", GetDisplayableMemory( uTotalBytes ).c_str() );
		}

		if( ImGui::CollapsingHeader( "Arrays" ) )
		{
			uint64 uTotalUsedBytes = 0;
			uint64 uTotalReservedBytes = 0;

			std::unordered_map< std::type_index, ArrayMemory > mArrays;

			for( const ArrayReference& oMemory : m_lArrays )
			{
				if( oMemory.m_pArray->Capacity() == 0 )
					continue;

				ArrayMemory& oArrayMemory = mArrays[ oMemory.m_oArrayType ];
				oArrayMemory.m_uUsedBytes += oMemory.m_uArrayTypeSize * oMemory.m_pArray->Count();
				oArrayMemory.m_uReservedBytes += oMemory.m_uArrayTypeSize * oMemory.m_pArray->Capacity();
				oArrayMemory.m_uArrayCount += 1;
				oArrayMemory.m_uElementCount += oMemory.m_pArray->Count();
			}

			if( ImGui::BeginTable( "ArrayTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp ) )
			{
				ImGui::TableSetupColumn( "Type" );
				ImGui::TableSetupColumn( "Used" );
				ImGui::TableSetupColumn( "Reserved" );
				ImGui::TableSetupColumn( "Usage ratio" );
				ImGui::TableSetupColumn( "Elements" );
				ImGui::TableSetupColumn( "Instances" );
				ImGui::TableHeadersRow();

				for( const auto& oPair : mArrays )
				{
					const ArrayMemory& oArrayMemory = oPair.second;
					if( oArrayMemory.m_uReservedBytes == 0 )
						continue;

					float fRatio = ( float )( ( double )oArrayMemory.m_uUsedBytes / ( double )oArrayMemory.m_uReservedBytes );

					uTotalUsedBytes += oArrayMemory.m_uUsedBytes;
					uTotalReservedBytes += oArrayMemory.m_uReservedBytes;

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex( 0 );
					ImGui::Text( GetDisplayableTypeName( oPair.first ).c_str() );
					ImGui::TableSetColumnIndex( 1 );
					ImGui::Text( GetDisplayableMemory( oArrayMemory.m_uUsedBytes ).c_str() );
					ImGui::TableSetColumnIndex( 2 );
					ImGui::Text( GetDisplayableMemory( oArrayMemory.m_uReservedBytes ).c_str() );
					ImGui::TableSetColumnIndex( 3 );
					ImGui::ProgressBar( fRatio, ImVec2( ImGui::GetContentRegionAvail().x, 0.0f ) );
					ImGui::TableSetColumnIndex( 4 );
					ImGui::Text( "%d", oArrayMemory.m_uElementCount );
					ImGui::TableSetColumnIndex( 5 );
					ImGui::Text( "%d", oArrayMemory.m_uArrayCount );
				}
				ImGui::EndTable();
			}

			float fRatio = ( float )( ( double )uTotalUsedBytes / ( double )uTotalReservedBytes );
			ImGui::Text( "Total : Used %s, Reserved %s, Usage Ratio %.0f%%", GetDisplayableMemory( uTotalUsedBytes ).c_str(), GetDisplayableMemory( uTotalReservedBytes ).c_str(), fRatio * 100.f );
		}

		ImGui::End();
	}
}

void MemoryTracker::RegisterIntrusive( const Intrusive* pIntrusive )
{
	std::unique_lock oLock( m_oMutex );

	m_lIntrusives.push_back( pIntrusive );
}

void MemoryTracker::UnRegisterIntrusive( const Intrusive* pIntrusive )
{
	std::unique_lock oLock( m_oMutex );

	m_lIntrusives.remove( pIntrusive );
}

void MemoryTracker::UnRegisterArray( ArrayBase* pArray )
{
	std::unique_lock oLock( m_oMutex );

	m_lArrays.remove_if( [ pArray ]( ArrayReference oArrayMemory ) { return oArrayMemory.m_pArray == pArray; } );
}
