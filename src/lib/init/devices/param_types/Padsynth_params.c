

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


#include <init/devices/param_types/Padsynth_params.h>

#include <containers/Vector.h>
#include <mathnum/common.h>
#include <memory.h>
#include <string/common.h>
#include <string/Streader.h>

#include <stdlib.h>


static bool read_harmonic(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    ignore(index);
    assert(userdata != NULL);

    Vector* harmonics = userdata;

    Padsynth_harmonic* info =
        &(Padsynth_harmonic){ .freq_mul = NAN, .amplitude = NAN };
    if (!Streader_readf(sr, "[%f,%f]", &info->freq_mul, &info->amplitude))
        return false;

    if (info->freq_mul <= 0)
    {
        Streader_set_error(
                sr, "PADsynth harmonic frequency multiplier must be positive");
        return false;
    }

    if (info->amplitude < 0)
    {
        Streader_set_error(sr, "PADsynth harmonic amplitude must be non-negative");
        return false;
    }

    if (!Vector_append(harmonics, info))
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for PADsynth harmonics");
        return false;
    }

    return true;
}


static bool read_param(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    Padsynth_params* pp = userdata;

    if (string_eq(key, "sample_length"))
    {
        int64_t length = -1;
        if (!Streader_read_int(sr, &length))
            return false;

        if (!(PADSYNTH_MIN_SAMPLE_LENGTH <= length &&
                    length <= PADSYNTH_MAX_SAMPLE_LENGTH))
        {
            Streader_set_error(
                    sr,
                    "PADsynth sample length must be a power of 2"
                        " within range [%ld, %ld]",
                    PADSYNTH_MIN_SAMPLE_LENGTH,
                    PADSYNTH_MAX_SAMPLE_LENGTH);
            return false;
        }

        if (!is_p2(length))
        {
            Streader_set_error(sr, "PADsynth sample length must be a power of 2");
            return false;
        }

        pp->sample_length = length;
    }
    else if (string_eq(key, "audio_rate"))
    {
        int64_t rate = -1;
        if (!Streader_read_int(sr, &rate))
            return false;

        if (!(0 < rate && rate <= 1048576))
        {
            Streader_set_error(sr, "PADsynth sample rate must be positive"
                    " and not greater than 1048576");
            return false;
        }

        pp->audio_rate = rate;
    }
    else if (string_eq(key, "sample_count"))
    {
        int64_t sample_count = 0;
        if (!Streader_read_int(sr, &sample_count))
            return false;

        if (!(0 < sample_count && sample_count <= 128))
        {
            Streader_set_error(
                    sr, "PADsynth sample count must be within range [1, 128]");
            return false;
        }

        pp->sample_count = sample_count;
    }
    else if (string_eq(key, "pitch_range"))
    {
        double min_pitch = NAN;
        double max_pitch = NAN;
        if (!Streader_readf(sr, "[%f,%f]", &min_pitch, &max_pitch))
            return false;

        if (min_pitch > max_pitch)
        {
            Streader_set_error(
                    sr,
                    "PADsynth minimum sample map pitch must not be greater"
                        " than the maximum pitch");
            return false;
        }

        pp->min_pitch = min_pitch;
        pp->max_pitch = max_pitch;
    }
    else if (string_eq(key, "center_pitch"))
    {
        double center_pitch = NAN;
        if (!Streader_read_float(sr, &center_pitch))
            return false;

        pp->center_pitch = center_pitch;
    }
    else if (string_eq(key, "bandwidth_base"))
    {
        double base = NAN;
        if (!Streader_read_float(sr, &base))
            return false;

        if (base <= 0)
        {
            Streader_set_error(sr, "PADsynth bandwidth base must be positive");
            return false;
        }

        pp->bandwidth_base = base;
    }
    else if (string_eq(key, "bandwidth_scale"))
    {
        double scale = NAN;
        if (!Streader_read_float(sr, &scale))
            return false;

        if (scale < 1)
        {
            Streader_set_error(sr, "PADsynth bandwidth scale must be at least 1");
            return false;
        }

        pp->bandwidth_scale = scale;
    }
    else if (string_eq(key, "harmonics"))
    {
        if (!Streader_read_list(sr, read_harmonic, pp->harmonics))
            return false;

        if (Vector_size(pp->harmonics) == 0)
        {
            Streader_set_error(sr, "List of PADsynth harmonics is empty");
            return false;
        }
    }
    else
    {
        Streader_set_error(
                sr, "Unrecognised key in PADsynth parameters: %s", key);
        return false;
    }

    return true;
}


Padsynth_params* new_Padsynth_params(Streader* sr)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Padsynth_params* pp = memory_alloc_item(Padsynth_params);
    if (pp == NULL)
        return NULL;

    pp->harmonics = new_Vector(sizeof(Padsynth_harmonic));
    if (pp->harmonics == NULL)
    {
        del_Padsynth_params(pp);
        return NULL;
    }

    pp->sample_length = PADSYNTH_DEFAULT_SAMPLE_LENGTH;
    pp->audio_rate = PADSYNTH_DEFAULT_AUDIO_RATE;
    pp->sample_count = 1;
    pp->min_pitch = 0;
    pp->max_pitch = 0;
    pp->center_pitch = 0;

    pp->bandwidth_base = PADSYNTH_DEFAULT_BANDWIDTH_BASE;
    pp->bandwidth_scale = PADSYNTH_DEFAULT_BANDWIDTH_SCALE;

    if (!Streader_read_dict(sr, read_param, pp))
    {
        del_Padsynth_params(pp);
        return NULL;
    }

    if (Vector_size(pp->harmonics) == 0)
    {
        Streader_set_error(sr, "No harmonics found in PADsynth parameters");
        del_Padsynth_params(pp);
        return NULL;
    }

    return pp;
}


void del_Padsynth_params(Padsynth_params* pp)
{
    if (pp == NULL)
        return;

    del_Vector(pp->harmonics);
    memory_free(pp);

    return;
}


