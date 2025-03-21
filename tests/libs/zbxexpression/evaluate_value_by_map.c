/*
** Copyright (C) 2001-2025 Zabbix SIA
**
** This program is free software: you can redistribute it and/or modify it under the terms of
** the GNU Affero General Public License as published by the Free Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
** without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public License along with this program.
** If not, see <https://www.gnu.org/licenses/>.
**/

#include "zbxcommon.h"

#include "zbxmocktest.h"
#include "zbxmockdata.h"
#include "zbxmockassert.h"
#include "zbxmockutil.h"

#include "zbxexpression.h"
#include "zbxlog.h"
#include "evalfunc.h"

void	zbx_mock_test_entry(void **state)
{
	int				ret, expected_ret;
	char				value[ZBX_VALUEMAP_STRING_LEN], *newvalue;
	unsigned char			value_type;
	zbx_vector_valuemaps_ptr_t	valuemaps;
	zbx_valuemaps_t			*valuemap;
	zbx_mock_handle_t		hvaluemaps, handle;
	zbx_mock_error_t		err;

	zbx_vector_valuemaps_ptr_create(&valuemaps);

	hvaluemaps = zbx_mock_get_parameter_handle("in.valuemaps");

	while (ZBX_MOCK_END_OF_VECTOR != (err = (zbx_mock_vector_element(hvaluemaps, &handle))))
	{
		if (ZBX_MOCK_SUCCESS != err)
		{
			fail_msg("Cannot read 'valuemaps' element #%d: %s", valuemaps.values_num,
					zbx_mock_error_string(err));
		}
		valuemap = (zbx_valuemaps_t *)zbx_malloc(NULL, sizeof(zbx_valuemaps_t));
		valuemap->type = (int)zbx_mock_get_object_member_uint64(handle, "type");

		zbx_snprintf(valuemap->value, ZBX_VALUEMAP_STRING_LEN, "%s",
				(char *)zbx_mock_get_object_member_string(handle, "value"));
		zbx_snprintf(valuemap->newvalue, ZBX_VALUEMAP_STRING_LEN, "%s",
				(char *)zbx_mock_get_object_member_string(handle, "newvalue"));
		zbx_vector_valuemaps_ptr_append(&valuemaps, valuemap);
	}

	zbx_snprintf(value, ZBX_VALUEMAP_STRING_LEN, "%s", zbx_mock_get_parameter_string("in.value"));
	value_type = (unsigned char)zbx_mock_get_parameter_uint64("in.type");
	newvalue = (char *)zbx_mock_get_parameter_string("out.value");
	expected_ret = zbx_mock_str_to_return_code(zbx_mock_get_parameter_string("out.return"));

	ret = evaluate_value_by_map(value, sizeof(value), &valuemaps, value_type);

	zbx_mock_assert_int_eq("valuemaps return value", expected_ret, ret);
	zbx_mock_assert_str_eq("new value", newvalue, value);

	zbx_vector_valuemaps_ptr_clear_ext(&valuemaps, zbx_valuemaps_free);
	zbx_vector_valuemaps_ptr_destroy(&valuemaps);
	ZBX_UNUSED(state);
}
