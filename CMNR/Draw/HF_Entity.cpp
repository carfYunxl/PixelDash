#include "pch.h"
#include "HF_Entity.hpp"
#include "HF_Scene.hpp"

HF_Entity::HF_Entity(entt::entity handle, HF_Scene* scene)
	: m_EntityHandle(handle), m_Scene(scene)
{
}