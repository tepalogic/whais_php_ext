#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_whais.h"
#include "zend_list.h"

#include "whais_params.h"

#include <string>
#include <vector>

using namespace std;


namespace whais {

long
map_php_to_whais_type(zval& param, long typeHint)
{
	if ((typeHint == WHC_TYPE_NOTSET) || (typeHint == 0))
	{
		switch (Z_TYPE(param))
		{
			case IS_NULL:
			case IS_UNDEF:
				return WHC_TYPE_NOTSET;

			case IS_FALSE:
			case IS_TRUE:
				return WHC_TYPE_BOOL;

			case IS_LONG:
				return WHC_TYPE_INT64;

			case IS_DOUBLE:
				return WHC_TYPE_RICHREAL;

			case IS_STRING:
				return WHC_TYPE_TEXT;

			case IS_ARRAY:
				if (zend_hash_index_exists(Z_ARRVAL(param), 0)) {
					zval* firstElem = zend_hash_index_find(Z_ARRVAL(param), 0);
					return WHC_TYPE_ARRAY_MASK | map_php_to_whais_type(*firstElem, WHC_TYPE_NOTSET);
				}
				return WHC_TYPE_TABLE_MASK;
			default:
				return WHC_TYPE_NOTSET;
		}
	}
	return typeHint;
}

static zval
zval_from_db(const bool doZvalConv, const uint_t type, const char* value)
{
	zval result;

	ZVAL_NULL(&result);
	if (value[0] == 0) {
		return result;
	}

	if ( ! doZvalConv) {
		ZVAL_STRING(&result, value);
		return result;
	}

	switch (type & ~(WHC_TYPE_ARRAY_MASK | WHC_TYPE_FIELD_MASK | WHC_TYPE_TABLE_MASK)) 
	{
		case WHC_TYPE_BOOL:
			ZVAL_BOOL(&result, (value[0] == '1') || (value[0] == 'T') || (value[0] == 't'));
			break;

		case WHC_TYPE_CHAR:
		case WHC_TYPE_TEXT:
			ZVAL_STRING(&result, value);
			break;

		case WHC_TYPE_INT8:
		case WHC_TYPE_INT16:
		case WHC_TYPE_INT32:
		case WHC_TYPE_INT64:
		case WHC_TYPE_UINT8:
		case WHC_TYPE_UINT16:
		case WHC_TYPE_UINT32:
		case WHC_TYPE_UINT64:
			ZVAL_LONG(&result, atol(value));
			break;

		case WHC_TYPE_REAL:
		case WHC_TYPE_RICHREAL:
			ZVAL_DOUBLE(&result, atof(value));
			break;

		case WHC_TYPE_DATE:
		case WHC_TYPE_DATETIME:
		case WHC_TYPE_HIRESTIME:
			ZVAL_STRING(&result, value);
			break;
	}

	return result;
}


static zval*
fetch_db_table_result(WH_CONNECTION hnd, const bool doZvalConv, const vector<string>& fields, const vector<uint_t>& fieldTypes, zval& result) 
{
	vector<zval> fieldRows(fields.size());

	ullong_t rowsCount;
	if (WValueRowsCount(hnd, &rowsCount) != WCS_OK) {
		return nullptr;
	}

	for (size_t f = 0; f < fields.size(); ++f) {
		array_init(&fieldRows[f]);
	}

	for (ullong_t row = 0; row < rowsCount; ++row) {
		for (size_t i = 0; i < fieldTypes.size(); ++i) {
			if (fieldTypes[i] & WHC_TYPE_ARRAY_MASK) {
				ullong_t arrayCount;
				zval fieldContent;
				array_init(&fieldContent);

				if (WValueArraySize(hnd, fields[i].c_str(), row, &arrayCount) != WCS_OK) {
					return nullptr;
				}

				for (ullong_t e = 0; e < arrayCount; ++e) {
					const char* entry;
					if (WValueEntry(hnd, fields[i].c_str(), row, e, WIGNORE_OFF, &entry) != WCS_OK) {
						return nullptr;
					}

					auto dbZval = zval_from_db(doZvalConv, fieldTypes[i], entry);
					add_next_index_zval(&fieldContent, & dbZval);
				}
				add_next_index_zval(&fieldRows[i], &fieldContent);
			} else if ((fieldTypes[i] & 0xFF) == WHC_TYPE_TEXT) {
				ullong_t textSize;
				string text;
				if (WValueTextLength(hnd, fields[i].c_str(), row, WIGNORE_OFF, &textSize) != WCS_OK) {
					return nullptr;
				}

				ullong_t textFetched = 0;
				while (textFetched < textSize) {
					const char* entry;
					if (WValueEntry(hnd, fields[i].c_str(), row, WIGNORE_OFF, textFetched, &entry) != WCS_OK) {
						return nullptr;
					}
					text += entry;
					textFetched += strlen(entry);
				}

				add_next_index_string(&fieldRows[i], text.c_str());
			} else {
				const char *entry;
				if (WValueEntry(hnd, fields[i].c_str(), row, WIGNORE_OFF, WIGNORE_OFF, &entry) != WCS_OK) {
					return nullptr;
				}
				auto dbZval = zval_from_db(doZvalConv, fieldTypes[i], entry);
				add_next_index_zval(&fieldRows[i], &dbZval);
			}
		}
	}

	array_init(&result);
	for (size_t f = 0; f < fields.size(); ++f ) {
		add_assoc_zval(&result, fields[f].c_str(), &fieldRows[f]);
	}	
	
	return &result;
}

zval* 
fetch_db_result(WH_CONNECTION hnd, const bool doZvalConv, zval& result)
{
	uint_t rawType;
	if (WDescribeStackTop(hnd, &rawType) != WCS_OK) {
	  return nullptr;
	}

	if (rawType & WHC_TYPE_TABLE_MASK) {
		vector<string> fields;
		vector<uint_t> fieldTypes;
		uint_t fieldsCount;
		if (WValueFieldsCount(hnd, &fieldsCount) != WCS_OK) {
			return nullptr;
		}

		for (uint_t f = 0; f < fieldsCount; ++f) {
			const char* fieldName;
			uint_t fieldType;
			if (WValueFetchField(hnd, &fieldName, &fieldType) != WCS_OK) {
				return nullptr;
			}
			fields.push_back(fieldName);
			fieldTypes.push_back(fieldType);
		}

		return fetch_db_table_result(hnd, doZvalConv, fields, fieldTypes, result);
	} else if (rawType & WHC_TYPE_FIELD_MASK) {
		ullong_t rowsCount;
		if (WValueRowsCount(hnd, &rowsCount) != WCS_OK) {
			return nullptr;
		}

		array_init(&result);
		for (ullong_t row = 0; row < rowsCount; ++row) {
			if (rawType & WHC_TYPE_ARRAY_MASK) {
				zval element;
				ullong_t count;

				if (WValueArraySize(hnd, WIGNORE_FIELD, row, &count) != WCS_OK) {
					return nullptr;
				}

				array_init(&element);
				for (ullong_t i = 0; i < count; ++i) {
					const char* arrayEntry;
					if (WValueEntry(hnd, WIGNORE_FIELD, row, i, WIGNORE_OFF, &arrayEntry) != WCS_OK) {
						return nullptr;
					}
					auto dbZval =  zval_from_db(doZvalConv, rawType, arrayEntry);
					add_next_index_zval(&element, &dbZval);
				}
				add_next_index_zval(&result, &element);
			} else if ((rawType & 0xFF) == WHC_TYPE_TEXT) {
				ullong_t textLength;
				if (WValueTextLength(hnd, WIGNORE_FIELD, row, WIGNORE_OFF, &textLength) != WCS_OK) {
					return nullptr;
				}

				string textResult;
				ullong_t fetchedLen = 0;
				while (fetchedLen < textLength) {
					const char *callResult;
					if (WValueEntry(hnd, WIGNORE_FIELD, row, WIGNORE_OFF, fetchedLen, &callResult) != WCS_OK) {
						return nullptr;
					}

					textResult += callResult;
					fetchedLen += strlen(callResult);
				}

				add_next_index_string(&result, textResult.c_str());
			} else {
				const char* callResult;
				if (WValueEntry(hnd, WIGNORE_FIELD, row, WIGNORE_OFF, WIGNORE_OFF, &callResult) != WCS_OK) {
					return nullptr;
				}
				auto dbZval = zval_from_db(doZvalConv, rawType, callResult);
				add_next_index_zval(&result, &dbZval);
			}
		}
	} else if (rawType & WHC_TYPE_ARRAY_MASK) {
		ullong_t count = 0;
		if (WValueArraySize(hnd, WIGNORE_FIELD, WIGNORE_ROW, &count) != WCS_OK) {
			return nullptr;
		}

		array_init(&result);
		for (ullong_t i = 0; i < count; ++i) {
			const char* arrayEntry;
			if (WValueEntry(hnd, WIGNORE_FIELD, WIGNORE_ROW, i, WIGNORE_OFF, &arrayEntry) != WCS_OK) {
				return nullptr;
			}

			if (arrayEntry[0] != 0) {
				auto dbZval = zval_from_db(doZvalConv, rawType, arrayEntry);
				add_next_index_zval(&result, &dbZval);
			}
		}
	} else if ((rawType & 0xFF) == WHC_TYPE_TEXT) {
		ullong_t resultLength;
		if (WValueTextLength(hnd, WIGNORE_FIELD, WIGNORE_ROW, WIGNORE_OFF, &resultLength) != WCS_OK) {
			return nullptr;
		}

		ullong_t fetchedLen = 0;
		string textResult;
		while (fetchedLen < resultLength) {
			const char *callResult;
			if (WValueEntry(hnd, WIGNORE_FIELD, WIGNORE_ROW, WIGNORE_OFF, fetchedLen, &callResult) != WCS_OK) {
				return nullptr;
			}

			textResult += callResult;
			fetchedLen += strlen(callResult);
		}
		ZVAL_STRING(&result, textResult.c_str());
	} else {
		const char* callResult;
		if (WValueEntry(hnd, WIGNORE_FIELD, WIGNORE_ROW, WIGNORE_OFF, WIGNORE_OFF, &callResult) != WCS_OK) {
			return nullptr;
		}
		result = zval_from_db(doZvalConv, rawType, callResult);
	}

	return &result;
}

} /*namespace whais */

