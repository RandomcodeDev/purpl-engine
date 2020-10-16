#pragma once

#ifndef PURPL_APP_INFO_H
#define PURPL_APP_INFO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include <json.h>

#include  "log.h"
#include  "macro.h"
#include  "types.h"
#include  "util.h"

namespace purpl
{
class P_EXPORT app_info {
    public:
	int logindex;
	logger *log;
	FILE *fp;

	/* 
	 * Parses the JSON file.
	 * Defined in app_info.cc
	 */
	bool parse(const char *fname =  "app.json", ...);

	/*
	 * Ensures the JSON fields are correct.
	 * Defined in app_info.cc
	 */
	bool validate(void);

	/* 
	 * Gets the resource path.
	 * Defined in app_info.cc
	 */
	const char *get_res_path(void);

	/* 
	 * Gets the log path.
	 * Defined in app_info.cc
	 */
	const char *get_log_path(void);

	/* 
	 * Gets the settings path
	 * Defined in app_info.cc
	 */
	const char *get_settings_path(void);
	
	/*
	 * Sets up the members in order to use the info in the class.
	 * Defined in app_info.cc
	 */
	app_info(const char *fname =  "app.json", ...);

	/* 
	 * Function pointers that are enabled if P_APPINFO_USERDEF_PARSE/P_APPINFO_USERDEF_VALIDATE are defined.
	 * Defined in app_info.cc
	 */
#ifdef P_APPINFO_USERDEF_PARSE
	bool (*userdef_parse)(void);
#endif /* P_APPINFO_PARSE */

#ifdef P_APPINFO_USERDEF_VALIDATE
	bool (*userdef_validate)(void);
#endif /* P_APPINFO_VALIDATE */

	/* Frees/closes everything for this instance */
	~app_info(void);

    private:
	struct json_object *root;
	struct json_object *res_path;
	struct json_object *log_path;
	struct json_object *settings_path;
};
}

#endif /* !PURPL_APP_INFO_H */
