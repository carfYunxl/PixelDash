#ifndef __Pixel_SCENE_HPP__
#define __Pixel_SCENE_HPP__

#include "entt.hpp"
#include "Pixel_Entity.hpp"
#include "Pixel_Header.h"
#include <string>
#include <memory>
#include "RendererD2D.hpp"
class CWnd;

class Pixel_Scene
{
public:
	Pixel_Scene(CWnd& pWnd);
	~Pixel_Scene() {};

	Pixel_Entity CreateEntity(DRAW_TYPE type);

	void OnDraw();

	void DestroyEntity(Pixel_Entity entity,DRAW_TYPE type);

	void FillSceneBackground(const D2D1::ColorF& bgColor);
	void DrawTpArea(const CD2DRectF& rect, D2D1::ColorF bgColor, D2D1::ColorF gridColor, float gap) {
		CRect recClient;
		m_pWnd.GetClientRect(&recClient);

		m_renderer->DrawTpArea(rect, bgColor, gridColor, gap);
	}

	entt::registry& Registry() { return m_Registry; }

	CWnd& Wnd() { return m_pWnd; }

private:
	CWnd& m_pWnd;
	entt::registry						m_Registry;
	std::unique_ptr<Pixel::RendererD2D>	m_renderer;

	friend class Pixel_Entity;
};

#endif //__Pixel_SCENE_HPP__