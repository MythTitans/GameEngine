#pragma once

#include <nlohmann/json.hpp>
#include <typeindex>
#include <unordered_map>

#include "Core/MemoryTracker.h"
#include "Core/Profiler.h"
#include "Core/Serialization.h"
#include "Editor/Inspector.h"
#include "Scene.h"

#define REGISTER_COMPONENT( COMPONENT )					\
static bool b##COMPONENT##Registered = []() {			\
	ComponentManager::RegisterComponent< COMPONENT >();	\
	return true;										\
}()

#define PROPERTIES( CLASS ) using PropertyClass = CLASS

#define PROPERTY( NAME, FIELD, TYPE )											\
TYPE FIELD = []() {																\
	static bool bRegistered = []() {											\
		RegisterProperty< TYPE, PropertyClass >( NAME, &PropertyClass::FIELD );	\
		return true;															\
	}();																		\
																				\
	return TYPE();																\
}()

#define PROPERTY_DEFAULT( NAME, FIELD, TYPE, DEFAULT )							\
TYPE FIELD = []() {																\
	static bool bRegistered = []() {											\
		RegisterProperty< TYPE, PropertyClass >( NAME, &PropertyClass::FIELD );	\
		return true;															\
	}();																		\
																				\
	return DEFAULT;																\
}()

struct GameContext;

struct PropertiesHolderBase
{
	virtual ~PropertiesHolderBase();

	virtual Array< nlohmann::json > Serialize( const void* pClass ) const = 0;
	virtual void					Deserialize( const nlohmann::json& oJsonContent, void* pClass ) = 0;
	virtual Array< std::string >	DisplayInspector( void* pClass ) = 0;

	Array< std::string, ArrayFlags::NO_TRACKING > m_aNames;
};

template < typename PropertyType, typename PropertyClass >
struct PropertiesHolder : PropertiesHolderBase
{
	Array< nlohmann::json > Serialize( const void* pClass ) const override
	{
		const PropertyClass* pTypedClass = static_cast< const PropertyClass* >( pClass );

		Array< nlohmann::json > aSerializedProperties;
		aSerializedProperties.Reserve( m_aNames.Count() );
		for( uint u = 0; u < m_aNames.Count(); ++u )
		{
			nlohmann::json oSerializedProperty;
			oSerializedProperty[ m_aNames[ u ] ] = pTypedClass->*m_aProperties[ u ];
			aSerializedProperties.PushBack( oSerializedProperty );
		}

		return aSerializedProperties;
	}

	void Deserialize( const nlohmann::json& oJsonContent, void* pClass ) override
	{
		PropertyClass* pTypedClass = static_cast< PropertyClass* >( pClass );

		for( uint u = 0; u < m_aNames.Count(); ++u )
		{
			for( const auto& it : oJsonContent.items() )
			{
				const nlohmann::json& oProperty = it.value();
				if( oProperty.contains( m_aNames[ u ] ) )
					pTypedClass->*m_aProperties[ u ] = oProperty[ m_aNames[ u ] ];
			}
		}
	}

	Array< std::string > DisplayInspector( void* pClass ) override
	{
		PropertyClass* pTypedClass = static_cast< PropertyClass* >( pClass );

		Array< std::string > aPropertiesChanged;
		for( uint u = 0; u < m_aNames.Count(); ++u )
		{
			if( ::DisplayInspector( m_aNames[ u ].c_str(), pTypedClass->*m_aProperties[ u ] ) )
				aPropertiesChanged.PushBack( m_aNames[ u ] );
		}

		return aPropertiesChanged;
	}

	Array< PropertyType PropertyClass::*, ArrayFlags::NO_TRACKING > m_aProperties;
};

enum class ComponentManagement : uint8
{
	NONE,
	INITIALIZE,
	INITIALIZE_THEN_START
};

class ComponentsHolderBase
{
public:
	ComponentsHolderBase();
	virtual ~ComponentsHolderBase();

