#pragma once

#include <nlohmann/json.hpp>
#include <typeindex>
#include <unordered_map>

#include "Core/MemoryTracker.h"
#include "Core/Profiler.h"
#include "Core/Serialization.h"
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

struct PropertiesHolderBase
{
	virtual ~PropertiesHolderBase();

	virtual Array< nlohmann::json > Serialize( const void* pClass ) const = 0;
	virtual void					Deserialize( const nlohmann::json& oJsonContent, void* pClass ) = 0;

	Array< std::string > m_aNames;
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

	Array< PropertyType PropertyClass::* > m_aProperties;
};

struct ComponentsHolderBase
{
	ComponentsHolderBase();
	virtual ~ComponentsHolderBase();

	virtual void			InitializeComponents() = 0;
	virtual bool			AreComponentsInitialized() const = 0;
	virtual void			StartComponents() = 0;
	virtual void			StopComponents() = 0;
	virtual void			UpdateComponents( const float fDeltaTime ) = 0;

	virtual nlohmann::json	SerializeComponent( const Entity* pEntity ) const = 0;
	virtual void			DeserializeComponent( const nlohmann::json& oJsonContent, const Entity* pEntity ) = 0;

	virtual void			DisplayGizmos( const uint64 uSelectedEntityID ) = 0;

	virtual uint			GetCount() const = 0;

	uint m_uVersion;
};

class ComponentManager;

template < typename ComponentType >
struct ComponentsHolder : ComponentsHolderBase
{
	ComponentsHolder()
	{
#ifdef TRACK_MEMORY
		g_pMemoryTracker->RegisterComponent< ComponentType >( this );
#endif
	}

	void InitializeComponents() override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Initialize();
	}

	bool AreComponentsInitialized() const override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( const ComponentType& oComponent : m_aComponents )
		{
			if( oComponent.IsInitialized() == false )
				return false;
		}

		return true;
	}

	void StartComponents() override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Start();
	}

	void StopComponents() override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Stop();
	}

	void UpdateComponents( const float fDeltaTime ) override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Update( fDeltaTime );
	}

	void DisplayGizmos( const uint64 uSelectedEntityID ) override
	{
		ProfilerBlock oBlock( GetComponentName().c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.DisplayGizmos( oComponent.GetEntity()->GetID() == uSelectedEntityID );
	}

	nlohmann::json SerializeComponent( const Entity* pEntity ) const override
	{
		nlohmann::json oJsonContent;

		if( ComponentManager::GetComponentsFactory().find( GetComponentName() ) == ComponentManager::GetComponentsFactory().end() )
			return oJsonContent;

		for( const ComponentType& oComponent : m_aComponents )
		{
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
		for( ComponentType& oComponent : m_aComponents )
		{
			if( oComponent.m_pEntity == pEntity )
			{
				for( const auto& it : s_mProperties )
					it.second->Deserialize( oJsonContent, &oComponent );

				break;
			}
		}
	}

	ComponentType* GetComponent( const Entity* pEntity )
	{
		for( ComponentType& oComponent : m_aComponents )
		{
			if( oComponent.m_pEntity == pEntity )
				return &oComponent;
		}

		return nullptr;
	}

	uint GetCount() const
	{
		return m_aComponents.Count();
	}

	static const std::string& GetComponentName()
	{
		static const std::string sComponentName = []() {
			const std::string sName = std::string( typeid( ComponentType ).name() );
			return sName.substr( sName.find( " " ) + 1 );
		}();
		
		return sComponentName;
	}

	Array< ComponentType >		m_aComponents;

	using ComponentProperties = std::unordered_map< std::type_index, PropertiesHolderBase* >;
	static ComponentProperties	s_mProperties;
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
	friend struct ComponentsHolder;

	friend class Scene;

	ComponentManager();
	~ComponentManager();

	// TODO #eric maybe we should always return component handles ?
	template < typename ComponentType >
	ComponentType& CreateComponent( Entity* oEntity )
	{
		ComponentsHolderBase*& pComponentsHolderBase = m_mComponentsHolders[ typeid( ComponentType ) ];
		if( pComponentsHolderBase == nullptr )
			pComponentsHolderBase = new ComponentsHolder< ComponentType >;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( pComponentsHolderBase );
		pComponentsHolder->m_aComponents.PushBack( ComponentType( oEntity ) );

		return pComponentsHolder->m_aComponents.Back();
	}

	template < typename ComponentType >
	ComponentType* GetComponent( const Entity* pEntity )
	{
		auto it = m_mComponentsHolders.find( typeid( ComponentType ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return nullptr;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( it->second );
		return pComponentsHolder->GetComponent( pEntity );
	}

	template < typename ComponentType >
	ArrayView< ComponentType > GetComponents()
	{
		auto it = m_mComponentsHolders.find( typeid( ComponentType ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return ArrayView< ComponentType >();

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( it->second );
		return pComponentsHolder->m_aComponents;
	}

	void					InitializeComponents();
	bool					AreComponentsInitialized() const;
	void					StartComponents();
	void					StopComponents();
	void					UpdateComponents( const float fDeltaTime );

	Array< nlohmann::json >	SerializeComponents( const Entity* pEntity );
	void					DeserializeComponents( const nlohmann::json& oJsonContent, Entity* pEntity );

	void					DisplayGizmos( const uint64 uSelectedEntityID );

	template < typename ComponentType >
	static void RegisterComponent()
	{
		GetComponentsFactory()[ ComponentsHolder< ComponentType >::GetComponentName() ] = []( Entity* pEntity ) {
			g_pComponentManager->CreateComponent< ComponentType >( pEntity );
		};
	}

private:
	template < typename ComponentType >
	uint GetVersion()
	{
		auto it = m_mComponentsHolders.find( typeid( ComponentType ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return UINT_MAX;

		return it->second->m_uVersion;
	}

	static std::unordered_map< std::string, std::function< void( Entity* ) > >& GetComponentsFactory()
	{
		static std::unordered_map< std::string, std::function< void( Entity* ) > > s_mComponentsFactory;
		return s_mComponentsFactory;
	}

	std::unordered_map< std::type_index, ComponentsHolderBase* > m_mComponentsHolders;
};