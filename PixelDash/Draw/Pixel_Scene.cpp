#include "pch.h"
#include "Pixel_Scene.hpp"
#include "Pixel_Components.hpp"
#include "Pixel_MainFrm.h"

Pixel_Scene::Pixel_Scene(CWnd& pWnd)
	: m_pWnd( pWnd )
	, m_renderer{ std::make_unique<Pixel::RendererD2D>(m_pWnd)}
{

}

/**
 * @Discription Draw Entity here!!!
 */

Pixel_Entity Pixel_Scene::CreateEntity(DRAW_TYPE type)
{
	auto ent = m_Registry.create();

	switch (type)
	{
		case DRAW_TYPE::LINE:
		{
			if ( !HFST::HasComponent<LineComponent>(this, ent) )
			{
				HFST::AddComponent<LineComponent>(this, ent);
				((Pixel_MainFrame*)(theApp.m_pMainWnd))->m_wndProperty.AddLineProperty({ent, this});
			}
			break;
		}
		case DRAW_TYPE::RECTANGLE:
		{
			if (!HFST::HasComponent<RectangleComponent>(this, ent))
			{
				HFST::AddComponent<RectangleComponent>(this, ent);
				((Pixel_MainFrame*)(theApp.m_pMainWnd))->m_wndProperty.AddRectangleProperty({ ent, this });
			}
			break;
		}
		case DRAW_TYPE::TEN:
		{
			if (!HFST::HasComponent<PointComponent>(this, ent))
			{
				HFST::AddComponent<PointComponent>(this, ent);
				((Pixel_MainFrame*)(theApp.m_pMainWnd))->m_wndProperty.AddPointProperty({ ent, this });
			}
			break;
		}
	}

	return { ent, this };
}

void Pixel_Scene::OnDraw()
{
	auto view = m_Registry.view<entt::entity>();

	D2D1::ColorF border_color{ D2D1::ColorF::Black };
	D2D1::ColorF fill_color{ D2D1::ColorF::Black };
	float border_width{ 1.0f };
	float opacity{ 1.0f };

	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Identity();

	for (auto ent : view)
	{
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

		if (HFST::HasComponent<TransformComponent>(this, ent))
		{
			auto& trans = HFST::GetComponent<TransformComponent>(this, ent);

			const auto translate = D2D1::Matrix3x2F::Translation(trans.m_Trans.width, trans.m_Trans.height);
			const auto scale = D2D1::Matrix3x2F::Scale(trans.m_Scale.width, trans.m_Scale.height, trans.m_Center);
			const auto rotate = D2D1::Matrix3x2F::Rotation(trans.m_Rotate, trans.m_Center);

			transform = rotate * scale * translate;
		}

		if (HFST::HasComponent<OpacityComponent>(this, ent))
		{
			auto& Opacity = HFST::GetComponent<OpacityComponent>(this, ent);
			opacity = Opacity.m_Opacity;
		}

		if ( HFST::HasComponent<LineComponent>(this, ent) )
		{
			auto& line = HFST::GetComponent<LineComponent>(this, ent);
			m_renderer->DrawLine(line.m_Start, line.m_End, transform, border_color, opacity, border_width);
		} 
		else if ( HFST::HasComponent<RectangleComponent>(this, ent) )
		{
			auto& rect = HFST::GetComponent<RectangleComponent>(this, ent);
			m_renderer->DrawRect(CRect(rect.m_LeftTop, rect.m_RightBottom), transform, border_color, border_width);
		}
		else if (HFST::HasComponent<PointComponent>(this, ent))
		{
			auto& pts = HFST::GetComponent<PointComponent>(this, ent);
			for (int i = 0;i < pts.m_CntX * pts.m_CntY; ++i)
			{
				m_renderer->DrawPoint( pts.m_PtArray[i], border_color, 5.0f);
			}
		}
	}
}

void Pixel_Scene::DestroyEntity(Pixel_Entity entity, DRAW_TYPE type)
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
		case DRAW_TYPE::TEN:
		{
			if (HFST::HasComponent<PointComponent>(this, entity.GetHandleID()))
			{
				HFST::RemoveComponent<PointComponent>(this, entity.GetHandleID());
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

void Pixel_Scene::FillSceneBackground(const D2D1::ColorF& bgColor)
{
	m_renderer->DrawBgColor(bgColor);
}
