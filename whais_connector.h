/******************************************************************************
WHAIS - An advanced database system
Copyright(C) 2008  Iulian Popa

Address: Str Olimp nr. 6
         Pantelimon Ilfov,
         Romania
Phone:   +40721939650
e-mail:  popaiulian@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef WHAIS_CONNECTOR_H_
#define WHAIS_CONNECTOR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONNECTOR_SHL
  #define CONNECTOR_SHL
#endif

/* Custom types defined to ease the usage of the Whais' connector API. */
typedef unsigned int uint_t;
typedef unsigned long long ullong_t;
typedef void*     WH_CONNECTION;
typedef ullong_t  WHT_ROW_INDEX;
typedef ullong_t  WHT_INDEX;

/* Constants describing the type of values or table fields. */
static const uint_t WHC_TYPE_BOOL        = 0x0001;
static const uint_t WHC_TYPE_CHAR        = 0x0002;
static const uint_t WHC_TYPE_DATE        = 0x0003;
static const uint_t WHC_TYPE_DATETIME    = 0x0004;
static const uint_t WHC_TYPE_HIRESTIME   = 0x0005;
static const uint_t WHC_TYPE_INT8        = 0x0006;
static const uint_t WHC_TYPE_INT16       = 0x0007;
static const uint_t WHC_TYPE_INT32       = 0x0008;
static const uint_t WHC_TYPE_INT64       = 0x0009;
static const uint_t WHC_TYPE_UINT8       = 0x000A;
static const uint_t WHC_TYPE_UINT16      = 0x000B;
static const uint_t WHC_TYPE_UINT32      = 0x000C;
static const uint_t WHC_TYPE_UINT64      = 0x000D;
static const uint_t WHC_TYPE_REAL        = 0x000E;
static const uint_t WHC_TYPE_RICHREAL    = 0x000F;
static const uint_t WHC_TYPE_TEXT        = 0x0010;
static const uint_t WHC_TYPE_NOTSET      = 0x0011;

/* Type modifiers to identify Whais's composite types. */
static const uint_t WHC_TYPE_ARRAY_MASK = 0x0100;
static const uint_t WHC_TYPE_FIELD_MASK = 0x0200;
static const uint_t WHC_TYPE_TABLE_MASK = 0x0400;

/* Constants returned by connector's API to communicate their results. */
static const uint_t WCS_OK                   = 0;
static const uint_t WCS_INVALID_ARGS         = 1;
static const uint_t WCS_OP_NOTSUPP           = 2;
static const uint_t WCS_OP_NOTPERMITED       = 3;
static const uint_t WCS_DROPPED              = 4;
static const uint_t WCS_PROTOCOL_NOTSUPP     = 5;
static const uint_t WCS_ENCTYPE_NOTSUPP      = 6;
static const uint_t WCS_UNEXPECTED_FRAME     = 7;
static const uint_t WCS_INVALID_FRAME        = 8;
static const uint_t WCS_COMM_OUT_OF_SYNC     = 9;
static const uint_t WCS_LARGE_ARGS           = 10;
static const uint_t WCS_LARGE_RESPONSE       = 11;
static const uint_t WCS_CONNECTION_TIMEOUT   = 12;
static const uint_t WCS_SERVER_BUSY          = 13;
static const uint_t WCS_INCOMPLETE_CMD       = 14;
static const uint_t WCS_INVALID_ARRAY_OFF    = 15;
static const uint_t WCS_INVALID_TEXT_OFF     = 16;
static const uint_t WCS_INVALID_ROW          = 17;
static const uint_t WCS_INVALID_FIELD        = 18;
static const uint_t WCS_TYPE_MISMATCH        = 19;
static const uint_t WCS_PROC_NOTFOUND        = 20;
static const uint_t WCS_PROC_RUNTIME_ERR     = 21;
static const uint_t WCS_GENERAL_ERR          = 0x0FFF;
static const uint_t WCS_OS_ERR_BASE          = 0x1000;

/* If a failure at the OS layer occurs, it will be signaled with the help of
 * 'WCS_OS_ERR_BASE'. Following macros encodes/decodes the error as it was
 * returned by the OS layer. That value is OS dependent. */
#define WENC_OS_ERROR(x)   ((x) + WCS_OS_ERR_BASE)
#define WDEC_OS_ERROR(x)   ((x) - WCS_OS_ERR_BASE)

