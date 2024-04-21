#ifndef COMPONENTS_H_
#define COMPONENTS_H_

#include <string>
#include <d2d1.h>
//#include "glm/glm.hpp"
//#include "glm/gtc/matrix_transform.hpp"
//
//#define GLM_ENABLE_EXPERIMENTAL
//#include "glm/gtx/quaternion.hpp"\

#include "Pixel_Scene.hpp"

struct PointComponent
{
	CD2DPointF* m_PtArray{ nullptr };
	int m_Index{0};

	float m_TpWidth = 1000.0f;
	float m_TpHeight = 600.0f;

	int m_CntX = 40;
	int m_CntY = 25;

	PointComponent()
	{
		m_PtArray = new CD2DPointF[m_CntX * m_CntY];

		UpdatePoint();
	};
	PointComponent(int index, int x, int y, float width, float height) 
		: m_Index(index), m_TpWidth(width), m_TpHeight(height), m_CntX(x), m_CntY(y)
	{
		m_PtArray = new CD2DPointF[x*y];

		UpdatePoint();
	}
	~PointComponent() {
		delete[] m_PtArray;
	}

	void UpdatePoint()
	{
		if (m_PtArray)
		{
			delete[] m_PtArray;

			m_PtArray = new CD2DPointF[m_CntX * m_CntY];
		}

		int nPointIdx = 0;

		float nGapX = m_TpWidth / (float(m_CntX - 1));
		float nGapY = m_TpHeight / (float(m_CntY - 1));

		for (int i = 0; i < m_CntX; ++i)
		{
			for (int j = 0; j < m_CntY; ++j)
			{
				float x = float(i) * nGapX + 100.0f;
				float y = float(j) * nGapY + 100.0f;

				m_PtArray[nPointIdx] = CD2DPointF{ x, y };

				nPointIdx++;
			}
		}
	}
};

struct LineComponent
{
	CD2DPointF m_Start	{300.0f, 300.0f};
	CD2DPointF m_End	{400.0f, 400.0f};

	LineComponent() = default;
	LineComponent(const CD2DPointF& start, const CD2DPointF& end)
		: m_Start(start), m_End(end) {
	
	}
};

struct RectangleComponent
{
	CD2DPointF m_LeftTop{ 100.0f, 100.0f };
	CD2DPointF m_RightBottom{ 400.0f, 400.0f };

	RectangleComponent() = default;
	RectangleComponent(const CD2DPointF& left_top, const CD2DPointF& right_bottom)
		: m_LeftTop(left_top), m_RightBottom(right_bottom) {

	}
};

struct TransformComponent
{
	CD2DSizeF	m_Trans { 0.0f, 0.0f };
	CD2DSizeF	m_Scale { 1.0f, 1.0f };
	CD2DPointF	m_Center{ 0.0f, 0.0f };
	float		m_Rotate{ 0.0f };

	TransformComponent() = default;
	TransformComponent(const CD2DSizeF& trans, const CD2DSizeF& scale, float angle, const CD2DPointF& center)
		: m_Trans(trans), m_Scale(scale), m_Rotate(angle), m_Center(center) {}
};

struct BorderColorComponent
{
	D2D1::ColorF m_BorderColor{ D2D1::ColorF::Black };

	BorderColorComponent() = default;
	BorderColorComponent(const D2D1::ColorF& color)
		: m_BorderColor(color) {}
};

struct BorderWidthComponent
{
	float m_BorderWidth{ 1.0f };

	BorderWidthComponent() = default;
	BorderWidthComponent(float width)
		: m_BorderWidth(width) {}
};

struct OpacityComponent
{
	float m_Opacity{ 1.0f };

	OpacityComponent() = default;
	OpacityComponent(float opacity)
		: m_Opacity(opacity) {}
};

struct FillColorComponent
{
	D2D1::ColorF m_FillColor{ D2D1::ColorF::Black };

	FillColorComponent() = default;
	FillColorComponent(const D2D1::ColorF& color)
		: m_FillColor(color) {}
};

namespace HFST
{
	template<typename T, typename... Args>
	T& AddComponent(Pixel_Scene* m_Scene, entt::entity handle, Args&&... args)
	{
		return m_Scene->Registry().emplace<T>(handle, std::forward<Args>(args)...);
	}

	template<typename T>
	T& GetComponent(Pixel_Scene* m_Scene, entt::entity handle)
	{
		return m_Scene->Registry().get<T>(handle);
	}

	template<typename T>
	bool HasComponent(Pixel_Scene* m_Scene, entt::entity handle)
	{
		return m_Scene->Registry().all_of<T>(handle);
	}

	template<typename T>
	void RemoveComponent(Pixel_Scene* m_Scene, entt::entity handle)
	{
		m_Scene->Registry().remove<T>(handle);
	}
}
#endif //COMPONENTS_H_