	virtual void			InitializeComponents() = 0;
	virtual bool			AreComponentsInitialized() const = 0;
	virtual void			StartPendingComponents() = 0;
	virtual void			StartComponents() = 0;
	virtual void			StopComponents() = 0;
	virtual void			DisposeComponent( Entity* pEntity ) = 0;
	virtual void			TickComponents() = 0;
	virtual void			NotifyBeforePhysicsOnComponents() = 0;
	virtual void			NotifyAfterPhysicsOnComponents() = 0;
	virtual void			UpdateComponents( const GameContext& oGameContext ) = 0;

	virtual nlohmann::json	SerializeComponent( const Entity* pEntity ) const = 0;
	virtual void			DeserializeComponent( const nlohmann::json& oJsonContent, const Entity* pEntity ) = 0;

	virtual void			DisplayInspector( const Entity* pEntity ) = 0;
	virtual void			DisplayGizmos( const uint64 uSelectedEntityID ) = 0;

	virtual uint			GetCount() const = 0;
	virtual uint			GetDisposedCount() const = 0;

	uint m_uVersion;
};

class ComponentManager;

template < typename ComponentType >
class ComponentsHolder : public ComponentsHolderBase
{
public:
	ComponentsHolder()
	{
#ifdef TRACK_MEMORY
		g_pMemoryTracker->RegisterComponent< ComponentType >( this );
#endif
	}

