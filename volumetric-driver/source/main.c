///////////////////////////////////////////////////////////////////////////////
// NAME:            main.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the application
//
// CREATED:         01/09/2022
//
// LAST EDITED:     01/09/2022
//
// Copyright 2022, Ethan D. Twardy
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////

#include <stdbool.h>
#include <stdio.h>

#include <glib.h>
#include <glib-unix.h>
#include <libsoup/soup.h>

const char* argp_program_name = "volumetric-driver";
const int CONFIG_WEB_SERVER_PORT = 8888;

void handle_connection(SoupServer* server, SoupServerMessage* message,
    const char* path, GHashTable* query, gpointer user_data)
{
    g_info("Got request!");
    soup_server_message_set_status(message, SOUP_STATUS_OK, NULL);
    static const char* RESPONSE_BODY = "Hello, soup!\n";
    soup_server_message_set_response(message, "text/html", SOUP_MEMORY_COPY,
        RESPONSE_BODY, strlen(RESPONSE_BODY));
}

static int signal_handler(gpointer user_data) {
    // All attached signal sources just cause the loop to exit gracefully
    bool* server_running = (bool*)user_data;
    *server_running = false;
    return 0;
}

int main() {
    // Set to false by a subroutine when the server needs to exit.
    bool server_running = true;

    GMainLoop* main_loop = g_main_loop_new(NULL, FALSE);
    GMainContext* main_context = g_main_loop_get_context(main_loop);

    // Signal handler
    GSource* signal_source = g_unix_signal_source_new(SIGINT);
    g_source_set_callback(signal_source, signal_handler, &server_running,
        NULL);
    g_source_attach(signal_source, main_context);

    // Web server
    SoupServer* soup_server = soup_server_new("tls-certificate", NULL,
        "raw-paths", FALSE, "server-header", argp_program_name, NULL);
    soup_server_add_handler(soup_server, "/", handle_connection,
        NULL, NULL);

    GError* error = NULL;
    soup_server_listen_all(soup_server, CONFIG_WEB_SERVER_PORT, 0, &error);
    g_info("Web server listening at 0.0.0.0:%d", CONFIG_WEB_SERVER_PORT);

    while (server_running) {
        g_main_context_iteration(main_context, FALSE);
    }

    g_info("Exiting gracefully");
}

///////////////////////////////////////////////////////////////////////////////
