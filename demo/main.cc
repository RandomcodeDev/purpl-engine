#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Include the Purpl API header */
#include <purpl/purpl.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	char *test_file = NULL;
	size_t test_file_len = 0;
	struct purpl::vert_info *triangle;
	size_t i;

	/* Seed the RNG for our window title and a log message */
	srand((uint)time(NULL));

	/* The log index */
	uint logindex = 0;

	/* The vertices for the triangle */
	triangle = (struct purpl::vert_info *)calloc(
		3, sizeof(struct purpl::vert_info));
	triangle[0].pos[0] = 0.0;
	triangle[0].pos[1] = -0.5;
	triangle[0].pos[2] = 0.0;
	triangle[0].pos[3] = 1.0;
	
	triangle[1].pos[0] = 0.5;
	triangle[1].pos[1] = 0.5;
	triangle[1].pos[2] = 0.0;
	triangle[1].pos[3] = 1.0;

	triangle[2].pos[0] = -0.5;
	triangle[2].pos[1] = 0.5;
	triangle[2].pos[2] = 0.0;
	triangle[2].pos[3] = 1.0;
	
	for (i = 0; i < 3; i++) {
		triangle[i].colour[i] = 1;
		triangle[i].colour[3] = 1;
	}

	/* Create a new engine instance, passing the constructors for the arguments as is the preferred way of using this constructor */
	purpl::engine_inst inst = purpl::engine_inst(
		new purpl::app_info(),
		new purpl::window(1024, 600, "Purpl Demo"), true,
		"triangle.vert", "triangle.frag", triangle, 3);

	/* Ensure we have the correct log index */
	logindex = inst.info->logindex;

	/* Write a message into the log */
	inst.info->log->write(logindex, INFO, P_FILENAME, __LINE__,
			      "Another random number: %d", rand() % 10);

	/* Open our test file */
	fp = fopen("test", "rb+");
	if (fp) {
		/* This is to test the map/unmap_file functions */
		if (purpl::map_file(fp, &test_file_len, &test_file) != 0)
			inst.info->log->write(logindex, ERR, P_FILENAME,
					      __LINE__, "Failed to map file!");
		else
			inst.info->log->write(logindex, INFO, P_FILENAME,
					      __LINE__,
					      "Test file contents:\n%s",
					      test_file);
	}

	/* Now unmap the file */
	purpl::unmap_file(test_file, test_file_len);

	/* Our main loop */
	while (inst.is_active) {
		/* TODO: add graphics usage/ImGui menus to demonstrate available features */

		inst.update(inst.wnd->width, inst.wnd->height, triangle, 3,
			    "Purpl Demo (%llu)", time(NULL));
	}

	return 0;
}