	void InitializeComponents() override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( uint u = 0; u < m_aComponents.Count(); ++u )
			InitializeComponentFromIndex( u );
	}

	bool AreComponentsInitialized() const override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				continue;

			if( m_aStates[ u ] == ComponentState::UNINITIALIZED || m_aComponents[ u ].IsInitialized() == false )
				return false;
		}

		return true;
	}

	void StartPendingComponents() override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( int u = ( int )m_aPendingComponents.Count() - 1; u >= 0; --u )
		{
			const uint uIndex = m_aPendingComponents[ u ];
			if( m_aStates[ uIndex ] == ComponentState::DISPOSED )
			{
				m_aPendingComponents.Remove( u );
				return;
			}

			if( m_aComponents[ uIndex ].IsInitialized() )
			{
				m_aPendingComponents.Remove( u );
				StartComponentFromIndex( uIndex );
				return;
			}
		}
	}

	void StartComponents() override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			StartComponentFromIndex( u );
		}
	}

	void StopComponents() override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			StopComponentFromIndex( u );
		}
	}

	void TickComponents() override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				continue;

			if( m_aStates[ u ] == ComponentState::STARTED )
				m_aComponents[ u ].Tick();
		}
	}

	void NotifyBeforePhysicsOnComponents() override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				continue;

			if( m_aStates[ u ] == ComponentState::STARTED )
				m_aComponents[ u ].BeforePhysics();
		}
	}

	void NotifyAfterPhysicsOnComponents() override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				continue;

			if( m_aStates[ u ] == ComponentState::STARTED )
				m_aComponents[ u ].AfterPhysics();
		}
	}

	void UpdateComponents( const GameContext& oGameContext ) override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				continue;

			if( m_aStates[ u ] == ComponentState::STARTED )
				m_aComponents[ u ].Update( oGameContext );
		}
	}

	nlohmann::json SerializeComponent( const Entity* pEntity ) const override
	{
		nlohmann::json oJsonContent;

		if( ComponentManager::GetComponentsFactory().find( GetComponentName() ) == ComponentManager::GetComponentsFactory().end() )
			return oJsonContent;

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				continue;

			const ComponentType& oComponent = m_aComponents[ u ];
			if( oComponent.m_pEntity == pEntity )
			{
				oJsonContent[ "name" ] = GetComponentName();

				Array< nlohmann::json > aSerializedProperties;
				for( const auto& it : s_mProperties )
				{
					const Array< nlohmann::json > aSerializedTypeProperties = it.second->Serialize( &oComponent );
					for( const nlohmann::json& oSerializedTypeProperty : aSerializedTypeProperties )
						aSerializedProperties.PushBack( oSerializedTypeProperty );
				}

				if( aSerializedProperties.Empty() == false )
					oJsonContent[ "properties" ] = aSerializedProperties;

				return oJsonContent;
			}
		}

		return oJsonContent;
	}

	void DeserializeComponent( const nlohmann::json& oJsonContent, const Entity* pEntity ) override
	{
		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				continue;

			ComponentType& oComponent = m_aComponents[ u ];
			if( oComponent.m_pEntity == pEntity )
			{
				for( const auto& it : s_mProperties )
					it.second->Deserialize( oJsonContent, &oComponent );

				break;
			}
		}
	}

	void DisplayInspector( const Entity* pEntity ) override
	{
		if( ComponentManager::GetComponentsFactory().find( GetComponentName() ) == ComponentManager::GetComponentsFactory().end() )
			return;

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				continue;

			ComponentType& oComponent = m_aComponents[ u ];
			if( oComponent.m_pEntity == pEntity )
			{
				ImGui::Separator();

				if( ImGui::CollapsingHeader( GetComponentName().c_str(), ImGuiTreeNodeFlags_DefaultOpen ) )
				{
					ImGui::Indent();

					int iImGuiID = 0;
					ImGui::PushID( iImGuiID++ );
					for( const auto& it : s_mProperties )
					{
						Array< std::string > aChangedProperties = it.second->DisplayInspector( &oComponent );
						for( const std::string& sChangedProperty : aChangedProperties )
							oComponent.OnPropertyChanged( sChangedProperty );
					}
					ImGui::PopID();

					ImGui::PushID( iImGuiID++ );
					oComponent.DisplayInspector();
					ImGui::PopID();

					ImGui::Unindent();
				}

				break;
			}
		}
	}

	void DisplayGizmos( const uint64 uSelectedEntityID ) override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				continue;

			ComponentType& oComponent = m_aComponents[ u ];
			oComponent.DisplayGizmos( oComponent.GetEntity()->GetID() == uSelectedEntityID );
		}
	}

	ComponentType* CreateComponent( Entity* pEntity, const ComponentManagement eComponentManagement )
	{
		for( uint u = 0; u < m_aStates.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
			{
				m_aComponents[ u ] = ComponentType( pEntity );
				m_aStates[ u ] = ComponentState::UNINITIALIZED;

				if( eComponentManagement != ComponentManagement::NONE )
					InitializeComponentFromIndex( u );

				if( eComponentManagement == ComponentManagement::INITIALIZE_THEN_START )
					m_aPendingComponents.PushBack( u );

				return &m_aComponents[ u ];
			}
		}

		m_aComponents.PushBack( ComponentType( pEntity ) );
		m_aStates.PushBack( ComponentState::UNINITIALIZED );

		if( eComponentManagement != ComponentManagement::NONE )
			InitializeComponentFromIndex( m_aComponents.Count() - 1 );

		if( eComponentManagement == ComponentManagement::INITIALIZE_THEN_START )
			m_aPendingComponents.PushBack( m_aComponents.Count() - 1 );

		return &m_aComponents.Back();
	}

	void InitializeComponent( Entity* pEntity )
	{
		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aComponents[ u ].m_pEntity == pEntity )
			{
				InitializeComponentFromIndex( u );
				break;
			}
		}
	}

	void InitializeComponentFromIndex( const int iIndex )
	{
		if( iIndex < 0 )
			return;

		ASSERT( iIndex < ( int )m_aComponents.Count() );

		if( m_aStates[ iIndex ] == ComponentState::DISPOSED )
			return;

		if( m_aStates[ iIndex ] == ComponentState::UNINITIALIZED )
		{
			m_aComponents[ iIndex ].Initialize();
			m_aStates[ iIndex ] = ComponentState::STOPPED;
		}
	}

	void StartComponent( Entity* pEntity )
	{
		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aComponents[ u ].m_pEntity == pEntity )
			{
				StartComponentFromIndex( u );
				break;
			}
		}
	}

	void StartComponentFromIndex( const int iIndex )
	{
		if( iIndex < 0 )
			return;

		ASSERT( iIndex < ( int )m_aComponents.Count() );

		if( m_aStates[ iIndex ] == ComponentState::DISPOSED )
			return;

		if( m_aStates[ iIndex ] == ComponentState::STOPPED )
		{
			m_aComponents[ iIndex ].Start();
			m_aStates[ iIndex ] = ComponentState::STARTED;
		}
	}

	void StopComponent( Entity* pEntity )
	{
		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aComponents[ u ].m_pEntity == pEntity )
			{
				StopComponentFromIndex( u );
				break;
			}
		}
	}

	void StopComponentFromIndex( const int iIndex )
	{
		if( iIndex < 0 )
			return;

		ASSERT( iIndex < ( int )m_aComponents.Count() );

		if( m_aStates[ iIndex ] == ComponentState::DISPOSED )
			return;

		if( m_aStates[ iIndex ] == ComponentState::STARTED )
		{
			m_aComponents[ iIndex ].Stop();
			m_aStates[ iIndex ] = ComponentState::STOPPED;
		}
	}

	void DisposeComponent( Entity* pEntity ) override
	{
		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aComponents[ u ].m_pEntity == pEntity )
			{
				DisposeComponentFromIndex( u );
				break;
			}
		}
	}

	void DisposeComponentFromIndex( const int iIndex )
	{
		if( iIndex < 0 )
			return;

		ASSERT( iIndex < ( int )m_aComponents.Count() );

		if( m_aStates[ iIndex ] == ComponentState::DISPOSED )
			return;

		if( m_aStates[ iIndex ] == ComponentState::STARTED )
		{
			m_aComponents[ iIndex ].Stop();
			m_aStates[ iIndex ] = ComponentState::STOPPED;
		}

		m_aComponents[ iIndex ].Dispose();
		m_aStates[ iIndex ] = ComponentState::DISPOSED;
		++m_uVersion;
	}

	ComponentType* GetComponent( const Entity* pEntity )
	{
		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aComponents[ u ].m_pEntity == pEntity )
				return GetComponentFromIndex( u );
		}

		return nullptr;
	}

	ComponentType* GetComponentFromIndex( const int iIndex )
	{
		if( iIndex < 0 )
			return nullptr;

		ASSERT( iIndex < ( int )m_aComponents.Count() );

		if( m_aStates[ iIndex ] == ComponentState::DISPOSED )
			return nullptr;

		return &m_aComponents[ iIndex ];
	}

	int GetComponentIndexFromEntity( const Entity* pEntity )
	{
		int iIndex = -1;

		for( uint u = 0; u < m_aComponents.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				continue;

			if( m_aComponents[ u ].m_pEntity == pEntity )
			{
				iIndex = u;
				break;
			}
		}

		return iIndex;
	}

	// TODO #eric I would like to remove this function but it's still needed for Editor and Renderer at the moment
	ArrayView< ComponentType > GetComponents()
	{
		return m_aComponents;
	}

	uint GetCount() const
	{
		return m_aComponents.Count();
	}

	uint GetDisposedCount() const
	{
		int iCount = 0;
		for( uint u = 0; u < m_aStates.Count(); ++u )
		{
			if( m_aStates[ u ] == ComponentState::DISPOSED )
				++iCount;
		}

		return iCount;
	}

	static const std::string& GetComponentName()
	{
		static const std::string sComponentName = []() {
			const std::string sName = std::string( typeid( ComponentType ).name() );
			return sName.substr( sName.find( " " ) + 1 );
		}();
		
		return sComponentName;
	}

	using ComponentProperties = std::unordered_map< std::type_index, PropertiesHolderBase* >;
	static ComponentProperties	s_mProperties;