/* Standard default parameters for connector's APIs. */
#define WIGNORE_FIELD   NULL
#define WIGNORE_ROW     (~0ull)
#define WIGNORE_OFF     (~0ull)
#define WPOP_ALL        (~0)

/* The interval of valid maximum communication frame sizes. */
#define MIN_FRAME_SIZE       512
#define MAX_FRAME_SIZE       65535
#define DEFAULT_FRAME_SIZE   MAX_FRAME_SIZE

/* Describes the field of a table. */
struct WField
{
  const char*   name;    /* Field name. */
  uint_t        type;    /* Field type. */
};


/* Connects to a remote sever.
 *
 * Called prior any other connector's API, it's used to setup the connection to
 * a remote Whais server. In case of success this function should return a
 * handle to identify this connection.
 *
 * @host                Specify the host name of the Whais server.
 * @port                Specify the port where to connect. This is an UTF-8
 *                      string so service names could be used too.
 * @database            Specify the name of the database to use.
 * @password            Specify to user password.
 * @userId              Set to 0 if this is an administrator user, anything
 *                      else for a regular user.
 * @maxFrameSize        Hint about the allowed communication maximum frame size.
 *                      It should be set to DEFAULT_FRAME_SIZE, or to any value
 *                      between MIN_FRAME_SIZE and MAX_FRAME_SIZE.
 * @outHnd              in case of a successful connection, this will hold the
 *                      connection handle to be used with the rest of the
 *                      function.
 *
 * @return              WCS_OK in case of success, other way it will return
 *                      the error's case corresponding code.
 * @return              WCS_OK in case of success, anything else other way.
 */
CONNECTOR_SHL uint_t
WConnect(const char* const      host,
         const char* const      port,
         const char* const      database,
         const char* const      password,
         const uint_t           userid,
         const uint16_t         maxFrameSize,
         WH_CONNECTION* const   outHnd);

/* Close a connection.
 *
 * Instructs the remote server to discard and data related to this connection.
 *
 * @hnd                 The connection handle.
 */
CONNECTOR_SHL void
WClose(WH_CONNECTION hnd);

/* Send a dummy command to server.
 *
 * This has several usages from keeping the connection alive or test the
 * connection healthiness.
 *
 * @hnd                 The connection handle.
 *
 * @return              WCS_OK in case of success, other way it will return the
 *                      error's case corresponding code.
 */
CONNECTOR_SHL uint_t
WPingServer(const WH_CONNECTION hnd);


/* Send a greeting command to server.
 *
 * This command is used to retrieve a text description of the database
 * server. The server description is optional and the format is of arbitrary
 * content.
 *
 * @hnd                 The connection handle.
 *
 * @return              WCS_OK in case of success, other way it will return the
 *                      error's case corresponding code.
 */
CONNECTOR_SHL uint_t
WGreetServer(const WH_CONNECTION   hnd,
             const char**          outAnswer);

/* Get the list of the global values.
 *
 * Used to initialize the fetching of the global values defined by the context
 * of the specified database.
 *
 * This function will fail if it's not called with a connection handler that
 * was successfully authenticated with the the administrator account.
 *
 * @hnd                 The connection handle.
 * @outCount            In case of success, this will hold the number of global
 *                      values defined be the context of the connected database.
 *                      It should be set to NULL for if this value is not
 *                      needed.
 *
 * @return              WCS_OK in case of success, other way it will return the
 *                      error's case corresponding code.
 */
CONNECTOR_SHL uint_t
WStartGlobalsList(const WH_CONNECTION   hnd,
                  uint_t* const         outCount);

/* Fetch the name of the next global value.
 *
 * This is used to retrieve the next global variable name defined in the
 * connected database. This function should be called after WStartGlobalsList
 * was used initialize the global list.
 * To retrieve all global values names one should call repeatedly WFetchGlobal.
 *
 * @hnd                 The connection handle.
 * @outpName            In case of success this will hold a pointer to the name
 *                      of the next global variable. The name is a null
 *                      terminated UTF-8 encoded string. If there are no
 *                      global values left it will be set to NULL.
 *
 * @return              WCS_OK in case of success, other way it will return
 *                      the error's case corresponding code.
 *
 * NOTE: The caller does not have the ownership of the memory pointed to
 *       by outpName. It should make a copy of its content prior calling any
 *       other API using the same connection handle.
 */
CONNECTOR_SHL uint_t
WFetchGlobal(const WH_CONNECTION    hnd,
             const char** const     outpName);

