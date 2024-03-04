
#include "CommDef.h"

bool ScanI2C_Addr()
{
	for ( int idx = 1; idx < 0x80; ++idx ) {
		SetI2CAddr( idx, 2, false );

		unsigned char tmp_data = 0xFF;
		if ( ReadI2CReg( &tmp_data, 0x01, 1 ) > 0 )
			return true;
	}

	return false;
}