private:
	enum ComponentState : uint8
	{
		UNINITIALIZED,
		STARTED,
		STOPPED,
		DISPOSED
	};

	Array< ComponentType >	m_aComponents;
	Array< ComponentState >	m_aStates;

	Array< uint >			m_aPendingComponents;
};

template < typename ComponentType >
ComponentsHolder< ComponentType >::ComponentProperties ComponentsHolder< ComponentType >::s_mProperties;

template < typename PropertyType, typename PropertyClass >
void RegisterProperty( const char* sName, PropertyType PropertyClass::* pProperty )
{
	PropertiesHolderBase*& pPropertiesHolderBase = ComponentsHolder< PropertyClass >::s_mProperties[ typeid( PropertyType ) ];
	if( pPropertiesHolderBase == nullptr )
		pPropertiesHolderBase = new PropertiesHolder< PropertyType, PropertyClass >;

	PropertiesHolder< PropertyType, PropertyClass >* pPropertiesHolder = static_cast< PropertiesHolder< PropertyType, PropertyClass >* >( pPropertiesHolderBase );
	pPropertiesHolder->m_aNames.PushBack( sName );
	pPropertiesHolder->m_aProperties.PushBack( pProperty );
}

class ComponentManager;

extern ComponentManager* g_pComponentManager;

