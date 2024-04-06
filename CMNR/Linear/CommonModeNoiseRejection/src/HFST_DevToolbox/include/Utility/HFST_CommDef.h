
#ifndef __HFST_COMMDEF_H__
#define __HFST_COMMDEF_H__

#include <string>
#include <vector>
#include <thread>
#include <Windows.h>

#ifdef HFSTDEVTOOLBOX_EXPORTS
#define EXPORT_CLASS _declspec(dllexport)
#else
#define EXPORT_CLASS _declspec(dllimport)
#endif

#define _HFST_BEGIN namespace HFST_DevToolbox {
#define _HFST_END   }

#endif // __HFST_COMMDEF_H__