/* Get the list of the procedures.
 *
 * Initialize the fetching of the procedure defined by the context of the
 * specified database.
 *
 * This function will fail if it's not called with a connection handler that
 * was successfully authenticated with the the administrator account.
 *
 * @hnd                 The connection handle.
 * @outCount            In case of success, this will hold the number of
 *                      procedures defined by the context of the connected
 *                      database. It should be set to NULL if this value is not
 *                      needed.
 *
 * @return              WCS_OK in case of success, other way it will return the
 *                      error's case corresponding code.
 */
CONNECTOR_SHL uint_t
WStartProceduresList(const WH_CONNECTION   hnd,
                     uint_t* const         outCount);

/* Fetch the name of the next procedure.
 *
 * This is used to retrieve the next procedure name defined on the context of
 * the connected database. This function should be called after
 * WStartProceduresList was used initialize the procedures names list.
 * To retrieve all procedures names one should call repeatedly WFetchProcedure.
 *
 * This function will fail if it's not called with a connection handler that
 * was successfully authenticated with the the administrator account.
 *
 * @hnd                 The connection handle.
 * @outpName            In case of success this will hold a pointer to the name
 *                      of the next procedure. The name is a null terminated
 *                      UTF-8 encoded string. If there are no procedures left
 *                      it will be set to NULL.
 *
 * @return              WCS_OK in case of success, other way it will return
 *                      the error's case corresponding code.
 *
 * NOTE: The caller does not have the ownership of the memory pointed to
 *       by outpName. It should make a copy of its content prior calling any
 *       other API using the same connection handle.
 */
CONNECTOR_SHL uint_t
WFetchProcedure(const WH_CONNECTION   hnd,
                const char** const    outpName);

/* Get the parameters count of a procedure.
 *
 * This function will fail if it's not called with a connection handler that
 * was successfully authenticated with the the administrator account.
 *
 * @hnd                 The connection handle.
 * @name                Name of the procedure.
 * @outCount            In case of success will hold the number of procedure
 *                      parameter.
 *
 * @return              WCS_OK in case of success, other way it will return
 *                      the error's case corresponding code.
 *
 * NOTE: 1. The output parameter count should be always greater than 1. This is
 *          because the parameter at index 0 represents the return type of the
 *          procedure and all Whais procedure have a return type defined.
 *       2. If the return code is WCS_LARGE_RESPONSE or WCS_LARGE_ARGS than
 *          the communication maximum frame size should be increased.
 */
CONNECTOR_SHL uint_t
WProcParamsCount(const WH_CONNECTION   hnd,
                 const char* const     name,
                 uint_t* const         outCount);

/* Get type information about a procedure parameter.
 *
 * This function will fail if it's not called with a connection handler that
 * was successfully authenticated with the the administrator account.
 *
 * @hnd                 The connection handle.
 * @procedure           Name of the procedure,
 * @parameter           The parameter index to retrieve information for. By
 *                      convention, the parameter at index 0 describes the
 *                      return type of a Whais procedure and continues with 1
 *                      for the first argument if it exists, 2 for the next,
 *                      and so on ...
 *
 * @return              WCS_OK in case of success, other way it will return
 *                      the error's case corresponding code.
 *
 * NOTE: If the return code is WCS_LARGE_RESPONSE or WCS_LARGE_ARGS than the
 *       communication maximum frame size should be increased.
 */
CONNECTOR_SHL uint_t
WProcParamType(const WH_CONNECTION   hnd,
               const char* const     procedure,
               const uint_t          parameter,
               uint_t* const         outRawType);

/* Get the fields count of a table procedure parameter.
 *
 * If a parameter procedure is described as being a table then this function
 * might be used to retrieve the count of table's fields.
 *
 * This function will fail if it's not called with a connection handler that
 * was successfully authenticated with the the administrator account.
 *
 * @hnd                 The connection handle.
 * @procedure           Name of the procedure,
 * @parameter           The parameter index to retrieve information for. By
 *                      convention, the parameter at index 0 describes the
 *                      return type of a Whais procedure and continues with 1
 *                      for the first argument if it exists, 2 for the next,
 *                      and so on ...
 * @outCount            In case of success it will hold the fields count.
 *
 * @return              WCS_OK in case of success, other way it will return
 *                      the error's case corresponding code.
 *
 * NOTE: If the return code is WCS_LARGE_RESPONSE or WCS_LARGE_ARGS than the
 *       communication maximum frame size should be increased.
 *
 */