// TODO #eric don't forget the version when dealing with destroyed components
class ComponentManager
{
public:
	template < typename ComponentType >
	friend class ComponentHandle;

	template < typename ComponentType >
	friend class ComponentsHolder;

	friend class Scene;
	friend class GameWorld;

	// TODO #eric I would like to remove these friendships but it's still needed for Editor and Renderer at the moment
	friend class Editor;
	friend class Renderer;

	ComponentManager();
	~ComponentManager();

	template < typename ComponentType >
	ComponentHandle< ComponentType > CreateComponent( Entity* pEntity, const ComponentManagement eComponentManagement = ComponentManagement::INITIALIZE_THEN_START )
	{
		ComponentsHolderBase*& pComponentsHolderBase = m_mComponentsHolders[ typeid( ComponentType ) ];
		if( pComponentsHolderBase == nullptr )
			pComponentsHolderBase = new ComponentsHolder< ComponentType >;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( pComponentsHolderBase );
		return pComponentsHolder->CreateComponent( pEntity, eComponentManagement );
	}

	template < typename ComponentType >
	void InitializeComponent( Entity* pEntity )
	{
		ComponentsHolder< ComponentType >* pComponentsHolder = GetComponentsHolder< ComponentType >();
		if( pComponentsHolder == nullptr )
			return;

		pComponentsHolder->InitializeComponent( pEntity );
	}

	template < typename ComponentType >
	void InitializeComponentFromIndex( const int iIndex )
	{
		ComponentsHolder< ComponentType >* pComponentsHolder = GetComponentsHolder< ComponentType >();
		if( pComponentsHolder == nullptr )
			return;

		pComponentsHolder->InitializeComponentFromIndex( iIndex );
	}

	template < typename ComponentType >
	void StartComponent( Entity* pEntity )
	{
		ComponentsHolder< ComponentType >* pComponentsHolder = GetComponentsHolder< ComponentType >();
		if( pComponentsHolder == nullptr )
			return;

		pComponentsHolder->StartComponent( pEntity );
	}

	template < typename ComponentType >
	void StartComponentFromIndex( const int iIndex )
	{
		ComponentsHolder< ComponentType >* pComponentsHolder = GetComponentsHolder< ComponentType >();
		if( pComponentsHolder == nullptr )
			return;

		pComponentsHolder->StartComponentFromIndex( iIndex );
	}

	template < typename ComponentType >
	void StopComponent( Entity* pEntity )
	{
		ComponentsHolder< ComponentType >* pComponentsHolder = GetComponentsHolder< ComponentType >();
		if( pComponentsHolder == nullptr )
			return;

		pComponentsHolder->StopComponent( pEntity );
	}

	template < typename ComponentType >
	void StopComponentFromIndex( const int iIndex )
	{
		ComponentsHolder< ComponentType >* pComponentsHolder = GetComponentsHolder< ComponentType >();
		if( pComponentsHolder == nullptr )
			return;

		pComponentsHolder->StopComponentFromIndex( iIndex );
	}

