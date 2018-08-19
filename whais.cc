
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_whais.h"
#include "zend_list.h"

#include "whais_connector.h"
#include "whais_params.h"

using namespace whais;

static zend_function_entry whais_functions[] = {
    PHP_FE(whais_connect, NULL)
    PHP_FE(whais_disconnect, NULL)
    PHP_FE(whais_is_alive, NULL)
    PHP_FE(whais_greet, NULL)
    PHP_FE(whais_push_parameter, NULL)
    PHP_FE(whais_call, NULL)
    PHP_FE(whais_clear_result, NULL)
    PHP_FE(whais_describe_result, NULL)
    PHP_FE(whais_push_str, NULL)
    PHP_FE(whais_push_int, NULL)
    PHP_FE(whais_push_float, NULL)
    PHP_FE(whais_push_bool, NULL)
    PHP_FE(whais_push, NULL)

    {NULL, NULL, NULL}
};

zend_module_entry whais_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_WHAIS_CLIENT_EXTNAME,
    whais_functions,
    PHP_MINIT(whais),
    PHP_MSHUTDOWN(whais),
    PHP_RINIT(whais),
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_WHAIS_CLIENT_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_WHAIS
extern "C" { 
	ZEND_GET_MODULE(whais)
}
#endif


int le_whais_connection = 0;


static void whais_connection_dtr(zend_resource *rsrc)
{
    auto* connection = reinterpret_cast<whais_connection*>(rsrc->ptr);
    auto  hnd = reinterpret_cast<WH_CONNECTION>(connection->pDbConnection);

    connection->pDbConnection = nullptr;
    efree(connection);
    if (hnd != nullptr) {
	    WClose(hnd);
    }
}

PHP_FUNCTION(whais_connect)
{
    zval *zhost, *zport, *zdb, *zpswd;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_DC, "zzzz",
			      &zhost, &zport, &zdb, &zpswd) == FAILURE) 
    {
	RETURN_NULL();
    }

    if (Z_TYPE_P(zhost) != IS_STRING) {
	    convert_to_string(zhost);
    }

    if (Z_TYPE_P(zport) != IS_STRING) {
	    convert_to_string(zport);
    }


    if (Z_TYPE_P(zdb) != IS_STRING) {
	    convert_to_string(zdb);
    }


    if (Z_TYPE_P(zpswd) != IS_STRING) {
	    convert_to_string(zpswd);
    }

    char* const host = Z_STRVAL_P(zhost);
    char* const port = Z_STRVAL_P(zport);
    char* const database = Z_STRVAL_P(zdb);
    char* const passwd = Z_STRVAL_P(zpswd);

    WH_CONNECTION hnd;
    if (WConnect(host, port, database, passwd, 1, 1000, &hnd) != WCS_OK) {
	    RETURN_NULL();
    }

    whais_connection *const result = (whais_connection*)ecalloc(1, sizeof(*result));
    if (result == NULL) {
	    RETURN_NULL();
    }

    result->pDbConnection = hnd;
    RETURN_RES(zend_register_resource(result, le_whais_connection));
}

PHP_FUNCTION(whais_disconnect)
{
    zval* zconn;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zconn) == FAILURE) 
    {
	RETURN_NULL();
    }

    zend_list_delete(Z_RES_P(zconn));
    RETURN_TRUE;
}


PHP_FUNCTION(whais_is_alive)
{
    zval* zconn;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zconn) == FAILURE) {
	RETURN_NULL();
    }

    auto* db_connection = (whais_connection*)zend_fetch_resource(Z_RES_P(zconn), PHP_WHAIS_DB_CONN_RES_NAME, le_whais_connection);
    if (db_connection == nullptr) {
	    RETURN_FALSE;
    }

    if (WPingServer(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection)) != WCS_OK) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_FUNCTION(whais_greet)
{
    zval* zconn;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zconn) == FAILURE) {
	RETURN_NULL();
    }

    auto* db_connection = (whais_connection*)zend_fetch_resource(Z_RES_P(zconn), PHP_WHAIS_DB_CONN_RES_NAME, le_whais_connection);
    if (db_connection == nullptr) {
	    RETURN_FALSE;
    }

    const char* greetResponse;
    if (WGreetServer(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), &greetResponse) != WCS_OK) {
        RETURN_FALSE;
    }

    RETURN_STR(zend_string_init(greetResponse, strlen(greetResponse), 0));
}


