

#ifndef K_VOICE_H
#define K_VOICE_H


#include <stdint.h>

#include <Event_queue.h>
#include <Instrument.h>
#include <Voice_state.h>


typedef enum
{
	VOICE_PRIO_INACTIVE = 0,
	VOICE_PRIO_BG,
	VOICE_PRIO_FG
} Voice_prio;


typedef struct Voice
{
	uint16_t pool_index;   ///< Storage position in the Voice pool.
	uint64_t id;           ///< An identification number for this initialisation.
	Voice_prio prio;       ///< Current priority of the Voice.
	Event_queue* events;   ///< Upcoming events.
	Instrument* ins;       ///< The Instrument played.
	Voice_state state;     ///< The current playback state.
} Voice;


/**
 * Creates a new Voice.
 *
 * \param events   The maximum number of events per tick -- must be > \c 0.
 *
 * \return   The new Voice if successful, or \c NULL if memory allocation
 *           failed.
 */
Voice* new_Voice(uint8_t events);


/**
 * Compares priorities of two Voices.
 *
 * \param v1   The first Voice -- must not be \c NULL.
 * \param v2   The second Voice -- must not be \c NULL.
 *
 * \return   An integer less than, equal to or greater than zero if \a v1 is
 *           found, respectively, to be lower than, equal to or greater than
 *           \a v2 in priority.
 */
int Voice_cmp(Voice* v1, Voice* v2);


/**
 * Sets the Instrument to be used by the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 * \param ins     The Instrument -- must not be \c NULL.
 */
void Voice_set_instrument(Voice* voice, Instrument* ins);


/**
 * Adds a new Event into the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 * \param event   The Event -- must not be \c NULL.
 * \param pos     The position of the Event.
 *
 * \return   \c true if successful, or \c false if the Event queue is full.
 */
bool Voice_add_event(Voice* voice, Event* event, uint32_t pos);


/**
 * Mixes the Voice.
 *
 * \param voice    The Voice -- must not be \c NULL.
 * \param amount   The number of frames to be mixed.
 * \param offset   The buffer offset.
 * \param freq     The mixing frequency -- must be > \c 0.
 */
void Voice_mix(Voice* voice,
		uint32_t amount,
		uint32_t offset,
		uint32_t freq);


/**
 * Destroys an existing Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 */
void del_Voice(Voice* voice);


#endif // K_VOICE_H


