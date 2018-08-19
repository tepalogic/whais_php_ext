
#ifndef PHP_WHAIS_H
#define PHP_WHAIS_H 1

#define PHP_WHAIS_CLIENT_VERSION "1.0"
#define PHP_WHAIS_CLIENT_EXTNAME "whais"


typedef struct _whais_connection {
  void* pDbConnection;
} whais_connection;


#define PHP_WHAIS_DB_CONN_RES_NAME "WHAIS DB Connection"


PHP_MINIT_FUNCTION(whais);
PHP_MSHUTDOWN_FUNCTION(whais);
PHP_RINIT_FUNCTION(whais);

PHP_FUNCTION(whais_connect);
PHP_FUNCTION(whais_disconnect);
PHP_FUNCTION(whais_is_alive);
PHP_FUNCTION(whais_greet);
PHP_FUNCTION(whais_push_parameter);
PHP_FUNCTION(whais_call);
PHP_FUNCTION(whais_clear_result);
PHP_FUNCTION(whais_describe_result);
PHP_FUNCTION(whais_push_str);
PHP_FUNCTION(whais_push_int);
PHP_FUNCTION(whais_push_float);
PHP_FUNCTION(whais_push_bool);
PHP_FUNCTION(whais_push);

extern zend_module_entry whais_module_entry;
#define phpext_whais_ptr &whais_module_entry

#endif

