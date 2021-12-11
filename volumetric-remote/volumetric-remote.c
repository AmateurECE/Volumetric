///////////////////////////////////////////////////////////////////////////////
// NAME:            volumetric-remote.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for volumetric-remote tool.
//
// CREATED:         12/10/2021
//
// LAST EDITED:     12/11/2021
//
// Copyright 2021, Ethan D. Twardy
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

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>

#include <libvolumetric/volumetric.h>

static const char* VOLUMETRIC_CONFIG = "volumetric.yaml";

static bool is_valid_url(const char* input) {
    CURLU* url = curl_url();
    CURLUcode result = curl_url_set(url, CURLUPART_URL, input, 0);
    if (CURLUE_OK != result) {
        return false;
    }
    curl_url_cleanup(url);
    return true;
}

int main(int argc, char** argv) {
    if (2 > argc) {
        printf("usage: %s <url>\n", argv[0]);
        exit(1);
    }

    // Attempt to load the configuration from the current directory.
    VolumetricConfig* config = volumetric_config_new();
    int result = volumetric_config_load(config);
    if (ENOENT == result) {
        fprintf(stderr, "Error: No configuration was found in the working "
            "directory\n");
        goto error_exit;
    }

    if (!is_valid_url(argv[1])) {
        fprintf(stderr, "Error: The provided URL was not recognized as valid "
            "for this application\n");
        goto error_exit;
    }

    return result;
 error_exit:
    volumetric_config_free(&config);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
