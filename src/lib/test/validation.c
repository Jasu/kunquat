

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include <handle_utils.h>
#include <test_common.h>

#include <kunquat/Handle.h>
#include <kunquat/testing.h>


// Case-sensitive substring search
bool string_contains(const char* haystack, const char* needle)
{
    return strstr(haystack, needle) != NULL;
}


// Case-insensitive substring search
bool string_contains_word(const char* haystack, const char* needle)
{
    if (needle == NULL)
        return true;
    if (haystack == NULL)
        return false;

    const char* hp = haystack;

    // Iterate over possible substring start positions
    while (*hp != '\0')
    {
        bool found = true;
        for (int i = 0; needle[i] != '\0'; ++i)
        {
            if (tolower(hp[i]) != tolower(needle[i]))
            {
                found = false;
                break;
            }
        }
        if (found)
            return true;

        ++hp;
    }

    return false;
}


static void set_silent_composition()
{
    set_data("album/p_manifest.json", "{}");
    set_data("album/p_tracks.json", "[0]");
    set_data("song_00/p_manifest.json", "{}");
    set_data("song_00/p_order_list.json", "[ [0, 0] ]");
    set_data("pat_000/p_manifest.json", "{}");
    set_data("pat_000/p_pattern.json", "{ \"length\": [16, 0] }");
    set_data("pat_000/instance_000/p_manifest.json", "{}");

    return;
}


START_TEST(Handle_refuses_to_render_unvalidated_module)
{
    set_silent_composition();

    long mixed = kqt_Handle_mix(handle, 16);

    fail_if(mixed > 0,
            "kqt_Handle_mix rendered %ld frames of unvalidated music",
            mixed);

    const char* error_msg = kqt_Handle_get_error(handle);
    fail_if(strlen(error_msg) == 0,
            "Attempting to render unvalidated music did not give an error"
            " message");
    fail_unless(string_contains(error_msg, "\"ArgumentError\""),
            "Error on missing validation was not an ArgumentError");
    fail_unless(string_contains_word(error_msg, "valid"),
            "Error message on missing validation does not contain word"
            " \"valid\"");
}
END_TEST


#define check_validation_error(context_str, ...)                    \
    if (true)                                                       \
    {                                                               \
        const char* error_msg = kqt_Handle_get_error(handle);       \
        fail_if(strlen(error_msg) == 0, __VA_ARGS__);               \
        fail_unless(string_contains(error_msg, "\"FormatError\""),  \
                "Validation error is not a FormatError");           \
        fail_unless(string_contains_word(error_msg, context_str),   \
                "Validation error message does not mention \"%s\"", \
                context_str);                                       \
    } else (void)0


START_TEST(Validation_rejects_album_without_tracks)
{
    set_silent_composition();
    validate();

    set_data("album/p_tracks.json", "[]");

    kqt_Handle_validate(handle);

    check_validation_error("album", "Handle accepts an album without tracks");
}
END_TEST


START_TEST(Validation_rejects_orphan_songs)
{
    set_silent_composition();
    validate();

    const int orphan_index = _i;

    if (orphan_index == 0)
    {
        // Set another valid song
        set_data("album/p_tracks.json", "[1]");
        set_data("song_01/p_manifest.json", "{}");
        set_data("song_01/p_order_list.json", "[ [0, 0] ]");
    }

    // Set orphan song
    char orphan_manifest[64] = "";
    snprintf(orphan_manifest, 64, "song_%02x/p_manifest.json", orphan_index);
    set_data(orphan_manifest, "{}");

    char orphan_order_list[64] = "";
    snprintf(orphan_order_list, 64, "song_%02x/p_order_list.json", orphan_index);
    set_data(orphan_order_list, "[ [0, 0] ]");

    kqt_Handle_validate(handle);

    check_validation_error("song",
            "Handle accepts an orphan song at index %d",
            orphan_index);
}
END_TEST


Suite* Validation_suite(void)
{
    Suite* s = suite_create("Validation");

    const int timeout = 4;

    TCase* tc_refuse = tcase_create("refuse");
    TCase* tc_reject = tcase_create("reject");

    suite_add_tcase(s, tc_refuse);
    suite_add_tcase(s, tc_reject);

    tcase_set_timeout(tc_refuse, timeout);
    tcase_set_timeout(tc_reject, timeout);

    tcase_add_checked_fixture(tc_refuse, setup_empty, handle_teardown);
    tcase_add_checked_fixture(tc_reject, setup_empty, handle_teardown);

    tcase_add_test(tc_refuse, Handle_refuses_to_render_unvalidated_module);

    tcase_add_test(tc_reject, Validation_rejects_album_without_tracks);
    tcase_add_loop_test(tc_reject, Validation_rejects_orphan_songs,
            0, KQT_SONGS_MAX);

    return s;
}


int main(void)
{
    Suite* suite = Validation_suite();
    SRunner* sr = srunner_create(suite);
#ifdef K_MEM_DEBUG
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    int fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit(fail_count > 0);
}