	template < typename ComponentType >
	void DisposeComponent( Entity* pEntity )
	{
		ComponentsHolder< ComponentType >* pComponentsHolder = GetComponentsHolder< ComponentType >();
		if( pComponentsHolder == nullptr )
			return;

		pComponentsHolder->DisposeComponent( pEntity );
	}

	template < typename ComponentType >
	void DisposeComponentFromIndex( const int iIndex )
	{
		ComponentsHolder< ComponentType >* pComponentsHolder = GetComponentsHolder< ComponentType >();
		if( pComponentsHolder == nullptr )
			return;

		pComponentsHolder->DisposeComponentFromIndex( iIndex );
	}

	template < typename ComponentType >
	ComponentHandle< ComponentType > GetComponent( const Entity* pEntity )
	{
		auto it = m_mComponentsHolders.find( typeid( ComponentType ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return nullptr;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( it->second );
		return pComponentsHolder->GetComponent( pEntity );
	}

	template < typename ComponentType >
	ComponentType* GetComponentFromIndex( const int iIndex )
	{
		auto it = m_mComponentsHolders.find( typeid( ComponentType ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return nullptr;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( it->second );
		return pComponentsHolder->GetComponentFromIndex( iIndex );
	}

	template < typename ComponentType >
	int GetComponentIndexFromEntity( const Entity* pEntity )
	{
		auto it = m_mComponentsHolders.find( typeid( ComponentType ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return -1;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( it->second );
		return pComponentsHolder->GetComponentIndexFromEntity( pEntity );
	}

	void					InitializeComponents();
	bool					AreComponentsInitialized() const;
	void					StartPendingComponents();
	void					StartComponents();
	void					StopComponents();
	void					DisposeComponents( Entity* pEntity );
	void					TickComponents();
	void					NotifyBeforePhysicsOnComponents();
	void					NotifyAfterPhysicsOnComponents();
	void					UpdateComponents( const GameContext& oGameContext );

	Array< nlohmann::json >	SerializeComponents( const Entity* pEntity );
	void					DeserializeComponents( const nlohmann::json& oJsonContent, Entity* pEntity );

	void					DisplayInspector( Entity* pEntity );
	void					DisplayGizmos( const uint64 uSelectedEntityID );

	template < typename ComponentType >
	static void RegisterComponent()
	{
		ComponentFactory& oFactory = GetComponentsFactory()[ ComponentsHolder< ComponentType >::GetComponentName() ];
		oFactory.m_pCreate = []( Entity* pEntity, const ComponentManagement eComponentManagement ) { g_pComponentManager->CreateComponent< ComponentType >( pEntity, eComponentManagement ); };
		oFactory.m_pDispose = []( Entity* pEntity ) { g_pComponentManager->DisposeComponent< ComponentType >( pEntity ); };
	}

private:
	// TODO #eric I would like to remove this function but it's still needed for Editor and Renderer at the moment
	template < typename ComponentType >
	ArrayView< ComponentType > GetComponents()
	{
		auto it = m_mComponentsHolders.find( typeid( ComponentType ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return ArrayView< ComponentType >();

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( it->second );
		return pComponentsHolder->GetComponents();
	}

	template < typename ComponentType >
	ComponentsHolder< ComponentType >* GetComponentsHolder()
	{
		auto it = m_mComponentsHolders.find( typeid( ComponentType ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return nullptr;

		return static_cast< ComponentsHolder< ComponentType >* >( it->second );
	}

	template < typename ComponentType >
	uint GetVersion()
	{
		auto it = m_mComponentsHolders.find( typeid( ComponentType ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return UINT_MAX;

		return it->second->m_uVersion;
	}

	struct ComponentFactory
	{
		std::function< void( Entity*, const ComponentManagement ) > m_pCreate;
		std::function< void( Entity* ) > m_pDispose;
	};

	static std::unordered_map< std::string, ComponentFactory >& GetComponentsFactory()
	{
		static std::unordered_map< std::string, ComponentFactory > s_mComponentsFactory;
		return s_mComponentsFactory;
	}

	std::unordered_map< std::type_index, ComponentsHolderBase* > m_mComponentsHolders;
};