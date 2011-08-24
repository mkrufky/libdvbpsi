/*****************************************************************************
 * dvbinfo.c: DVB PSI Information
 *****************************************************************************
 * Copyright (C) 2010-2011 M2X BV
 *
 * Authors: Jean-Paul Saman <jpsaman@videolan.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#   include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#   include <stdint.h>
#endif

#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_SOCKET_H
#   include <netdb.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#endif

#include <assert.h>

#include "dvbinfo.h"
#include "libdvbpsi.h"
#include "buffer.h"

#ifdef HAVE_SYS_SOCKET_H
#   include "udp.h"
#   include "tcp.h"
#endif

/*****************************************************************************
 *
 *****************************************************************************/
typedef struct dvbinfo_capture_s
{
    fifo_t  *fifo;
    fifo_t  *empty;

    size_t   size;  /* prefered capture size */

    const params_t *params;
    bool            b_alive;
} dvbinfo_capture_t;

/*****************************************************************************
 * Usage
 *****************************************************************************/
static void usage(void)
{
#ifdef HAVE_SYS_SOCKET_H
    printf("Usage: dvbinfo [-h] [-d <debug>] [-f| [[-u|-t] -i <ipaddress:port>] -o <outputfile>\n");
#else
    printf("Usage: dvbinfo [-h] [-d <debug>] [-f|\n");
#endif
    printf(" -d : debug level (default:none, error, warn, debug)\n");
    printf(" -f : filename\n");
#ifdef HAVE_SYS_SOCKET_H
    printf(" -i : hostname or ipaddress\n");
    printf(" -u : udp network transport\n");
    printf(" -t : tcp network transport\n");
    printf(" -o : output to filename\n");
#endif
    printf(" -h : help information\n");
    exit(EXIT_FAILURE);
}

