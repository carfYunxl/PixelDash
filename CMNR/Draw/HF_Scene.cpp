#include "pch.h"
#include "HF_Scene.hpp"
#include "HF_Components.hpp"
#include "HF_MainFrm.h"

HF_Scene::HF_Scene(CWnd& pWnd)
	: m_pWnd( pWnd )
	, m_renderer{ std::make_unique<HFST::RendererD2D>(m_pWnd)}
{

}

/**
 * @Discription Draw Entity here!!!
 */

HF_Entity HF_Scene::CreateEntity(DRAW_TYPE type)
{
	auto ent = m_Registry.create();

	switch (type)
	{
		case DRAW_TYPE::LINE:
		{
			if ( !HFST::HasComponent<LineComponent>(this, ent) )
			{
				HFST::AddComponent<LineComponent>(this, ent);
				((HF_MainFrame*)(theApp.m_pMainWnd))->m_wndProperty.AddLineProperty({ent, this});
			}
			break;
		}
		case DRAW_TYPE::RECTANGLE:
		{
			if (!HFST::HasComponent<RectangleComponent>(this, ent))
			{
				HFST::AddComponent<RectangleComponent>(this, ent);
				((HF_MainFrame*)(theApp.m_pMainWnd))->m_wndProperty.AddRectangleProperty({ ent, this });
			}
			break;
		}
	}

	return { ent, this };
}

void HF_Scene::OnDraw()
{
	auto view = m_Registry.view<entt::entity>();

	for (auto ent : view)
	{
		if ( HFST::HasComponent<LineComponent>(this, ent) )
		{
			auto& line = HFST::GetComponent<LineComponent>(this, ent);
			m_renderer->DrawLine(line.m_Start, line.m_End, D2D1::ColorF::Red);
		}

		if ( HFST::HasComponent<RectangleComponent>(this, ent) )
		{
			auto& rect = HFST::GetComponent<RectangleComponent>(this, ent);
			m_renderer->DrawRect(CRect(rect.m_LeftTop, rect.m_RightBottom), D2D1::ColorF::Red, 1.0f);
		}

		if ( HFST::HasComponent<PosComponent>(this, ent) )
		{
			// Draw PosComponent UI
		}

		if (HFST::HasComponent<LineColorComponent>(this, ent))
		{
			// Draw LineColorComponent UI
		}

		if (HFST::HasComponent<FillColorComponent>(this, ent))
		{
			// Draw FillColorComponent UI
		}

		if (HFST::HasComponent<TransformComponent>(this, ent))
		{
			// Draw TransformComponent UI
		}

		// TO DO
		// Other Component you want to add
	}
}

void HF_Scene::DestroyEntity(HF_Entity entity, DRAW_TYPE type)
{
	switch (type)
	{
		case DRAW_TYPE::LINE:
		{
			if ( HFST::HasComponent<LineComponent>(this, entity.GetHandleID()) )
			{
				HFST::RemoveComponent<LineComponent>(this, entity.GetHandleID());
			}
			break;
		}
		case DRAW_TYPE::RECTANGLE:
		{
			if ( HFST::HasComponent<RectangleComponent>(this, entity.GetHandleID()) )
			{
				HFST::RemoveComponent<RectangleComponent>(this, entity.GetHandleID());
			}
			break;
		}
	}
	m_Registry.destroy(entity.GetHandleID());
}

void HF_Scene::FillSceneBackground(const D2D1::ColorF& bgColor)
{
	m_renderer->DrawBgColor(bgColor);
}
