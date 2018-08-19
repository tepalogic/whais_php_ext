#ifndef _WHAIS_PARAMS_H
#define _WHAIS_PARAMS_H

#include "whais_connector.h"

namespace whais 
{

zval* fetch_db_result(WH_CONNECTION hnd, const bool doZvalConv, zval& result);
long map_php_to_whais_type(zval& param, long typeHint);

} /* namespace whais */

#endif /* _WHAIS_PARAMS_H */
