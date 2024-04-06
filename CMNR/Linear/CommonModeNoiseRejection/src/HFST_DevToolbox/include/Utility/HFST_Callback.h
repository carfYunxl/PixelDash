
#ifndef __HFST_CALLBACK_H__
#define __HFST_CALLBACK_H__

#include "HFST_CommDef.h"

_HFST_BEGIN
/**
 * @brief Callback class to simplify callback operations 
 * -# Inherit from HFST_Callback and assign FunctionT
 * -# Call trigger function by param 
 * @code
 *  using CallbackFunc = void (*)( int code, const char * result );
 *  class CallbackDemo : public HFST_Callback< CallbackFunc > {
 *  };
 *
 *  CallbackDemo cb;
 *  cb.trigger( 1, nullptr );
 * @endcode
 */
template< typename FunctionT >
class EXPORT_CLASS HFST_Callback {
public:
	/**
	 * @brief Set callback function 
	 * @param	[in]	func 	Callback function define
	 */
	void SetCallback( FunctionT func ) {
		m_func = func;
	}

	/**
	 * @brief Trigger callback function 
	 * @param	[in]	args 	Callback function param
	 */
	template< typename ... Args >
	void trigger( Args&& ... args ) {
		m_func( std::forward<Args>(args)... );
	}

private:
	FunctionT m_func;
};

_HFST_END
#endif // __HFST_CALLBACK_H__