/* */
static params_t *params_init(void)
{
    params_t *param;
    param = (params_t *) calloc(1, sizeof(params_t));
    if (param == NULL)
    {
        fprintf(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    }

    param->fd_in = param->fd_out = -1;
    param->input = NULL;
    param->output = NULL;
    param->debug = 0;
    param->pf_read = NULL;
    param->pf_write = NULL;

    return param;
}

static void params_free(params_t *param)
{
    free(param->input);
    free(param->output);
}

/* */
static void dvbinfo_close(params_t *param)
{
#ifdef HAVE_SYS_SOCKET_H
    if (param->input && param->b_udp)
        udp_close(param->fd_in);
    else if (param->input && param->b_tcp)
        tcp_close(param->fd_in);
    else
#endif
    if (param->input)
        close(param->fd_in);
    if (param->output)
        close(param->fd_out);
}

static void dvbinfo_open(params_t *param)
{
#ifdef HAVE_SYS_SOCKET_H
    if (param->output)
    {
        param->fd_out = open(param->output, O_CREAT | O_RDWR | O_NONBLOCK
                             | O_EXCL | O_CLOEXEC, S_IRWXU);
        if (param->fd_out < 0)
            goto error;
    }
    if (param->input && param->b_udp)
    {
        param->fd_in = udp_open(param->input, param->port);
        if (param->fd_in < 0)
            goto error;
    }
    else if (param->input && param->b_tcp)
    {
        param->fd_in = tcp_open(param->input, param->port);
        if (param->fd_in < 0)
            goto error;
    }
    else
#endif
    if (param->input)
    {
        param->fd_in = open(param->input, O_RDONLY | O_NONBLOCK);
        if (param->fd_in < 0)
            goto error;
    }
    return;

error:
    dvbinfo_close(param);
    params_free(param);
    exit(EXIT_FAILURE);
}

static void *dvbinfo_capture(void *data)
{
    dvbinfo_capture_t *capture = (dvbinfo_capture_t *)data;
    const params_t *param = capture->params;
    bool b_eof = false;

    while (capture->b_alive && !b_eof)
    {
        buffer_t *buffer;

        if (fifo_count(capture->empty) == 0)
            buffer = buffer_new(capture->size);
        else
            buffer = fifo_pop(capture->empty);

        if (buffer == NULL) /* out of memory */
            break;

        ssize_t size = param->pf_read(param->fd_in, buffer->p_data, buffer->i_size);
        if (size < 0) /* short read ? */
        {
            fifo_push(capture->empty, buffer);
            continue;
        }
        else if (size == 0)
        {
            fifo_push(capture->empty, buffer);
            b_eof = true;
            continue;
        }

        buffer->i_date = mdate();

        /* store buffer */
        fifo_push(capture->fifo, buffer);
        buffer = NULL;
    }

    capture->b_alive = false;
    fifo_wake(capture->fifo);
    return NULL;
}

static void dvbinfo_process(dvbinfo_capture_t *capture)
{
    bool b_error = false;
    const params_t *param = capture->params;
    buffer_t *buffer = NULL;

    ts_stream_t *stream = libdvbpsi_init(param->debug);
    if (!stream)
    {
        fprintf(stderr, "error: out of memory\n");
        exit(EXIT_FAILURE);
    }

    while (!b_error && capture->b_alive)
    {
        /* Wait for data to arrive */
        buffer = fifo_pop(capture->fifo);
        if (buffer == NULL)
            break;

        if (param->output)
        {
            size_t size = param->pf_write(param->fd_out, buffer->p_data, buffer->i_size);
            if (size < 0) /* error writing */
            {
                fprintf(stderr, "error (%d) writting to %s", errno, param->output);
                break;
            }
            else if (size < buffer->i_size) /* short writting disk full? */
            {
                fprintf(stderr, "error writting to %s (disk full?)", param->output);
                break;
            }
        }

        if (!libdvbpsi_process(stream, buffer->p_data, buffer->i_size, buffer->i_date))
            b_error = true;

        /* reuse buffer */
        fifo_push(capture->empty, buffer);
        buffer = NULL;
    }

    libdvbpsi_exit(stream);

    if (b_error)
        fprintf(stderr, "error while processing\n" );

    buffer_free(buffer);
}

/*
 * DVB Info main application
 */
int main(int argc, char **pp_argv)
{
    dvbinfo_capture_t capture;
    params_t *parm = NULL;
    char c;

    printf("dvbinfo: Copyright (C) 2011 M2X BV\n");
    printf("License: LGPL v2.1\n");

    if (argc == 1)
        usage();

    parm = params_init();
    capture.params = parm;
    capture.fifo = fifo_new();
    capture.empty = fifo_new();

    static const struct option long_options[] =
    {
        { "debug",     required_argument, NULL, 'd' },
        { "file",      required_argument, NULL, 'f' },
#ifdef HAVE_SYS_SOCKET_H
        { "ipaddress", required_argument, NULL, 'i' },
        { "tcp",       no_argument,       NULL, 't' },
        { "udp",       no_argument,       NULL, 'u' },
        { "output",    required_argument, NULL, 'o' },
#endif
        { "help",      no_argument,       NULL, 'h' },
        { 0, 0, 0, 0 }
    };
#ifdef HAVE_SYS_SOCKET_H
    while ((c = getopt_long(argc, pp_argv, "d:f:i:ho:tu", long_options, NULL)) != -1)
#else
    while ((c = getopt_long(argc, pp_argv, "d:f:h", long_options, NULL)) != -1)
#endif
    {
        switch(c)
        {
            case 'd':
                if (optarg)
                {
                    parm->debug = 0;
                    if (strncmp(optarg, "error", 5) == 0)
                        parm->debug = 1;
                    else if (strncmp(optarg, "warn", 4) == 0)
                        parm->debug = 2;
                    else if (strncmp(optarg, "debug", 5) == 0)
                        parm->debug = 3;
                }
                break;

            case 'f':
                if (optarg)
                {
                    if (asprintf(&parm->input, "%s", optarg) < 0)
                    {
                        fprintf(stderr, "error: out of memory\n");
                        usage();
                    }
                    /* */
                    parm->pf_read = read;
                }
                break;

#ifdef HAVE_SYS_SOCKET_H
            case 'i':
                if (optarg)
                {
                    char *psz_tmp = strtok(optarg,":");
                    if (psz_tmp)
                    {
                        size_t len = strlen(psz_tmp);
                        parm->port = strtol(&optarg[len+1], NULL, 0);
                        parm->input = strdup(psz_tmp);
                    }
                    else usage();
                }
                break;

            case 'o':
                if (optarg)
                {
                    if (asprintf(&parm->output, "%s", optarg) < 0)
                    {
                        fprintf(stderr, "error: out of memory\n");
                        usage();
                    }
                    /* */
                    parm->pf_write = write;
                }
                break;

            case 't':
                parm->b_tcp = true;
                parm->pf_read = tcp_read;
                break;

            case 'u':
                parm->b_udp = true;
                parm->pf_read = udp_read;
                break;
#endif

            case '?':
               fprintf(stderr, "Unknown option %c found\n", c);
               exit(EXIT_FAILURE);
               break;

            case 'h':
            default:
                usage();
                break;
        }
    };

    if (parm->input == NULL)
    {
        fprintf(stderr, "No source given\n");
        params_free(parm);
        usage(); /* exits application */
    }

#ifdef HAVE_SYS_SOCKET_H
    if (parm->b_udp || parm->b_tcp)
    {
        capture.size = 7*188;
        printf("Listen: host=%s port=%d\n", parm->input, parm->port);
    }
    else
#endif
    {
        capture.size = 188;
        printf("Examining: %s\n", parm->input);
    }

    /* */
    dvbinfo_open(parm);
    pthread_t handle;
    capture.b_alive = true;
    if (pthread_create(&handle, NULL, dvbinfo_capture, (void *)&capture) < 0)
    {
        fprintf(stderr, "failed creating thread\n");
        dvbinfo_close(parm);
        exit(EXIT_FAILURE);
    }
    dvbinfo_process(&capture);
    capture.b_alive = false;     /* stop thread */
    if (pthread_join(handle, NULL) < 0)
        fprintf(stderr, "error joining capture thread\n");
    dvbinfo_close(parm);

    /* cleanup */
    params_free(parm);

    fifo_wake((&capture)->fifo);
    fifo_wake((&capture)->empty);

    fifo_free((&capture)->fifo);
    fifo_free((&capture)->empty);

    exit(EXIT_SUCCESS);
}