CONNECTOR_SHL uint_t
WProcParamFieldCount(const WH_CONNECTION   hnd,
                     const char* const     procedure,
                     const uint_t          param,
                     uint_t* const         outCount);

/* Describes a field of a procedure parameter.
 *
 * If a parameter procedure is described as being a table then this function
 * might be used to retrieve details about the table's fields.
 *
 * This function will fail if it's not called with a connection handler that
 * was successfully authenticated with the the administrator account.
 *
 * @hnd                 The connection handle.
 * @procedure           Name of the procedure,
 * @parameter           The parameter index to retrieve information for. By
 *                      convention, the parameter at index 0 describes the
 *                      return type of a Whais procedure and continues with 1
 *                      for the first argument if it exists, 2 for the next,
 *                      and so on ...
 * @field               The field index to retrieve  information for.
 * @outpFieldName       In case of success it will hold a pointer the field
 *                      name. The name is a null terminated UTF-8 encoded
 *                      string.
 * @outFieldType        In case of success it will hold the field type.
 *
 * @return              WCS_OK in case of success, other way it will return
 *                      the error's case corresponding code.
 *
 * NOTE: 1. If the return code is WCS_LARGE_RESPONSE or WCS_LARGE_ARGS than the
 *          communication maximum frame size should be increased.
 *       2. The caller does not have the ownership of the memory pointed to
 *          by outpFieldName. It should make a copy of its content prior
 *          calling any other API using the same connection handle.
 */
CONNECTOR_SHL uint_t
WProcParamField(const WH_CONNECTION   hnd,
                const char* const     procedure,
                const uint_t          param,
                const uint_t          field,
                const char**          outpFieldName,
                uint_t* const         outFieldType);

/* Get the type of a global value.
 *
 * Retrieve the type of a global value. In case the global types is a table
 * WFetchField needs to be called repeatedly to retrieve the tables's fields'
 * names and types
 *
 * This function will fail if it's not called with a connection handler that
 * was successfully authenticated with the the administrator account.
 *
 * @hnd                 The connection handle.
 * @name                The global name. It should be a null terminated, UTF-8
 *                      encoded string.
 *
 * @outType             In case of success it will hold the global variable's
 *                      type.
 *
 * @return              WCS_OK in case of success, other way it will return
 *                      the error's case corresponding code.
 */
CONNECTOR_SHL uint_t
WDescribeGlobal(const WH_CONNECTION    hnd,
                const char* const      name,
                uint_t* const          outType);

/* Get the type of the stack top.
 *
 * Retrieves that type of the value from the connection stack's top. It is a
 * raw value, so extra effort has to be made in order to get the full value's
 * type description.
 */
CONNECTOR_SHL uint_t
WDescribeStackTop(const WH_CONNECTION   hnd,
                  uint_t* const         outRawType);

/* Get the list of fields of a table value.
 *
 * This function should be called only after a call to WDescribeGlobal or
 * WDescribeStackTop().
 * In case the value's type turns out to be a table, this function will be
 * called to retrieve the count of table fields.
  *
 * @hnd                 The connection handle.
 * @outCount            In case of success, this will hold the number of fields.
 *
 * @return              WCS_OK in case of success, other way it will return
 *                      the error's case corresponding code.
 */
CONNECTOR_SHL uint_t
WValueFieldsCount(const WH_CONNECTION  hnd,
                  uint_t* const        outCount);

/* Fetch the field type of the stack top value.
 *
 * This function should be called only after a call to WDescribeGlobal or
 * WDescribeStackTop().
 * In case the value's type turns out to be a table, this function will be
 * called repeatedly to retrieve the tables fields names and types.
 *
 * @hnd                 The connection handle.
 * @outFieldName        In case of success this will hold a pointer to the name
 *                      of the next table's field. The field name is a null
 *                      terminated UTF-8 encoded string. If there are no
 *                      fields left than this will be set to NULL.
 * @outFieldType        In case of success, this will hold the field type.
 *
 * @return              WCS_OK in case of success, other way it will return
 *                      the error's case corresponding code.
 *
 * NOTE: The caller does not have the ownership of the memory pointed to
 *       by outFieldName. It should make a copy of its content prior calling a
 *       any other API using the same connection handle.
 */
CONNECTOR_SHL uint_t
WValueFetchField(const WH_CONNECTION   hnd,
                 const char**          outFieldName,
                 uint_t* const         outFieldType);


