

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


START_TEST(Validation_rejects_empty_songs)
{
    set_silent_composition();
    validate();

    set_data("song_00/p_order_list.json", "[]");
    set_data("pat_000/p_manifest.json", "");

    kqt_Handle_validate(handle);

    check_validation_error("song", "Handle accepts a song without systems");
}
END_TEST


typedef enum
{
    TEST_SONG_FIRST,
    TEST_SONG_SECOND,
    TEST_SONG_PENULTIMATE,
    TEST_SONG_LAST,
    TEST_SONG_COUNT
} Test_song;

static const int test_songs[] =
{
    [TEST_SONG_FIRST]       = 0,
    [TEST_SONG_SECOND]      = 1,
    [TEST_SONG_PENULTIMATE] = KQT_SONGS_MAX - 2,
    [TEST_SONG_LAST]        = KQT_SONGS_MAX - 1,
};


typedef enum
{
    TEST_PAT_FIRST,
    TEST_PAT_SECOND,
    TEST_PAT_PENULTIMATE,
    TEST_PAT_LAST,
    TEST_PAT_COUNT
} Test_pat;

static const int test_pats[] =
{
    [TEST_PAT_FIRST]       = 0,
    [TEST_PAT_SECOND]      = 1,
    [TEST_PAT_PENULTIMATE] = KQT_PATTERNS_MAX - 2,
    [TEST_PAT_LAST]        = KQT_PATTERNS_MAX - 1,
};


typedef enum
{
    TEST_PAT_INST_FIRST,
    TEST_PAT_INST_SECOND,
    TEST_PAT_INST_PENULTIMATE,
    TEST_PAT_INST_LAST,
    TEST_PAT_INST_COUNT
} Test_pat_inst;

static const int test_pat_insts[] =
{
    [TEST_PAT_INST_FIRST]       = 0,
    [TEST_PAT_INST_SECOND]      = 1,
    [TEST_PAT_INST_PENULTIMATE] = KQT_PAT_INSTANCES_MAX - 2,
    [TEST_PAT_INST_LAST]        = KQT_PAT_INSTANCES_MAX - 1,
};


START_TEST(Validation_rejects_orphan_songs)
{
    set_silent_composition();
    validate();

    const int orphan_index = test_songs[_i];

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


START_TEST(Validation_rejects_nonexistent_songs_in_album)
{
    set_silent_composition();
    validate();

    set_data("song_00/p_manifest.json", "");

    const int missing_index = test_songs[_i];

    char track_list[16] = "";
    snprintf(track_list, 16, "[%d]", missing_index);
    set_data("album/p_tracks.json", track_list);

    kqt_Handle_validate(handle);

    check_validation_error("song",
            "Handle accepts an album with nonexistent song %d", missing_index);
}
END_TEST


START_TEST(Validation_rejects_patterns_without_instances)
{
    set_silent_composition();
    validate();

    const int invalid_index = test_pats[_i];

    if (invalid_index == 0)
    {
        // Set another valid pattern
        set_data("song_00/p_order_list.json", "[ [1, 0] ]");
        set_data("pat_001/p_manifest.json", "{}");
        set_data("pat_001/p_pattern.json", "{ \"length\": [16, 0] }");
        set_data("pat_001/instance_000/p_manifest.json", "{}");
    }

    // Set a new pattern without instances
    char invalid_prefix[16] = "";
    snprintf(invalid_prefix, 16, "pat_%03x", invalid_index);

    char invalid_manifest[64] = "";
    snprintf(invalid_manifest, 64, "%s/p_manifest.json", invalid_prefix);
    set_data(invalid_manifest, "{}");

    char invalid_pattern[64] = "";
    snprintf(invalid_pattern, 64, "%s/p_pattern.json", invalid_prefix);
    set_data(invalid_pattern, "{ \"length\": [16, 0] }");

    char invalid_inst[64] = "";
    snprintf(invalid_inst, 64, "%s/instance_000/p_manifest.json", invalid_prefix);
    set_data(invalid_inst, "");

    kqt_Handle_validate(handle);

    check_validation_error("instance",
            "Handle accepts pattern %d without instance", invalid_index);
}
END_TEST


START_TEST(Validation_rejects_orphan_pattern_instances)
{
    set_silent_composition();
    validate();

    const int orphan_index = test_pat_insts[_i];

    if (orphan_index == 0)
    {
        // Set another valid pattern instance
        set_data("song_00/p_order_list.json", "[ [0, 1] ]");
        set_data("pat_000/instance_001/p_manifest.json", "{}");
    }

    // Set a new pattern instance
    char orphan_manifest[64] = "";
    snprintf(orphan_manifest, 64, "pat_000/instance_%03x/p_manifest.json",
            orphan_index);
    set_data(orphan_manifest, "{}");

    kqt_Handle_validate(handle);

    check_validation_error("instance",
            "Handle accepts an orphan pattern instance at index %d",
            orphan_index);
}
END_TEST


START_TEST(Validation_rejects_nonexistent_pattern_instances_in_songs)
{
    set_silent_composition();
    validate();

    set_data("song_00/p_order_list.json", "[ [0, 0], [0, 1] ]");

    kqt_Handle_validate(handle);

    check_validation_error("instance",
            "Handle accepts an album with nonexistent pattern instance");
}
END_TEST


START_TEST(Validation_rejects_reused_pattern_instances_in_song)
{
    set_silent_composition();
    validate();

    set_data("song_00/p_order_list.json", "[ [0, 0], [0, 0] ]");

    kqt_Handle_validate(handle);

    check_validation_error("instance",
            "Handle accepts reused pattern instances in a song");
}
END_TEST


START_TEST(Validation_rejects_shared_pattern_instances_between_songs)
{
    set_silent_composition();
    validate();

    set_data("album/p_tracks.json", "[0, 1]");
    set_data("song_01/p_manifest.json", "{}");
    set_data("song_01/p_order_list.json", "[ [0, 0] ]");

    kqt_Handle_validate(handle);

    check_validation_error("instance",
            "Handle accepts shared pattern instances between songs");
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
    tcase_add_test(tc_reject, Validation_rejects_empty_songs);
    tcase_add_loop_test(tc_reject, Validation_rejects_orphan_songs,
            0, TEST_SONG_COUNT);
    tcase_add_loop_test(tc_reject, Validation_rejects_nonexistent_songs_in_album,
            0, TEST_SONG_COUNT);
    tcase_add_loop_test(tc_reject, Validation_rejects_patterns_without_instances,
            0, TEST_PAT_COUNT);
    tcase_add_loop_test(tc_reject, Validation_rejects_orphan_pattern_instances,
            0, TEST_PAT_INST_COUNT);
    tcase_add_test(tc_reject,
            Validation_rejects_nonexistent_pattern_instances_in_songs);
    tcase_add_test(tc_reject,
            Validation_rejects_reused_pattern_instances_in_song);
    tcase_add_test(tc_reject,
            Validation_rejects_shared_pattern_instances_between_songs);

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


