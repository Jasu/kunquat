

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <module/Env_var.h>
#include <string/common.h>
#include <string/var_name.h>


struct Env_var
{
    char name[KQT_VAR_NAME_MAX];
    Value value;
};


Env_var* new_Env_var(Value_type type, const char* name)
{
    assert((type == VALUE_TYPE_BOOL) || (type == VALUE_TYPE_INT) ||
            (type == VALUE_TYPE_FLOAT) || (type == VALUE_TYPE_TSTAMP));
    assert(name != NULL);
    assert(is_valid_var_name(name));

    Env_var* var = memory_alloc_item(Env_var);
    if (var == NULL)
        return NULL;

    strcpy(var->name, name);
    var->value.type = type;

    return var;
}


Env_var* new_Env_var_from_string(Streader* sr)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    char type_name[16] = "";
    char name[KQT_VAR_NAME_MAX] = "";

    if (!Streader_readf(sr, "[%s,%s,", 16, type_name, KQT_VAR_NAME_MAX, name))
        return NULL;

    if (!is_valid_var_name(name))
    {
        Streader_set_error(
                sr,
                "Illegal variable name %s"
                    " (Variable names may only contain"
                    " lower-case letters and underscores"
                    " (and digits as other than first characters))",
                name);
        return NULL;
    }

    Value* value = VALUE_AUTO;

    if (string_eq(type_name, "bool"))
    {
        value->type = VALUE_TYPE_BOOL;
        Streader_read_bool(sr, &value->value.bool_type);
    }
    else if (string_eq(type_name, "int"))
    {
        value->type = VALUE_TYPE_INT;
        Streader_read_int(sr, &value->value.int_type);
    }
    else if (string_eq(type_name, "float"))
    {
        value->type = VALUE_TYPE_FLOAT;
        Streader_read_float(sr, &value->value.float_type);
    }
#if 0
    else if (string_eq(type_name, "real"))
    {
        value->type = VALUE_TYPE_REAL;
        *str = read_tuning(*str, &value->value.Real_type, NULL, state);
    }
#endif
    else if (string_eq(type_name, "timestamp"))
    {
        value->type = VALUE_TYPE_TSTAMP;
        Streader_read_tstamp(sr, &value->value.Tstamp_type);
    }
    else
    {
        Streader_set_error(
                sr,
                "Invalid type of environment variable %s: %s",
                name,
                type_name);
        return NULL;
    }

    if (!Streader_match_char(sr, ']'))
        return NULL;

    Env_var* var = new_Env_var(value->type, name);
    if (var == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for environment variable");
        return NULL;
    }

    Env_var_set_value(var, value);

    return var;
}


Value_type Env_var_get_type(const Env_var* var)
{
    assert(var != NULL);
    return var->value.type;
}


const char* Env_var_get_name(const Env_var* var)
{
    assert(var != NULL);
    return var->name;
}


void Env_var_set_value(Env_var* var, const Value* value)
{
    assert(var != NULL);
    assert(value != NULL);
    assert(var->value.type == value->type);

    Value_copy(&var->value, value);

    return;
}


const Value* Env_var_get_value(const Env_var* var)
{
    assert(var != NULL);
    return &var->value;
}


void del_Env_var(Env_var* var)
{
    if (var == NULL)
        return;

    memory_free(var);

    return;
}


