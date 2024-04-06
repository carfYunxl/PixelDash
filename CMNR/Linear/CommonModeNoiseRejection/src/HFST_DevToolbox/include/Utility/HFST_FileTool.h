
#ifndef __HFST_FILETOOL_H__
#define __HFST_FILETOOL_H__

#include "HFST_CommDef.h"

_HFST_BEGIN

/**
 * Read text data into data
 * @param	[in]	fileName	The absolute file path to be read
 * @param	[out]	data		Save read data buffer
 * @return	true/false
 */
bool HFST_ReadTextFile( const char * fileName, std::string & data );

/**
 * Read binary data into 'data' and return data size
 * @param	[in]		fileName	The absolute file path to be read
 * @param	[in,out]	data		Save read data buffer, user alloc, can be nullptr
 * @param	[in,out]	size		Return read data size
 * @return	true/false
 * @note	if param data is nullptr, it will return file size, user can alloc memory use return size
 *			and call this function again to read file data
 */
bool HFST_ReadBinFile( const char * fileName, unsigned char * data, unsigned int & size );

/**
 * Write text file
 * @param	[in]	fileName	The absolute file path to be read
 * @param	[in]	data		The text file data
 * @param	[in]	size		The file data size
 * @return	true/false
 */
bool HFST_WriteTextFile( const char * fileName, char * data, unsigned int size );

/**
 * Write binary file
 * @param	[in]	fileName	The absolute file path to be read
 * @param	[in]	data		The binary file data
 * @param	[in]	size		The binary file data size
 * @return	true/false
 */
bool HFST_WriteBinFile( const char * fileName, unsigned char * data, unsigned int size );

/**
 * Format variable parameters into string
 * @param	[in,out]	str		The base string to be format
 * @param	[in]		fmt		Variable parameters format
 * @code
 *  std::string text = "";
 *  HFST_Format( text, "1 + 1 = %d", 1 + 1 );	// text =>  1 + 1 = 2
 * @endcode
 */
void HFST_AppendFormat( std::string & str, const char * fmt, ... );

/**
 * Get module version
 * @param	[in]	strModule	The module to get version
 * @return	If success, file version will return like x.x.x.x,
 *			else return empty
 */
std::string HFST_GetModuleVersion( const std::string & strModule );

_HFST_END
#endif // __HFST_FILETOOL_H__
