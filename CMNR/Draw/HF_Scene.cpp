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
		D2D1::ColorF border_color{ D2D1::ColorF::Black };
		D2D1::ColorF fill_color{ D2D1::ColorF::Black };
		float border_width{ 1.0f };
		float opacity{ 1.0f };

		if (HFST::HasComponent<BorderColorComponent>(this, ent))
		{
			auto& BorderColor = HFST::GetComponent<BorderColorComponent>(this, ent);
			border_color = BorderColor.m_BorderColor;
		}

		if (HFST::HasComponent<FillColorComponent>(this, ent))
		{
			auto& FillColor = HFST::GetComponent<FillColorComponent>(this, ent);
			fill_color = FillColor.m_FillColor;
		}

		if (HFST::HasComponent<BorderWidthComponent>(this, ent))
		{
			auto& BorderWidth = HFST::GetComponent<BorderWidthComponent>(this, ent);
			border_width = BorderWidth.m_BorderWidth;
		}

		if (HFST::HasComponent<OpacityComponent>(this, ent))
		{
			auto& Opacity = HFST::GetComponent<OpacityComponent>(this, ent);
			opacity = Opacity.m_Opacity;
		}

		if ( HFST::HasComponent<LineComponent>(this, ent) )
		{
			auto& line = HFST::GetComponent<LineComponent>(this, ent);
			m_renderer->DrawLine(line.m_Start, line.m_End, border_color, opacity, border_width);
		} 
		else if ( HFST::HasComponent<RectangleComponent>(this, ent) )
		{
			auto& rect = HFST::GetComponent<RectangleComponent>(this, ent);
			m_renderer->DrawRect(CRect(rect.m_LeftTop, rect.m_RightBottom), border_color, border_width);
		}
	}
}

void HF_Scene::DestroyEntity(HF_Entity entity, DRAW_TYPE type)
{
	switch (type)
	{
		case DRAW_TYPE::LINE:
		{
			// ÒÆ³ý Component
			if ( HFST::HasComponent<LineComponent>(this, entity.GetHandleID()) )
			{
				HFST::RemoveComponent<LineComponent>(this, entity.GetHandleID());
			}

			// ÒÆ³ýUI

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

	if ( HFST::HasComponent<BorderColorComponent>(this, entity.GetHandleID()) )
	{
		HFST::RemoveComponent<BorderColorComponent>(this, entity.GetHandleID());
	}

	if (HFST::HasComponent<BorderWidthComponent>(this, entity.GetHandleID()))
	{
		HFST::RemoveComponent<BorderWidthComponent>(this, entity.GetHandleID());
	}

	m_Registry.destroy(entity.GetHandleID());
}

void HF_Scene::FillSceneBackground(const D2D1::ColorF& bgColor)
{
	m_renderer->DrawBgColor(bgColor);
}
