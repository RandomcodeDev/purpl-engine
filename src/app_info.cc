#include "purpl/app_info.h"
using namespace purpl;

bool P_EXPORT purpl::app_info::parse(void)
{
#ifndef P_APPINFO_USERDEF_PARSE
	char *json;

	/* Get the file's contents */
	json = load_file_fp(this->fp, false, NULL);
	if (!json)
		return false;

	/* Get the JSON objects */
	this->root = json_tokener_parse(json);
	json_object_object_get_ex(this->root, "res_path", &this->res_path);
	json_object_object_get_ex(this->root, "log_path", &this->log_path);
	json_object_object_get_ex(this->root, "settings_path",
				  &this->settings_path);

	/* Free our buffer */
	free(json);

	return true;
#else
	return this->userdef_parse();
#endif /* !P_APPINFO_USERDEF_PARSE */
}

P_EXPORT bool purpl::app_info::validate(void)
{
#ifndef P_APPINFO_USERDEF_PARSE
	/*
	 * This function is really simple. Basically, it first checks
	 * whether the objects are present, then if they're the right type.
	 */
	if (!this->res_path || !this->log_path || !this->settings_path) {
		errno = EINVAL;
		return false;
	}

	if (json_object_get_type(this->res_path) != json_type_string) {
		errno = EINVAL;
		return false;
	}

	if (json_object_get_type(this->log_path) != json_type_string) {
		errno = EINVAL;
		return false;
	}

	if (json_object_get_type(this->settings_path) != json_type_string) {
		errno = EINVAL;
		return false;
	}

	return true;
#else
	return this->userdef_parse();
#endif /* !P_APPINFO_USERDEF_VALIDATE */
}

const char *purpl::app_info::get_res_path(void)
{
	return json_object_get_string(this->res_path);
}

const char *purpl::app_info::get_log_path(void)
{
	return json_object_get_string(this->log_path);
}

const char *purpl::app_info::get_settings_path(void)
{
	return json_object_get_string(this->settings_path);
}

P_EXPORT purpl::app_info::app_info(const char *fname, ...)
{
	va_list args;
	char *buf;

	/* Format the file name string */
	va_start(args, fname);
	buf = fmt_text_va(fname, &args);
	va_end(args);

	this->fp = fopen(buf, "rb");
	if (!this->fp)
		return;

	/* Fill in the JSON objects */
	if (!this->parse()) {
		errno = EFAULT;
		return;
	}

	/* Now make sure they're correct */
	if (!this->validate()) {
		errno = EINVAL;
		return;
	}

	/* Free the buffer */
	free(buf);

	/* Then start up the logger */
	this->log = new logger(&this->logindex, P_DEFAULT_LOG_LEVEL, "%s",
			       this->get_log_path());
}

P_EXPORT purpl::app_info::~app_info(void)
{
	delete log;

	fclose(this->fp);
}
