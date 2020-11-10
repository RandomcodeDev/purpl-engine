#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Include the Purpl API header */
#include <purpl/purpl.h>

int main(int argc, char *argv[])
{
	/* Seed the RNG for our window title and a log message */
	srand((uint)time(NULL));

	/* The log index */
	uint logindex = 0;

	/* Create a new engine instance, passing the constructors for the arguments as is the preferred way of using this constructor */
	purpl::engine_inst inst = purpl::engine_inst(new purpl::app_info(), new purpl::window(1024, 600, "Purpl Demo"), true, "triangle_predef.vert", "triangle.frag");

	/* Ensure we have the correct log index */
	logindex = inst.info->logindex;

	/* Write a message into the log */
	inst.info->log->write(logindex, INFO, P_FILENAME, __LINE__, "A random number: %d", rand() % 10);

	/* Our main loop */
	while (inst.is_active) {
		/* TODO: add graphics usage/ImGui menus to demonstrate available features */

		inst.update();
	}

	return 0;
}
