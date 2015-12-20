

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <Connections.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <Device_node.h>
#include <devices/Audio_unit.h>
#include <memory.h>
#include <string/common.h>

#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Connections
{
    AAtree* nodes;
    AAiter* iter;
};


/**
 * Resets the graph for searching purposes.
 *
 * \param graph   The Connections -- must not be \c NULL.
 */
static void Connections_reset(Connections* graph);


/**
 * Tells whether there is a cycle inside Connections.
 *
 * All Connections must be acyclic.
 *
 * \param graph   The Connections -- must not be \c NULL.
 *
 * \return   \c true if there is a cycle in \a graph, otherwise \c false.
 */
static bool Connections_is_cyclic(Connections* graph);


/**
 * Validates a connection path.
 *
 * This function also strips the port directory off the path.
 *
 * \param str     The path -- must not be \c NULL.
 * \param level   The connection level -- must be valid.
 * \param type    The type of the path -- must be valid.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The port number if the path is valid, otherwise \c -1.
 */
static int validate_connection_path(
        Streader* sr, char* str, Connection_level level, Device_port_type type);


#define mem_error_if(expr, graph, node, sr)                           \
    if (true)                                                         \
    {                                                                 \
        if ((expr))                                                   \
        {                                                             \
            Streader_set_memory_error(                                \
                    sr, "Could not allocate memory for connections"); \
            del_Device_node(node);                                    \
            del_Connections(graph);                                   \
            return NULL;                                              \
        }                                                             \
    } else (void)0

typedef struct read_conn_data
{
    Connections* graph;
    Connection_level level;
    Au_table* au_table;
    Device* master;
} read_conn_data;

static bool read_connection(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    assert(userdata != NULL);
    ignore(index);

    read_conn_data* rcdata = userdata;

    char src_name[KQT_DEVICE_NODE_NAME_MAX] = "";
    char dest_name[KQT_DEVICE_NODE_NAME_MAX] = "";
    if (!Streader_readf(
                sr,
                "[%s,%s]",
                KQT_DEVICE_NODE_NAME_MAX, src_name,
                KQT_DEVICE_NODE_NAME_MAX, dest_name))
        return false;

    int src_port = validate_connection_path(
            sr,
            src_name,
            rcdata->level,
            DEVICE_PORT_TYPE_SEND);
    int dest_port = validate_connection_path(
            sr,
            dest_name,
            rcdata->level,
            DEVICE_PORT_TYPE_RECEIVE);
    if (Streader_is_error_set(sr))
        return false;

    if (rcdata->level == CONNECTION_LEVEL_AU)
    {
        if (string_eq(src_name, ""))
            strcpy(src_name, "Iin");
    }

    if (AAtree_get_exact(rcdata->graph->nodes, src_name) == NULL)
    {
        const Device* actual_master = rcdata->master;
        if ((rcdata->level == CONNECTION_LEVEL_AU) &&
                string_eq(src_name, "Iin"))
            actual_master = Audio_unit_get_input_interface((Audio_unit*)rcdata->master);

        Device_node* new_src = new_Device_node(
                src_name, rcdata->au_table, actual_master);

        mem_error_if(new_src == NULL, rcdata->graph, NULL, sr);
        mem_error_if(
                !AAtree_ins(rcdata->graph->nodes, new_src),
                rcdata->graph,
                new_src,
                sr);
    }
    Device_node* src_node = AAtree_get_exact(rcdata->graph->nodes, src_name);

    if (AAtree_get_exact(rcdata->graph->nodes, dest_name) == NULL)
    {
        Device_node* new_dest = new_Device_node(
                dest_name, rcdata->au_table, rcdata->master);

        mem_error_if(new_dest == NULL, rcdata->graph, NULL, sr);
        mem_error_if(
                !AAtree_ins(rcdata->graph->nodes, new_dest),
                rcdata->graph,
                new_dest,
                sr);
    }
    Device_node* dest_node = AAtree_get_exact(rcdata->graph->nodes, dest_name);

    assert(src_node != NULL);
    assert(dest_node != NULL);
    mem_error_if(
            !Device_node_connect(dest_node, dest_port, src_node, src_port),
            rcdata->graph,
            NULL,
            sr);

    return true;
}

