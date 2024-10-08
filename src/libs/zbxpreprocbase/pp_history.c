/*
** Copyright (C) 2001-2024 Zabbix SIA
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

#include "zbxpreprocbase.h"

#include "zbxalgo.h"
#include "zbxtime.h"
#include "zbxvariant.h"

ZBX_VECTOR_IMPL(pp_step_history, zbx_pp_step_history_t)

/******************************************************************************
 *                                                                            *
 * Purpose: create preprocessing history                                      *
 *                                                                            *
 * Parameters: history_num - [IN] number of steps using history               *
 *                                                                            *
 * Return value: The created preprocessing history.                           *
 *                                                                            *
 ******************************************************************************/
zbx_pp_history_t	*zbx_pp_history_create(int history_num)
{
	zbx_pp_history_t	*history = (zbx_pp_history_t *)zbx_malloc(NULL, sizeof(zbx_pp_history_t));
	history->refcount = 1;

	zbx_vector_pp_step_history_create(&history->step_history);

	if (0 != history_num)
		zbx_vector_pp_step_history_reserve(&history->step_history, (size_t)history_num);

	return history;
}

/******************************************************************************
 *                                                                            *
 * Purpose: free preprocessing history                                        *
 *                                                                            *
 ******************************************************************************/
void	zbx_pp_history_free(zbx_pp_history_t *history)
{
	if (NULL == history || 0 != --history->refcount)
		return;

	for (int i = 0; i < history->step_history.values_num; i++)
		zbx_variant_clear(&history->step_history.values[i].value);

	zbx_vector_pp_step_history_destroy(&history->step_history);

	zbx_free(history);
}

/******************************************************************************
 *                                                                            *
 * Purpose: reserve preprocessing history                                     *
 *                                                                            *
 ******************************************************************************/
void	zbx_pp_history_reserve(zbx_pp_history_t *history, int history_num)
{
	zbx_vector_pp_step_history_reserve(&history->step_history, (size_t)history_num);
}

/******************************************************************************
 *                                                                            *
 * Purpose: add value to preprocessing history                                *
 *                                                                            *
 * Parameters: history - [IN] preprocessing history                           *
 *             index   - [IN] preprocessing step index                        *
 *             value   - [IN/OUT] value to add, its resources are copied      *
 *                         over to history and the value itself is reset      *
 *             ts      - [IN] value timestamp                                 *
 *                                                                            *
 ******************************************************************************/
void	zbx_pp_history_add(zbx_pp_history_t *history, int index, zbx_variant_t *value, zbx_timespec_t ts)
{
	zbx_pp_step_history_t	step_history;

	step_history.index = index;
	step_history.value = *value;
	step_history.ts = ts;

	zbx_variant_set_none(value);

	zbx_vector_pp_step_history_append_ptr(&history->step_history, &step_history);
}

/******************************************************************************
 *                                                                            *
 * Purpose: remove value from preprocessing history and return it             *
 *                                                                            *
 * Parameters: history - [IN] preprocessing history                           *
 *             index   - [IN] preprocessing step index                        *
 *             value   - [OUT] value. If there is no history for the          *
 *                             requested step then empty variant              *
 *                             (ZBX_VBARIANT_NONE) is returned                *
 *             ts      - [OUT] value timestamp. If there is no history        *
 *                             for the requested step then 0 timestamp is     *
 *                             returned                                       *
 *                                                                            *
 ******************************************************************************/
void	zbx_pp_history_pop(zbx_pp_history_t *history, int index, zbx_variant_t *value, zbx_timespec_t *ts)
{
	if (NULL != history)
	{
		for (int i = 0; i < history->step_history.values_num; i++)
		{
			if (history->step_history.values[i].index == index)
			{
				*value = history->step_history.values[i].value;
				*ts = history->step_history.values[i].ts;
				zbx_vector_pp_step_history_remove_noorder(&history->step_history, i);

				return;
			}
		}
	}

	zbx_variant_set_none(value);
	ts->sec = 0;
	ts->ns = 0;
}

void	zbx_pp_history_init(zbx_pp_history_t *history)
{
	zbx_vector_pp_step_history_create(&history->step_history);
}

void	zbx_pp_history_clear(zbx_pp_history_t *history)
{
	for (int i = 0; i < history->step_history.values_num; i++)
		zbx_variant_clear(&history->step_history.values[i].value);

	zbx_vector_pp_step_history_destroy(&history->step_history);
}

zbx_pp_history_t	*zbx_pp_history_clone(zbx_pp_history_t *history)
{
	if (NULL != history)
		history->refcount++;

	return history;
}