PHP_FUNCTION(whais_call)
{
    zval*  zconn;
    char*  procedureName;
    size_t procedureNameLen;
    zend_bool doZvalConv = true;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|b", &zconn,
			    &procedureName, &procedureNameLen, doZvalConv) == FAILURE) {
	RETURN_NULL();
    }

    auto* db_connection = (whais_connection*)zend_fetch_resource(Z_RES_P(zconn), PHP_WHAIS_DB_CONN_RES_NAME, le_whais_connection);
    if (db_connection == nullptr) {
	    RETURN_FALSE;
    }

    if (WExecuteProcedure(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), procedureName) != WCS_OK) {
        RETURN_FALSE;
    }

    zval callResult;
    auto* const fetchResult = whais::fetch_db_result(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), doZvalConv, callResult);
    if (fetchResult == nullptr) {
	    RETURN_NULL();
    }

    RETURN_ZVAL(fetchResult, 0, 0);
}

PHP_FUNCTION(whais_clear_result)
{
    zval* zconn;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zconn) == FAILURE) {
	RETURN_NULL();
    }

    auto* db_connection = (whais_connection*)zend_fetch_resource(Z_RES_P(zconn), PHP_WHAIS_DB_CONN_RES_NAME, le_whais_connection);
    if (db_connection == nullptr) {
	RETURN_NULL();
    }

    if (WPopValues(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WPOP_ALL) != WCS_OK) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(whais_describe_result)
{
    zval* zconn;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zconn) == FAILURE) {
	RETURN_NULL();
    }

    auto* db_connection = (whais_connection*)zend_fetch_resource(Z_RES_P(zconn), PHP_WHAIS_DB_CONN_RES_NAME, le_whais_connection);
    if (db_connection == nullptr) {
	RETURN_NULL();
    }

    uint_t stackTopType;
    if (WDescribeStackTop(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), &stackTopType) != WCS_OK) {
        RETURN_NULL();
    }

    RETURN_LONG(stackTopType);
}


PHP_FUNCTION(whais_push_str)
{
    zval*  zconn;
    char*  strParam;
    size_t strParamLen;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &zconn,
			    &strParam, &strParamLen) == FAILURE) {
	RETURN_NULL();
    }

    auto* db_connection = (whais_connection*)zend_fetch_resource(Z_RES_P(zconn), PHP_WHAIS_DB_CONN_RES_NAME, le_whais_connection);
    if (db_connection == nullptr) {
	    RETURN_FALSE;
    }

    if (WPushValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WHC_TYPE_TEXT, 0, nullptr) != WCS_OK) {
        RETURN_FALSE;
    }

    if (WUpdateValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WHC_TYPE_TEXT,
			    WIGNORE_FIELD, WIGNORE_ROW, WIGNORE_OFF, 0, strParam) != WCS_OK) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(whais_push_int)
{
    zval* zconn;
    long param;
    char txtParam[64];

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &zconn,
			    &param) == FAILURE) {
	RETURN_NULL();
    }

    auto* db_connection = (whais_connection*)zend_fetch_resource(Z_RES_P(zconn), PHP_WHAIS_DB_CONN_RES_NAME, le_whais_connection);
    if (db_connection == nullptr) {
	    RETURN_FALSE;
    }

    if (WPushValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WHC_TYPE_INT64, 0, nullptr) != WCS_OK) {
        RETURN_FALSE;
    }

    snprintf(txtParam, sizeof txtParam, "%ld", param);
    if (WUpdateValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WHC_TYPE_INT64,
			    WIGNORE_FIELD, WIGNORE_ROW, WIGNORE_OFF, WIGNORE_OFF, txtParam) != WCS_OK) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}


PHP_FUNCTION(whais_push_float)
{
    zval* zconn;
    double param;
    char txtParam[64];

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rd", &zconn,
			    &param) == FAILURE) {
	RETURN_NULL();
    }

    auto* db_connection = (whais_connection*)zend_fetch_resource(Z_RES_P(zconn), PHP_WHAIS_DB_CONN_RES_NAME, le_whais_connection);
    if (db_connection == nullptr) {
	    RETURN_FALSE;
    }

    if (WPushValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WHC_TYPE_RICHREAL, 0, nullptr) != WCS_OK) {
        RETURN_FALSE;
    }

    snprintf(txtParam, sizeof txtParam, "%f", param);
    if (WUpdateValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WHC_TYPE_RICHREAL,
			    WIGNORE_FIELD, WIGNORE_ROW, WIGNORE_OFF, WIGNORE_OFF, txtParam) != WCS_OK) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}


