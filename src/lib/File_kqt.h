

/*
 * Copyright 2010 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_FILE_KQT_H
#define K_FILE_KQT_H


#include <stdbool.h>

#include <Handle_r.h>


/**
 * Opens a .kqt file inside the Kunquat Handle.
 *
 * \param handle_r   The read-only Kunquat Handle -- must not be \c NULL.
 * \param path       The path of the .kqt file -- must not be \c NULL.
 *
 * \return   \c true if successful. Otherwise \c false is returned and the
 *           handle error is set to describe the error.
 */
bool File_kqt_open(Handle_r* handle_r, const char* path);


#endif // K_FILE_KQT_H


