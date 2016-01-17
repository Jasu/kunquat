

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_force.h>

#include <debug/assert.h>
#include <Error.h>
#include <init/devices/Device_impl.h>
#include <memory.h>
#include <player/devices/processors/Force_state.h>
#include <string/Streader.h>

#include <stdlib.h>
#include <string.h>


static Set_float_func       Proc_force_set_global_force;
static Set_float_func       Proc_force_set_force_variation;

static Set_envelope_func    Proc_force_set_env;
static Set_bool_func        Proc_force_set_env_enabled;
static Set_bool_func        Proc_force_set_env_loop_enabled;
static Set_float_func       Proc_force_set_env_scale_amount;
static Set_float_func       Proc_force_set_env_scale_center;

static Set_envelope_func    Proc_force_set_env_rel;
static Set_bool_func        Proc_force_set_env_rel_enabled;
static Set_float_func       Proc_force_set_env_rel_scale_amount;
static Set_float_func       Proc_force_set_env_rel_scale_center;

static Set_bool_func        Proc_force_set_release_ramp;


static void del_Proc_force(Device_impl* dimpl);


Device_impl* new_Proc_force(void)
{
    Proc_force* force = memory_alloc_item(Proc_force);
    if (force == NULL)
        return NULL;

    force->global_force = 0.0;
    force->force_var = 0.0;

    force->force_env = NULL;
    force->is_force_env_enabled = false;
    force->is_force_env_loop_enabled = false;
    force->force_env_scale_amount = 0.0;
    force->force_env_scale_center = 0.0;

    force->force_release_env = NULL;
    force->is_force_release_env_enabled = false;
    force->force_release_env_scale_amount = 0.0;
    force->force_release_env_scale_center = 0.0;

    force->def_force_release_env = NULL;

    force->is_release_ramping_enabled = false;

    if (!Device_impl_init(&force->parent, del_Proc_force))
    {
        del_Device_impl(&force->parent);
        return NULL;
    }

    force->parent.get_vstate_size = Force_vstate_get_size;
    force->parent.init_vstate = Force_vstate_init;

    // Add default release envelope
    {
        force->def_force_release_env = new_Envelope(2, 0, 1, 0, 0, 1, 0);
        if (force->def_force_release_env == NULL)
        {
            del_Device_impl(&force->parent);
            return NULL;
        }

        static const char* env_data =
            "{ \"nodes\": [ [0, 1], [1, 0] ], \"smooth\": false }";
        Streader* sr = Streader_init(STREADER_AUTO, env_data, strlen(env_data));

        if (!Envelope_read(force->def_force_release_env, sr))
        {
            // The default envelope should be valid
            assert((sr->error.type == ERROR_MEMORY) ||
                    (sr->error.type == ERROR_RESOURCE));

            del_Device_impl(&force->parent);
            return NULL;
        }

        force->force_release_env = force->def_force_release_env;
    }

    // Register key handlers
    bool reg_success = true;

#define REGISTER_KEY(type, field, key, def_val)                           \
    reg_success &= Device_impl_register_set_ ## type(                     \
            &force->parent, key, def_val, Proc_force_set_ ## field, NULL)

    REGISTER_KEY(float,     global_force,       "p_f_global_force.json",        0.0);
    REGISTER_KEY(float,     force_variation,    "p_f_force_variation.json",     0.0);
    REGISTER_KEY(envelope,  env,                "p_e_env.json",                 NULL);
    REGISTER_KEY(bool,      env_enabled,        "p_b_env_enabled.json",         false);
    REGISTER_KEY(bool,      env_loop_enabled,   "p_b_env_loop_enabled.json",    false);
    REGISTER_KEY(float,     env_scale_amount,   "p_f_env_scale_amount.json",    0.0);
    REGISTER_KEY(float,     env_scale_center,   "p_f_env_scale_center.json",    0.0);
    REGISTER_KEY(envelope,  env_rel,            "p_e_env_rel.json",             NULL);
    REGISTER_KEY(bool,      env_rel_enabled,    "p_b_env_rel_enabled.json",     false);
    REGISTER_KEY(float,   env_rel_scale_amount, "p_f_env_rel_scale_amount.json", 0.0);
    REGISTER_KEY(float,   env_rel_scale_center, "p_f_env_rel_scale_center.json", 0.0);
    REGISTER_KEY(bool,      release_ramp,       "p_b_release_ramp.json",        false);

#undef REGISTER_KEY

    if (!reg_success)
    {
        del_Device_impl(&force->parent);
        return NULL;
    }

    return &force->parent;
}


static bool Proc_force_set_global_force(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->global_force = isfinite(value) ? value : 0.0;

    return true;
}


static bool Proc_force_set_force_variation(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->force_var = (isfinite(value) && (value >= 0)) ? value : 0.0;

    return true;
}


static bool is_valid_force_envelope(const Envelope* env)
{
    if (env == NULL)
        return false;

    const int node_count = Envelope_node_count(env);
    if ((node_count < 2) || (node_count > 32))
        return false;

    // Check the first node x coordinate
    {
        const double* node = Envelope_get_node(env, 0);
        if (node[0] != 0)
            return false;
    }

    // Check y coordinates
    for (int i = 0; i < node_count; ++i)
    {
        const double* node = Envelope_get_node(env, i);
        if ((node[1] < 0) || (node[1] > 1))
            return false;
    }

    return true;
}


static bool Proc_force_set_env(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->force_env = is_valid_force_envelope(value) ? value : NULL;

    return true;
}


static bool Proc_force_set_env_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->is_force_env_enabled = value;

    return true;
}


static bool Proc_force_set_env_loop_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->is_force_env_loop_enabled = value;

    return true;
}


static bool Proc_force_set_env_scale_amount(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->force_env_scale_amount = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_force_set_env_scale_center(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->force_env_scale_center = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_force_set_env_rel(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;

    force->force_release_env = force->def_force_release_env;

    if (!is_valid_force_envelope(value))
        return true;

    // Check that we end with silence
    {
        const int node_count = Envelope_node_count(value);
        const double* last_node = Envelope_get_node(value, node_count - 1);
        if (last_node[1] != 0)
            return true;
    }

    force->force_release_env = value;

    return true;
}


static bool Proc_force_set_env_rel_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->is_force_release_env_enabled = value;

    return true;
}


static bool Proc_force_set_env_rel_scale_amount(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->force_release_env_scale_amount = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_force_set_env_rel_scale_center(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->force_release_env_scale_center = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_force_set_release_ramp(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->is_release_ramping_enabled = value;

    return true;
}


static void del_Proc_force(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_force* force = (Proc_force*)dimpl;
    del_Envelope(force->def_force_release_env);
    memory_free(force);

    return;
}


