/**
 * @file
 * @brief Test config parsing.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#include "config.h"
#include "libslavery.h"

#include <stdlib.h>

int main(int argc, char *argv[]) {
	UNUSED(argc);

	slavery_config_t *config = slavery_config_new(argv[1]);

	if (config->num_entries != 1) {
		log_warning(SLAVERY_ERROR_CONFIG, "expected %u config entries, found %u", 1, config->num_entries);

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
