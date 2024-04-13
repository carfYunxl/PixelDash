#ifndef __HF_SCENE_HPP__
#define __HF_SCENE_HPP__

#include "entt.hpp"
#include "HF_Entity.hpp"
#include "HF_Header.h"
#include <string>
#include <memory>
#include "HFST_RendererD2D.hpp"
class CWnd;

class HF_Scene
{
public:
	HF_Scene(CWnd& pWnd);
	~HF_Scene() {};

	HF_Entity CreateEntity(DRAW_TYPE type);

	void OnDraw();

	void DestroyEntity(HF_Entity entity);

	void FillSceneBackground(const D2D1::ColorF& bgColor);

	entt::registry& Registry() { return m_Registry; }

	CWnd& Wnd() { return m_pWnd; }

private:
	CWnd& m_pWnd;
	entt::registry						m_Registry;
	std::unique_ptr<HFST::RendererD2D>	m_renderer;

	friend class HF_Entity;
};

#endif //__HF_SCENE_HPP__