Connections* new_Connections_from_string(
        Streader* sr, Connection_level level, Au_table* au_table, Device* master)
{
    assert(sr != NULL);
    assert(au_table != NULL);
    assert(master != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Connections* graph = memory_alloc_item(Connections);
    if (graph == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for connections");
        return NULL;
    }

    graph->nodes = NULL;
    graph->iter = NULL;
    graph->nodes = new_AAtree(
            (int (*)(const void*, const void*))Device_node_cmp,
            (void (*)(void*))del_Device_node);
    mem_error_if(graph->nodes == NULL, graph, NULL, sr);
    graph->iter = new_AAiter(graph->nodes);
    mem_error_if(graph->iter == NULL, graph, NULL, sr);

    Device_node* master_node = NULL;
    if (level == CONNECTION_LEVEL_AU)
    {
        const Device* iface = Audio_unit_get_output_interface((Audio_unit*)master);
        master_node = new_Device_node("", au_table, iface);
    }
    else
    {
        master_node = new_Device_node("", au_table, master);
    }
    mem_error_if(master_node == NULL, graph, NULL, sr);
    mem_error_if(!AAtree_ins(graph->nodes, master_node), graph, master_node, sr);

    if (!Streader_has_data(sr))
    {
        Connections_reset(graph);
        return graph;
    }

    read_conn_data rcdata = { graph, level, au_table, master };
    if (!Streader_read_list(sr, read_connection, &rcdata))
    {
        del_Connections(graph);
        return NULL;
    }

    if (Connections_is_cyclic(graph))
    {
        Streader_set_error(sr, "The connection graph contains a cycle");
        del_Connections(graph);
        return NULL;
    }

    Connections_reset(graph);

    return graph;
}

#undef mem_error_if


bool Connections_check_connections(
        const Connections* graph, char err[DEVICE_CONNECTION_ERROR_LENGTH_MAX])
{
    assert(graph != NULL);
    assert(err != NULL);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, graph->nodes);

    Device_node* node = AAiter_get_at_least(iter, "");
    while (node != NULL)
    {
        if (!Device_node_check_connections(node, err))
            return false;

        node = AAiter_get_next(iter);
    }

    return true;
}


Device_node* Connections_get_master(Connections* graph)
{
    assert(graph != NULL);
    return AAtree_get_exact(graph->nodes, "");
}


bool Connections_prepare(Connections* graph, Device_states* states)
{
    assert(graph != NULL);
    assert(states != NULL);

    return Connections_init_buffers(graph, states);
}


bool Connections_init_buffers(Connections* graph, Device_states* states)
{
    assert(graph != NULL);
    assert(states != NULL);

    Device_node* master = AAtree_get_exact(graph->nodes, "");
    assert(master != NULL);
    Device_node_reset(master);
    if (!Device_node_init_buffers_simple(master, states))
        return false;

    Device_node_reset(master);
    return Device_node_init_effect_buffers(master, states);
}


void Connections_clear_buffers(
        Connections* graph, Device_states* states, uint32_t start, uint32_t until)
{
    assert(graph != NULL);
    assert(states != NULL);

    Device_node* master = AAtree_get_exact(graph->nodes, "");
    assert(master != NULL);
    if (start >= until)
        return;

    Device_node_reset(master);
    Device_node_clear_buffers(master, states, start, until);

    return;
}