PHP_FUNCTION(whais_push_bool)
{
    zval* zconn;
    zend_bool param;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rb", &zconn,
			    &param) == FAILURE) {
	RETURN_NULL();
    }

    auto* db_connection = (whais_connection*)zend_fetch_resource(Z_RES_P(zconn), PHP_WHAIS_DB_CONN_RES_NAME, le_whais_connection);
    if (db_connection == nullptr) {
	    RETURN_FALSE;
    }

    if (WPushValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WHC_TYPE_BOOL, 0, nullptr) != WCS_OK) {
        RETURN_FALSE;
    }

    if (WUpdateValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WHC_TYPE_BOOL,
			    WIGNORE_FIELD, WIGNORE_ROW, WIGNORE_OFF, 
			    WIGNORE_OFF, (param ? "1" : "0")) != WCS_OK) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(whais_push)
{
    zval* zconn;
    zval* val;
    long type = WHC_TYPE_NOTSET;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz|l", &zconn, 
			    &val, &type) == FAILURE) {
	RETURN_NULL();
    }

    auto* db_connection = (whais_connection*)zend_fetch_resource(Z_RES_P(zconn), PHP_WHAIS_DB_CONN_RES_NAME, le_whais_connection);
    if (db_connection == nullptr) {
	    RETURN_FALSE;
    }

    type = map_php_to_whais_type(*val, type);
    if (type == WHC_TYPE_NOTSET) {
	    RETURN_FALSE;
    }

    if ((type & WHC_TYPE_ARRAY_MASK) != 0) {
	    if ((type & ~WHC_TYPE_ARRAY_MASK) == WHC_TYPE_TEXT) {
		    RETURN_FALSE;
	    }

	    if (WPushValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), type, 0, nullptr) != WCS_OK) {
		RETURN_FALSE;
	    }

	    type &= ~WHC_TYPE_ARRAY_MASK;
	    int i = 0;
	    while (zend_hash_index_exists(Z_ARRVAL_P(val), i)) {
		    zval *elem = zend_hash_index_find(Z_ARRVAL_P(val), i);

		    convert_to_string(elem);
    		    const char* value = Z_STRVAL_P(elem);
		    if (WUpdateValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), type,
					    WIGNORE_FIELD, WIGNORE_ROW, i, WIGNORE_OFF, value) != WCS_OK) {
			RETURN_FALSE;
		    }
		    ++i;
	    }
	    RETURN_TRUE;
    }

    convert_to_string(val);
    const char* value = Z_STRVAL_P(val);
    if (type != WHC_TYPE_TEXT) {
	    if (WPushValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), type, 0, nullptr) != WCS_OK) {
		RETURN_FALSE;
	    }

	    if (WUpdateValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), type,
				    WIGNORE_FIELD, WIGNORE_ROW, WIGNORE_OFF, WIGNORE_OFF, value) != WCS_OK) {
		RETURN_FALSE;
	    }
    } else {
	    if (WPushValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WHC_TYPE_TEXT, 0, nullptr) != WCS_OK) {
		RETURN_FALSE;
	    }

	    if (WUpdateValue(reinterpret_cast<WH_CONNECTION>(db_connection->pDbConnection), WHC_TYPE_TEXT,
				    WIGNORE_FIELD, WIGNORE_ROW, WIGNORE_OFF, 0, value) != WCS_OK) {
		RETURN_FALSE;
	    }
    }

    RETURN_TRUE;
}

PHP_RINIT_FUNCTION(whais)
{
    return SUCCESS;
}

PHP_MINIT_FUNCTION(whais)
{
    le_whais_connection = zend_register_list_destructors_ex(whais_connection_dtr, NULL, PHP_WHAIS_DB_CONN_RES_NAME, module_number);

    REGISTER_LONG_CONSTANT("WHAIS_BOOL", WHC_TYPE_BOOL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_CHAR", WHC_TYPE_CHAR, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_DATE", WHC_TYPE_DATE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_DATETIME", WHC_TYPE_DATE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_HIRESTIME", WHC_TYPE_DATE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_INT8", WHC_TYPE_INT8, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_INT16", WHC_TYPE_INT16, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_INT32", WHC_TYPE_INT32, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_INT64", WHC_TYPE_INT64, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("WHAIS_UINT8", WHC_TYPE_UINT8, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_UINT16", WHC_TYPE_UINT16, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_UINT32", WHC_TYPE_UINT32, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_UINT64", WHC_TYPE_UINT64, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("WHAIS_REAL", WHC_TYPE_REAL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_RICHREAL", WHC_TYPE_RICHREAL, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("WHAIS_TEXT", WHC_TYPE_TEXT, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("WHAIS_ARRAY", WHC_TYPE_ARRAY_MASK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_FIELD", WHC_TYPE_FIELD_MASK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("WHAIS_TABLE", WHC_TYPE_TABLE_MASK, CONST_CS | CONST_PERSISTENT);

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(whais)
{
    return SUCCESS;
}
