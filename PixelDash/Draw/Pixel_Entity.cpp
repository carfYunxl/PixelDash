#include "pch.h"
#include "Pixel_Entity.hpp"
#include "Pixel_Scene.hpp"

Pixel_Entity::Pixel_Entity(entt::entity handle, Pixel_Scene* scene)
	: m_EntityHandle(handle), m_Scene(scene)
{
}