void Connections_process_voice_group(
        Connections* graph,
        Voice_group* vgroup,
        Device_states* dstates,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(graph != NULL);
    assert(vgroup != NULL);
    assert(dstates != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(audio_rate > 0);
    assert(tempo > 0);

    Device_node* master = AAtree_get_exact(graph->nodes, "");
    assert(master != NULL);
    if (buf_start >= buf_stop)
        return;

    Device_node_reset(master);
    Device_node_process_voice_group(
            master, vgroup, dstates, wbs, buf_start, buf_stop, audio_rate, tempo);

    return;
}


void Connections_mix(
        Connections* graph,
        Device_states* states,
        const Work_buffers* wbs,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(graph != NULL);
    assert(states != NULL);
    assert(wbs != NULL);
    assert(freq > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Device_node* master = AAtree_get_exact(graph->nodes, "");
    assert(master != NULL);
    if (start >= until)
        return;

#if 0
    static bool called = false;
    if (!called)
    {
        Connections_print(graph, stderr);
    }
    called = true;
//    fprintf(stderr, "Mix process:\n");
#endif

    Device_node_reset(master);
    Device_node_mix(master, states, wbs, start, until, freq, tempo);

    return;
}


static void Connections_reset(Connections* graph)
{
    assert(graph != NULL);

    const char* name = "";
    Device_node* node = AAiter_get_at_least(graph->iter, name);
    while (node != NULL)
    {
        Device_node_set_state(node, DEVICE_NODE_STATE_NEW);
        node = AAiter_get_next(graph->iter);
    }

    return;
}


static bool Connections_is_cyclic(Connections* graph)
{
    assert(graph != NULL);

    Connections_reset(graph);
    const char* name = "";
    Device_node* node = AAiter_get_at_least(graph->iter, name);
    while (node != NULL)
    {
        assert(Device_node_get_state(node) != DEVICE_NODE_STATE_REACHED);
        if (Device_node_cycle_in_path(node))
            return true;

        node = AAiter_get_next(graph->iter);
    }
    return false;
}


void Connections_init_processor_voice_cut_settings(Connections* graph)
{
    assert(graph != NULL);

    // Clear all voice cut settings
    {
        AAiter* iter = AAITER_AUTO;
        AAiter_change_tree(iter, graph->nodes);

        Device_node* node = AAiter_get_at_least(iter, "");
        while (node != NULL)
        {
            Device_node_clear_processor_voice_cut_settings(node);
            node = AAiter_get_next(iter);
        }
    }

    // Enable voice cut for all devices that need it
    {
        AAiter* iter = AAITER_AUTO;
        AAiter_change_tree(iter, graph->nodes);

        Device_node* node = AAiter_get_at_least(iter, "");
        while (node != NULL)
        {
            Device_node_init_processor_voice_cut_settings(node);
            node = AAiter_get_next(iter);
        }
    }

    return;
}


void Connections_print(Connections* graph, FILE* out)
{
    assert(graph != NULL);
    assert(out != NULL);

//    Connections_reset(graph);
    Device_node* master = AAtree_get_exact(graph->nodes, "");
    assert(master != NULL);
    Device_node_print(master, out);
    fprintf(out, "\n");

    return;
}


void del_Connections(Connections* graph)
{
    if (graph == NULL)
        return;

    del_AAiter(graph->iter);
    del_AAtree(graph->nodes);
    memory_free(graph);

    return;
}


static int read_index(char* str)
{
    assert(str != NULL);

    static const char* hex_digits = "0123456789abcdef";
    if (strspn(str, hex_digits) != 2)
        return INT_MAX;

    int res = (strchr(hex_digits, str[0]) - hex_digits) * 0x10;

    return res + (strchr(hex_digits, str[1]) - hex_digits);
}


static int validate_connection_path(
        Streader* sr, char* str, Connection_level level, Device_port_type type)
{
    assert(sr != NULL);
    assert(str != NULL);
    assert(type < DEVICE_PORT_TYPES);

    if (Streader_is_error_set(sr))
        return -1;

    bool root = true;
    char* path = str;
    char* trim_point = str;

    // Device
    if (string_has_prefix(str, "au_"))
    {
        // TODO: disallow audio unit in more than 2 levels
        if ((level != CONNECTION_LEVEL_GLOBAL) && (level != CONNECTION_LEVEL_AU))
        {
            Streader_set_error(
                    sr,
                    "Audio unit directory in a deep-level connection: \"%s\"",
                    path);
            return -1;
        }

        root = false;
        str += strlen("au_");
        if (read_index(str) >= KQT_AUDIO_UNITS_MAX)
        {
            Streader_set_error(
                    sr,
                    "Invalid audio unit number in the connection: \"%s\"",
                    path);
            return -1;
        }

        str += 2;
        if (!string_has_prefix(str, "/"))
        {
            Streader_set_error(
                    sr,
                    "Missing trailing '/' after the audio unit number"
                        " in the connection: \"%s\"",
                    path);
            return -1;
        }

        ++str;
        trim_point = str - 1;
    }
    else if (string_has_prefix(str, "proc_"))
    {
        if (level != CONNECTION_LEVEL_AU)
        {
            Streader_set_error(
                    sr,
                    "Processor directory in a root-level connection: \"%s\"",
                    path);
            return -1;
        }

        root = false;
        str += strlen("proc_");
        if (read_index(str) >= KQT_PROCESSORS_MAX)
        {
            Streader_set_error(
                    sr,
                    "Invalid processor number in the connection: \"%s\"",
                    path);
            return -1;
        }

        str += 2;
        if (!string_has_prefix(str, "/"))
        {
            Streader_set_error(
                    sr,
                    "Missing trailing '/' after the processor number"
                        " in the connection: \"%s\"",
                    path);
            return -1;
        }

        ++str;
        if (!string_has_prefix(str, "C/"))
        {
            Streader_set_error(
                    sr,
                    "Invalid processor parameter directory"
                        " in the connection: \"%s\"",
                    path);
            return -1;
        }

        str += strlen("C/");
        trim_point = str - 1;
    }

    // Port
    if (string_has_prefix(str, "in_") || string_has_prefix(str, "out_"))
    {
        if (string_has_prefix(str, "in_") &&
                root &&
                (level != CONNECTION_LEVEL_AU))
        {
            Streader_set_error(
                    sr, "Input ports are not allowed for master: \"%s\"", path);
            return -1;
        }

        if (type == DEVICE_PORT_TYPE_RECEIVE)
        {
            bool can_receive = (!root && string_has_prefix(str, "in_")) ||
                               (root && string_has_prefix(str, "out_"));
            if (!can_receive)
            {
                Streader_set_error(
                        sr,
                        "Destination port is not for receiving data: \"%s\"",
                        path);
                return -1;
            }
        }
        else
        {
            assert(type == DEVICE_PORT_TYPE_SEND);
            bool can_send = (string_has_prefix(str, "out_") && !root) ||
                            (string_has_prefix(str, "in_") && root);
            if (!can_send)
            {
                Streader_set_error(
                        sr,
                        "Source port is not for sending data: \"%s\"",
                        path);
                return -1;
            }
        }

        str += strcspn(str, "_") + 1;
        int port = read_index(str);
        if (port >= KQT_DEVICE_PORTS_MAX)
        {
            Streader_set_error(sr, "Invalid port number: \"%s\"", path);
            return -1;
        }

        str += 2;
        if (str[0] != '/' && str[0] != '\0' && str[1] != '\0')
        {
            Streader_set_error(
                    sr,
                    "Connection path contains garbage"
                        " after the port specification: \"%s\"",
                    path);
            return -1;
        }

        *trim_point = '\0';
        return port;
    }

    Streader_set_error(sr, "Invalid connection: \"%s\"", path);

    return -1;
}


