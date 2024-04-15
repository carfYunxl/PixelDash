#ifndef __HF_HEADER_HPP__
#define __HF_HEADER_HPP__

enum class LogType
{
	INFO = 0,
	WARN,
	ERR
};

enum class DRAW_TYPE
{
	NONE = -1,
	LINE,
	RECTANGLE,
	TRIANGLE,
	CIRCLE,
	ELLIPSE,
	MI,
	KOU,
	HUI,
	LUO,
	X
};

enum class LINE_GROUP
{
	POSITION = 0,
	STYLE,
	BORDER_WIDTH,
	LINE_STYLE,
	BORDER_COLOR,
	FILL_color
};

enum class LINE
{
	START_X = 10,
	START_Y,
	END_X,
	END_Y,
};

enum class RECTANGLE
{
	LEFT = 20,
	TOP,
	RIGHT,
	BOTTOM
};

enum class NORMAL_PROPERTY
{
	BORDER_COLOR = 30,
	BORDER_WIDTH,
	FILL_COLOR,
	OPACITY
};

#endif //__HF_HEADER_HPP__
