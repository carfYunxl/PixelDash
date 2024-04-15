#ifndef COMPONENTS_H_
#define COMPONENTS_H_

#include <string>
#include <d2d1.h>
//#include "glm/glm.hpp"
//#include "glm/gtc/matrix_transform.hpp"
//
//#define GLM_ENABLE_EXPERIMENTAL
//#include "glm/gtx/quaternion.hpp"\

#include "HF_Scene.hpp"

struct PointComponent
{
	CD2DPointF Point;

	PointComponent() = default;
	PointComponent(const CD2DPointF& point) 
		: Point(point)
	{}
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
	TransformComponent() = default;
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
	T& AddComponent(HF_Scene* m_Scene, entt::entity handle, Args&&... args)
	{
		return m_Scene->Registry().emplace<T>(handle, std::forward<Args>(args)...);
	}

	template<typename T>
	T& GetComponent(HF_Scene* m_Scene, entt::entity handle)
	{
		return m_Scene->Registry().get<T>(handle);
	}

	template<typename T>
	bool HasComponent(HF_Scene* m_Scene, entt::entity handle)
	{
		return m_Scene->Registry().all_of<T>(handle);
	}

	template<typename T>
	void RemoveComponent(HF_Scene* m_Scene, entt::entity handle)
	{
		m_Scene->Registry().remove<T>(handle);
	}
}
#endif //COMPONENTS_H_