#ifndef __Pixel_ENTITY_HPP__
#define __Pixel_ENTITY_HPP__
#include "entt.hpp"

class Pixel_Scene;

class Pixel_Entity
{
public:
	Pixel_Entity() = default;
	Pixel_Entity(entt::entity handle, Pixel_Scene* scene);
	Pixel_Entity(const Pixel_Entity& other) = default;

	/*template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
	}

	template<typename T>
	T& GetComponent()
	{
		return m_Scene->m_Registry.get<T>(m_EntityHandle);
	}

	template<typename T>
	bool HasComponent()
	{
		return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
	}

	template<typename T>
	void RemoveComponent()
	{
		m_Scene->m_Registry.remove<T>(m_EntityHandle);
	}*/

	operator bool() const { return m_EntityHandle != entt::null; }

	uint32_t toInt() const { return static_cast<uint32_t>(m_EntityHandle); }

	bool operator==(const Pixel_Entity& other) {
		return (*this).m_EntityHandle == other.m_EntityHandle && (*this).m_Scene == other.m_Scene;
	}

	bool operator!=(const Pixel_Entity& other) {
		return !operator==(other);
	}

	entt::entity GetHandleID() const { return m_EntityHandle; }

	bool isValid() const { return m_EntityHandle != entt::null; }

	Pixel_Scene* GetScene() const { return m_Scene; }
private:
	entt::entity	m_EntityHandle{ entt::null };
	Pixel_Scene*		m_Scene{ nullptr };
};


#endif //__Pixel_ENTITY_HPP__