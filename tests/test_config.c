#include "libslavery.h"

int main(int argc, char *argv[]) {
	UNUSED(argc);

	slavery_config_t *config = slavery_config_read(argv[1]);

	slavery_config_print(config);

	return 0;
}
