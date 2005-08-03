/*****************************************************************************
 * connect.h: Routines for establishing a network connection.
 *----------------------------------------------------------------------------
 * Copyright (c) 2005 M2X
 * $Id: connect.c 104 2005-03-21 13:38:56Z massiot $
 *
 * Authors: Jean-Paul Saman <jpsaman #_at_# m2x dot nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#if !defined(_CONNECT_H_)
#define _CONNECT_H_ 1

int create_tcp_connection( const char *ipaddress, int port );
int create_udp_connection( const char *ipaddress, int port );

int close_connection( int socket_fd );
#define close_tcp_connection    close_connection
#define close_udp_connection    close_connection

#endif