/* Add a value on the stack top.
 *
 * Push a value on top of the connections stack. The newly added value will
 * have a null value bu default, but can be update with 'WUpdateValue()'.
 * The actual update would be cached internally(see 'WFlush()' for
 * more information).
 */

CONNECTOR_SHL uint_t
WPushValue(const WH_CONNECTION          hnd,
           const uint_t                 type,
           const uint_t                 fieldsCount,
           const struct WField* const   fields);

/* Remove values from the stack top.
 *
 * Clears a certain number of values from the connection stack. The actual
 * update would be cached internally(see 'WFlush()' for more
 * information).
 */
CONNECTOR_SHL uint_t
WPopValues(const WH_CONNECTION hnd, const uint_t count);


/* Set stack's top value.
 *
 * Change the content of the top stack value. It can be used repeatedly in case
 * of table setups. The updates should made sequentially due to avoid failures
 * of internal limit. The rows of a table should be updated starting
 * from lower to bigger, array elements should update again from lower to
 * bigger indexes, etc.
 *
 * Note: 1. There is no way to to have a table with a NULL row.
 *       2. The result of this could be a fake success, as all it could be
 *          cached internally(see 'WFlush()' for more details).
 */
CONNECTOR_SHL uint_t
WUpdateValue(const WH_CONNECTION   hnd,
             const uint_t          type,
             const char* const     field,
             const WHT_ROW_INDEX   row,
             const WHT_INDEX       arrayOff,
             const WHT_INDEX       textOff,
             const char* const     value);

/* Add rows to a stack's top table value.
 *
 *
 */
CONNECTOR_SHL uint_t
WAddTableRows(const WH_CONNECTION   hnd,
              const int32_t         rowsCount);

/* Flush all stack commits.
 *
 * To maximise network bandwidth, all stack update operations(push, pop and
 * set) might be cached internally. This forces their processing by server.
 *
 * Note: 1. If of the operations fails while is processed on the server side,
 *          the rest of the operation are canceled.
 *       2. If ones to know exactly what operation fails, this has to be called
 *          after all stack update functions.
 */
CONNECTOR_SHL uint_t
WFlush(const WH_CONNECTION hnd);

/* Get the number of rows of the stack top values.
 *
 * It fails if the stack top does not hold a value with valid
 * type(table or field).
 */
CONNECTOR_SHL uint_t
WValueRowsCount(const WH_CONNECTION   hnd,
                ullong_t* const       outCount);

/* Get the top stack values array elements count. */
CONNECTOR_SHL uint_t
WValueArraySize(const WH_CONNECTION   hnd,
                const char*           field,
                const WHT_ROW_INDEX   row,
                ullong_t* const       outCount);

/* Get the stack top text length.
 *
 * The text values are encoded using UTF-8, and the length represents the
 * number of characters and not the number of bytes used to represent the
 * text. Also the length does not count the null terminator character.
 */
CONNECTOR_SHL uint_t
WValueTextLength(const WH_CONNECTION   hnd,
                 const char*           field,
                 const WHT_ROW_INDEX   row,
                 const WHT_INDEX       arrayOff,
                 ullong_t* const       outCount);

/* Retrieve the stack top value.
 *
 * Get a representation of the actual value. The value is represented by an
 * null terminated UTF-8 encoded string.
 *
 * Note: 1. To improve caching usage the rows values are faster retrieved
 *          sequentially, from lower rows to bigger ones, and using the same
 *          fields order as the one returned during fetching.
 *       2. For big text values, not all chars will be returned by this call.
 *          Using a combination of 'WValueTextLength()' and a text
 *          offset the all text should be accessible.
 *       3. Any text offsets are in terms of Unicode code points rather than
 *          bytes.
 */
CONNECTOR_SHL uint_t
WValueEntry(const WH_CONNECTION   hnd,
            const char* const     field,
            WHT_ROW_INDEX         row,
            const WHT_INDEX       arrayOff,
            const WHT_INDEX       textOff,
            const char** const    outpValue);

/* Execute a procedure remotely.
 *
 * The arguments of the specified procedure shall already been passed on the
 * stack, using the stack update functions.
 */
CONNECTOR_SHL uint_t
WExecuteProcedure(const WH_CONNECTION   hnd,
                  const char* const     procedure);

#ifdef __cplusplus
}
#endif
#endif /* WHAIS_CONNECTOR_H_ */

